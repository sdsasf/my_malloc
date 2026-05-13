const hist = [];
const maxN = 260;
let smoothSample = null;
let comparisonData = null;
let oracleTrace = [];
let comparisonFetchPending = false;
const adaptiveTpsTrace = [];
const maxAdaptiveTrace = 420;
const modes = [
  ['balanced', 'balanced'],
  ['throughput_cache', 'throughput'],
  ['deterministic_latency', 'latency'],
  ['compact_rss', 'compact'],
  ['fragmentation_stable', 'fragment'],
  ['cross_thread', 'x-thread'],
  ['large_object', 'large stream'],
  ['hardened_debug', 'debug']
];
let lastMode = '';
const modeIndex = Object.fromEntries(modes.map(([id], i) => [id, i]));
const modeColors = {
  balanced: '#8e98a5',
  throughput_cache: '#b6f2c2',
  deterministic_latency: '#99d6ff',
  compact_rss: '#f0b35b',
  fragmentation_stable: '#f5f5f1',
  cross_thread: '#78b9e8',
  large_object: '#ffd27d',
  hardened_debug: '#ff7d70'
};

const icons = {
  pulse: '<svg viewBox="0 0 24 24"><path d="M3 12h4l2-6 4 13 3-7h5"/></svg>',
  memory: '<svg viewBox="0 0 24 24"><rect x="6" y="6" width="12" height="12" rx="1"/><path d="M9 2v4M15 2v4M9 18v4M15 18v4M2 9h4M2 15h4M18 9h4M18 15h4"/></svg>',
  orbit: '<svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="2"/><ellipse cx="12" cy="12" rx="9" ry="4"/><ellipse cx="12" cy="12" rx="4" ry="9" transform="rotate(35 12 12)"/></svg>',
  shield: '<svg viewBox="0 0 24 24"><path d="M12 3l8 3v6c0 5-3.5 8-8 9-4.5-1-8-4-8-9V6l8-3z"/><path d="M8 12l3 3 5-6"/></svg>'
};

document.querySelectorAll('.metric-icon').forEach(el => {
  el.innerHTML = icons[el.dataset.icon] || '';
});

function initModeRail() {
  document.getElementById('modeLabels').innerHTML = modes
    .map(([, label]) => `<div class="mode-label">${label}</div>`)
    .join('');
  document.getElementById('modeRail').innerHTML = modes
    .map(([id]) => `<div class="mode-dot" data-mode="${id}" title="${id}"></div>`)
    .join('');
}

function renderModeRail(current, previous) {
  const normalized = current || '';
  const switched = lastMode && normalized && lastMode !== normalized;
  document.querySelectorAll('.mode-dot').forEach(dot => {
    const active = dot.dataset.mode === normalized;
    dot.classList.toggle('active', active);
    dot.classList.toggle('previous', dot.dataset.mode === previous && !active);
    dot.classList.toggle('switched', active && switched);
  });
  document.querySelectorAll('.mode-label').forEach((label, i) => {
    label.classList.toggle('active', modes[i][0] === normalized);
  });
  if (normalized) lastMode = normalized;
}

function fmt(n) {
  if (n === undefined || n === null) return 'n/a';
  if (typeof n === 'boolean') return n ? 'yes' : 'no';
  if (typeof n === 'string') return n;
  if (!Number.isFinite(Number(n))) return String(n);
  return Number(n).toLocaleString(undefined, { maximumFractionDigits: 2 });
}

function bytes(n) {
  const value = Number(n || 0);
  const units = ['B', 'KiB', 'MiB', 'GiB'];
  let v = Math.abs(value);
  let i = 0;
  while (v >= 1024 && i < units.length - 1) {
    v /= 1024;
    i++;
  }
  const signed = value < 0 ? -v : v;
  return `${signed.toLocaleString(undefined, { maximumFractionDigits: i ? 1 : 0 })} ${units[i]}`;
}

function missionTime(ms) {
  const total = Math.max(0, Number(ms || 0) / 1000);
  const min = Math.floor(total / 60);
  const sec = total - min * 60;
  return `T+${String(min).padStart(2, '0')}:${sec.toFixed(1).padStart(4, '0')}`;
}

function rows(items) {
  return items.map(x => `<tr><td>${x[0]}</td><td>${fmt(x[1])}</td></tr>`).join('');
}

function numericArray(value, count) {
  const out = Array.isArray(value) ? value.slice(0, count) : [];
  while (out.length < count) out.push(0);
  return out.map(v => Number(v || 0));
}

function sum(values) {
  return values.reduce((acc, v) => acc + Number(v || 0), 0);
}

function pct(value, max = 1) {
  if (max <= 0) return 0;
  return Math.max(0, Math.min(100, Number(value || 0) / max * 100));
}

function setText(id, text) {
  const el = document.getElementById(id);
  if (el) el.textContent = text;
}

function setMeter(id, percent) {
  const el = document.getElementById(id);
  if (el) el.style.width = `${Math.max(0, Math.min(100, percent))}%`;
}

function setNodeState(id, state) {
  const el = document.getElementById(id);
  if (!el) return;
  el.classList.toggle('active', state === 'active');
  el.classList.toggle('hot', state === 'hot');
  el.classList.toggle('warn', state === 'warn');
  el.classList.toggle('alert', state === 'alert');
}

const policyProfiles = {
  balanced: {
    intent: 'auto substrate with moderate reuse',
    release: 'cache / central pool',
    chips: ['auto', 'reuse', 'soft reclaim']
  },
  throughput_cache: {
    intent: 'reuse-first TLS magazines and larger batches',
    release: 'cache',
    chips: ['tcache', 'large batch', 'high retain']
  },
  deterministic_latency: {
    intent: 'stable reuse with bounded maintenance',
    release: 'cache / delayed purge',
    chips: ['stable batch', 'low jitter', 'no hot purge']
  },
  compact_rss: {
    intent: 'low RSS with aggressive reclaim',
    release: 'purge / unmap',
    chips: ['low rss', 'empty keep 0', 'unmap']
  },
  fragmentation_stable: {
    intent: 'occupancy packing for mixed sizes',
    release: 'central pool / purge',
    chips: ['pack spans', 'fit classes', 'low waste']
  },
  cross_thread: {
    intent: 'owner-aware remote-free routing',
    release: 'remote queue / owner drain',
    chips: ['owner id', 'remote queue', 'drain']
  },
  large_object: {
    intent: 'true-large isolation through direct maps',
    release: 'unmap direct regions',
    chips: ['>256 KiB', 'direct map', 'no pool pollution']
  },
  hardened_debug: {
    intent: 'debug metadata, poison, redzone, quarantine',
    release: 'quarantine / unmap',
    chips: ['canary', 'redzone', 'poison']
  }
};

function featureValue(e, key) {
  return Math.max(0, Number((e && e[key]) || 0));
}

const featureDefs = [
  {
    key: 'large_bytes_ratio',
    label: 'stream-large >256 KiB',
    short: 'stream-large',
    max: 1,
    severity: v => v / 0.55
  },
  {
    key: 'remote_free_ratio',
    label: 'remote free',
    short: 'remote free',
    max: 1,
    severity: v => v / 0.25
  },
  {
    key: 'mapped_live_ratio',
    label: 'mapped/live pressure',
    short: 'mapped/live',
    max: 4096,
    scale: 'log',
    severity: v => Math.log1p(v) / Math.log1p(1024)
  },
  {
    key: 'slow_path_ratio',
    label: 'slow path',
    short: 'slow path',
    max: 1,
    severity: v => v / 0.45
  },
  {
    key: 'fragmentation_estimate',
    label: 'fragmentation',
    short: 'fragmentation',
    max: 1,
    severity: v => v / 0.25
  },
  {
    key: 'cache_hit_rate',
    label: 'cache hit',
    short: 'cache hit',
    max: 1,
    severity: v => v
  }
];

function featureByReason(reason) {
  const map = {
    large_bytes_ratio: 'large_bytes_ratio',
    remote_free_ratio: 'remote_free_ratio',
    mapped_live_ratio: 'mapped_live_ratio',
    slow_path_ratio: 'slow_path_ratio',
    fragmentation: 'fragmentation_estimate',
    cache_reuse: 'cache_hit_rate'
  };
  const key = map[reason];
  return featureDefs.find(f => f.key === key);
}

function dominantFeature(e) {
  let best = null;
  for (const def of featureDefs) {
    const value = featureValue(e, def.key);
    const score = Math.max(0, def.severity(value));
    if (!best || score > best.score) best = { def, value, score };
  }
  return best;
}

function importantValue(e) {
  if (!e) return '';
  const reasonDef = featureByReason(e.reason);
  if (reasonDef) return `${reasonDef.short} ${fmt(featureValue(e, reasonDef.key))}`;
  const top = dominantFeature(e);
  return top ? `${top.def.short} ${fmt(top.value)}` : '';
}

function featureBarPct(def, value) {
  if (def.scale === 'log') {
    return Math.max(0, Math.min(100, Math.log1p(value) / Math.log1p(def.max) * 100));
  }
  return Math.max(0, Math.min(100, value / def.max * 100));
}

function niceRange(values) {
  let mn = Math.min(...values);
  let mx = Math.max(...values);
  if (!Number.isFinite(mn) || !Number.isFinite(mx)) return [0, 1];
  if (mx <= mn) {
    mn = 0;
    mx = Math.max(1, mx);
  }
  const pad = (mx - mn) * 0.12;
  return [Math.max(0, mn - pad), mx + pad];
}

function unitFmt(value, unit) {
  if (unit === 'ratio') return fmt(value);
  if (unit === 'ops/sec') return fmt(value);
  if (unit === 'KB') return fmt(value) + ' KB';
  return fmt(value);
}

function phaseX(p) {
  return Number(p.phase_index || 0) + Number(p.phase_progress || 0);
}

function traceBucket(w) {
  return Math.max(0, Math.min(20, Math.floor(Number(w.phase_progress || 0) * 20)));
}

function pointBucket(point) {
  if (point && Number.isFinite(Number(point.phase_bucket))) {
    return Number(point.phase_bucket);
  }
  return traceBucket(point || {});
}

function comparisonModeTraces() {
  return comparisonData && Array.isArray(comparisonData.modes) ? comparisonData.modes : [];
}

function modeTrace(mode) {
  return comparisonModeTraces().find(trace => trace.mode === mode) || null;
}

function rebuildOracleTrace() {
  const best = new Map();
  comparisonModeTraces().forEach(trace => {
    (trace.points || []).forEach(point => {
      const key = `${point.phase_index}:${point.phase_bucket}`;
      const tps = Number(point.ops_per_sec || 0);
      const prev = best.get(key);
      if (!prev || tps > prev.ops_per_sec) {
        best.set(key, {
          phase_index: Number(point.phase_index || 0),
          phase_bucket: Number(point.phase_bucket || 0),
          phase_progress: Number(point.phase_progress || 0),
          ops_per_sec: tps,
          mode: trace.mode
        });
      }
    });
  });
  oracleTrace = [...best.values()].sort((a, b) => phaseX(a) - phaseX(b));
}

function nearestTracePoint(points, w) {
  if (!points || !points.length) return null;
  const phase = Number(w.phase_index || 0);
  const bucket = pointBucket(w);
  let nearest = null;
  let bestDistance = Number.POSITIVE_INFINITY;
  points.forEach(point => {
    if (Number(point.phase_index) !== phase) return;
    const distance = Math.abs(pointBucket(point) - bucket);
    if (distance < bestDistance) {
      nearest = point;
      bestDistance = distance;
    }
  });
  return nearest;
}

function nearestOraclePoint(w) {
  return nearestTracePoint(oracleTrace, w);
}

function lastAdaptivePoint() {
  return adaptiveTpsTrace.length ? adaptiveTpsTrace[adaptiveTpsTrace.length - 1] : null;
}

function comparisonCursor(w) {
  const livePhase = Number(w.phase_index || 0);
  const knownPhaseCount = maxPhaseCount();
  const lastAdaptive = lastAdaptivePoint();
  if ((!w.running && lastAdaptive) || livePhase >= knownPhaseCount) {
    return lastAdaptive || w;
  }
  return w;
}

function recordAdaptiveTps(w, a, tps) {
  if (!w || !w.running || !Number.isFinite(Number(tps)) || Number(tps) <= 0) return;
  const point = {
    phase_index: Number(w.phase_index || 0),
    phase_bucket: traceBucket(w),
    phase_progress: Number(w.phase_progress || 0),
    ops_per_sec: Number(tps || 0),
    mode: a.current_mode || 'n/a'
  };
  const last = adaptiveTpsTrace[adaptiveTpsTrace.length - 1];
  if (last && last.phase_index === point.phase_index && last.phase_bucket === point.phase_bucket) {
    adaptiveTpsTrace[adaptiveTpsTrace.length - 1] = point;
    return;
  }
  adaptiveTpsTrace.push(point);
  if (adaptiveTpsTrace.length > maxAdaptiveTrace) adaptiveTpsTrace.shift();
}

function tpsSeriesRange() {
  const values = [];
  comparisonModeTraces().forEach(trace => (trace.points || []).forEach(p => values.push(Number(p.ops_per_sec || 0))));
  oracleTrace.forEach(p => values.push(Number(p.ops_per_sec || 0)));
  adaptiveTpsTrace.forEach(p => values.push(Number(p.ops_per_sec || 0)));
  return niceRange(values.length ? values : [0, 1]);
}

function maxPhaseCount() {
  let maxPhase = 1;
  comparisonModeTraces().forEach(trace => {
    (trace.points || []).forEach(point => {
      maxPhase = Math.max(maxPhase, Number(point.phase_index || 0) + 1);
    });
  });
  adaptiveTpsTrace.forEach(point => {
    maxPhase = Math.max(maxPhase, Number(point.phase_index || 0) + 1);
  });
  return maxPhase;
}

function drawPolyline(ctx, points, xOf, yOf, color, width, dashed = false, opacity = 1) {
  if (!points || !points.length) return;
  ctx.save();
  ctx.globalAlpha = opacity;
  ctx.strokeStyle = color;
  ctx.lineWidth = width;
  ctx.setLineDash(dashed ? [8, 5] : []);
  ctx.beginPath();
  points.forEach((point, i) => {
    const x = xOf(point);
    const y = yOf(point);
    if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
  });
  ctx.stroke();
  ctx.restore();
}

function drawComparisonChart(activeMode = '') {
  const canvas = document.getElementById('comparisonChart');
  if (!canvas) return;
  const ctx = canvas.getContext('2d');
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.fillStyle = '#020303';
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  const top = 42, right = 22, bottom = 42, left = 82;
  const h = canvas.height - top - bottom;
  const w = canvas.width - left - right;
  const [mn, mx] = tpsSeriesRange();
  const phaseCount = maxPhaseCount();
  const xOf = p => left + phaseX(p) / Math.max(1, phaseCount) * w;
  const yOf = p => top + h - (Number(p.ops_per_sec || 0) - mn) / Math.max(1e-9, mx - mn) * h;

  ctx.strokeStyle = '#171c23';
  ctx.lineWidth = 1;
  for (let i = 0; i <= 4; i++) {
    const y = top + i * h / 4;
    ctx.beginPath(); ctx.moveTo(left, y); ctx.lineTo(left + w, y); ctx.stroke();
    const value = mx - (mx - mn) * i / 4;
    ctx.fillStyle = '#56606d';
    ctx.font = '11px system-ui';
    ctx.fillText(unitFmt(value, 'ops/sec'), 8, y + 4);
  }
  for (let i = 0; i <= phaseCount; i++) {
    const x = left + i * w / Math.max(1, phaseCount);
    ctx.beginPath(); ctx.moveTo(x, top); ctx.lineTo(x, top + h); ctx.stroke();
    ctx.fillStyle = '#56606d';
    ctx.font = '11px system-ui';
    ctx.fillText(`P${i + 1}`, Math.min(left + w - 24, x + 4), canvas.height - 16);
  }

  const selectedTrace = modeTrace(activeMode);
  if (selectedTrace) {
    drawPolyline(ctx, selectedTrace.points || [], xOf, yOf, modeColors[activeMode] || '#99d6ff', 2.0, false, 0.88);
  }
  drawPolyline(ctx, oracleTrace, xOf, yOf, '#f0b35b', 2.5, true, 0.98);
  drawPolyline(ctx, adaptiveTpsTrace, xOf, yOf, '#f5f5f1', 3.2, false, 1);

  let previousMode = '';
  adaptiveTpsTrace.forEach(point => {
    if (!previousMode) {
      previousMode = point.mode;
      return;
    }
    if (point.mode === previousMode) return;
    const x = xOf(point);
    ctx.save();
    ctx.strokeStyle = '#f5f5f1';
    ctx.globalAlpha = 0.58;
    ctx.setLineDash([4, 5]);
    ctx.beginPath(); ctx.moveTo(x, top); ctx.lineTo(x, top + h); ctx.stroke();
    ctx.restore();
    ctx.fillStyle = '#f5f5f1';
    ctx.font = '11px system-ui';
    ctx.fillText(point.mode, Math.min(left + w - 110, x + 5), top + 14);
    previousMode = point.mode;
  });

  ctx.fillStyle = '#f5f5f1';
  ctx.font = '12px system-ui';
  ctx.fillText('ADAPTIVE TPS AGAINST ORACLE', left, 19);
  ctx.fillStyle = '#8e98a5';
  ctx.fillText(`adaptive white | oracle dashed | selected fixed ${activeMode || 'mode'} colored`, left + 208, 19);
}

function ensureModeMiniGrid() {
  const grid = document.getElementById('modeMiniGrid');
  if (!grid || grid.dataset.ready === '1') return;
  grid.innerHTML = modes.map(([mode, label]) => `
    <article class="mode-mini-card">
      <div class="mode-mini-head">
        <strong>${label}</strong>
        <span class="mode-mini-stats">
          <span id="miniNow-${mode}">last 0</span>
          <span id="miniPeak-${mode}">peak 0</span>
        </span>
      </div>
      <canvas id="mini-${mode}" class="mode-mini-canvas" width="300" height="120"></canvas>
    </article>`).join('');
  grid.dataset.ready = '1';
}

function drawMiniModeCharts() {
  const grid = document.getElementById('modeMiniGrid');
  if (!comparisonData || !comparisonData.ready || !comparisonModeTraces().length) {
    if (grid) {
      grid.innerHTML = '';
      grid.dataset.ready = '0';
    }
    return;
  }
  ensureModeMiniGrid();
  const phaseCount = maxPhaseCount();
  comparisonModeTraces().forEach(trace => {
    const canvas = document.getElementById(`mini-${trace.mode}`);
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.fillStyle = '#020303';
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    const top = 12, right = 8, bottom = 14, left = 8;
    const h = canvas.height - top - bottom;
    const w = canvas.width - left - right;
    const values = (trace.points || []).map(p => Number(p.ops_per_sec || 0));
    const [mn, mx] = niceRange(values.length ? values : [0, 1]);
    const xOf = p => left + phaseX(p) / Math.max(1, phaseCount) * w;
    const yOf = p => top + h - (Number(p.ops_per_sec || 0) - mn) / Math.max(1e-9, mx - mn) * h;
    ctx.strokeStyle = '#171c23';
    for (let i = 0; i <= phaseCount; i++) {
      const x = left + i * w / Math.max(1, phaseCount);
      ctx.beginPath(); ctx.moveTo(x, top); ctx.lineTo(x, top + h); ctx.stroke();
    }
    drawPolyline(ctx, trace.points || [], xOf, yOf, modeColors[trace.mode] || '#8e98a5', 2.0, false, 1);
    const peak = Math.max(0, ...(trace.points || []).map(p => Number(p.ops_per_sec || 0)));
    const last = (trace.points || [])[Math.max(0, (trace.points || []).length - 1)] || null;
    setText(`miniNow-${trace.mode}`, `last ${fmt(last ? last.ops_per_sec : 0)}`);
    setText(`miniPeak-${trace.mode}`, `peak ${fmt(peak)}`);
  });
}

function stripHtml(points, currentKey) {
  return (points || []).map(point => {
    const key = `${point.phase_index}:${point.phase_bucket}`;
    const active = key === currentKey ? ' active' : '';
    const color = modeColors[point.mode] || '#242a33';
    const title = `${point.mode || 'n/a'} | ${fmt(point.ops_per_sec || 0)} ops/sec`;
    return `<div class="strip-cell${active}" title="${title}" style="background:${color}"></div>`;
  }).join('');
}

function renderOracleState(w, a, currentTps) {
  const state = document.getElementById('comparisonState');
  if (!comparisonData || !comparisonData.enabled) {
    setText('comparisonState', 'reference off');
    setText('oracleMode', 'enable compare');
    setText('oracleEfficiency', '--telemetry');
    setText('oracleAlignment', 'compare-modes');
    setText('oracleGap', 'no oracle');
    return;
  }
  state.classList.toggle('hot', !!comparisonData.ready);
  setText('comparisonState', comparisonData.ready ? 'TPS oracle ready' : comparisonData.status || 'collecting');
  const cursor = comparisonCursor(w);
  const lastAdaptive = lastAdaptivePoint();
  const displayTps = (!w.running && lastAdaptive)
    ? Number(lastAdaptive.ops_per_sec || 0)
    : Number(currentTps || 0);
  const oracle = nearestOraclePoint(cursor);
  if (!oracle || !Number.isFinite(displayTps) || displayTps <= 0) {
    setText('oracleMode', oracle ? oracle.mode : comparisonData.ready ? 'awaiting phase' : 'collecting');
    setText('oracleEfficiency', comparisonData.ready ? 'awaiting TPS' : 'collecting');
    setText('oracleAlignment', comparisonData.ready ? 'awaiting run' : 'collecting');
    setText('oracleGap', comparisonData.ready ? 'awaiting TPS' : 'collecting');
    return;
  }
  const adaptiveTps = displayTps;
  const oracleTps = Number(oracle.ops_per_sec || 0);
  const efficiency = oracleTps > 0 ? adaptiveTps / oracleTps : 0;
  const gap = Math.max(0, oracleTps - adaptiveTps);
  const adaptiveMode = (!w.running && lastAdaptive && lastAdaptive.mode)
    ? lastAdaptive.mode
    : (a.current_mode || '');
  const aligned = adaptiveMode === oracle.mode;
  setText('oracleMode', oracle.mode || 'n/a');
  setText('oracleEfficiency', `${fmt(efficiency * 100)}%`);
  setText('oracleAlignment', aligned ? 'matched' : 'diverged');
  setText('oracleGap', `${fmt(gap)} ops/sec`);
  const eff = document.getElementById('oracleEfficiency');
  const align = document.getElementById('oracleAlignment');
  [eff, align].forEach(el => el && el.classList.remove('good', 'warn', 'bad'));
  if (eff) eff.classList.add(efficiency >= 0.92 ? 'good' : efficiency >= 0.75 ? 'warn' : 'bad');
  if (align) align.classList.add(aligned ? 'good' : 'warn');
  const key = `${pointBucket(cursor) >= 0 ? Number(cursor.phase_index || 0) : 0}:${pointBucket(cursor)}`;
  document.getElementById('oracleModeStrip').innerHTML = stripHtml(oracleTrace, key);
  document.getElementById('adaptiveModeStrip').innerHTML = stripHtml(adaptiveTpsTrace, key);
}

function renderModeRanking(w, a, currentTps) {
  const host = document.getElementById('modeRanking');
  const foot = document.getElementById('rankingFoot');
  if (!host || !foot) return;
  if (!comparisonData || !comparisonData.enabled) {
    setText('rankingPhase', 'reference off');
    host.innerHTML = '<div class="ranking-foot">No fixed-mode reference traces were requested for this run.</div>';
    foot.textContent = 'Start the workload with --telemetry-compare-modes to rank all modes.';
    return;
  }
  if (!comparisonData.ready) {
    setText('rankingPhase', comparisonData.status || 'collecting');
    host.innerHTML = '<div class="ranking-foot">Collecting isolated fixed-mode TPS references.</div>';
    foot.textContent = 'Ranking appears after comparison traces are ready.';
    return;
  }

  const cursor = comparisonCursor(w);
  const oracle = nearestOraclePoint(cursor);
  const lastAdaptive = lastAdaptivePoint();
  const adaptiveMode = (!w.running && lastAdaptive && lastAdaptive.mode)
    ? lastAdaptive.mode
    : (a.current_mode || '');
  const displayTps = (!w.running && lastAdaptive)
    ? Number(lastAdaptive.ops_per_sec || 0)
    : Number(currentTps || 0);
  const rows = comparisonModeTraces()
    .map(trace => {
      const point = nearestTracePoint(trace.points || [], cursor);
      return point ? {
        mode: trace.mode,
        ops_per_sec: Number(point.ops_per_sec || 0),
        point
      } : null;
    })
    .filter(Boolean)
    .sort((lhs, rhs) => rhs.ops_per_sec - lhs.ops_per_sec);

  if (!rows.length) {
    setText('rankingPhase', 'awaiting phase');
    host.innerHTML = '<div class="ranking-foot">No comparable fixed-mode sample exists for the current phase.</div>';
    foot.textContent = 'The viewer will reuse the last valid phase after the live run finishes.';
    return;
  }

  const best = rows[0];
  const scale = Math.max(1, best.ops_per_sec);
  setText('rankingPhase', `P${Number(cursor.phase_index || 0) + 1}.${pointBucket(cursor) + 1}`);
  host.innerHTML = rows.map((row, index) => {
    const isActive = row.mode === adaptiveMode;
    const isOracle = oracle && row.mode === oracle.mode;
    const width = Math.max(2, Math.min(100, row.ops_per_sec / scale * 100));
    const label = modes.find(([id]) => id === row.mode)?.[1] || row.mode;
    const color = modeColors[row.mode] || '#f5f5f1';
    return `<div class="ranking-row${isActive ? ' active' : ''}${isOracle ? ' oracle' : ''}">
      <div class="ranking-rank">#${index + 1}</div>
      <div class="ranking-mode"><span class="ranking-dot" style="background:${color}"></span><span>${label}</span></div>
      <div class="ranking-track"><div class="ranking-fill" style="width:${width}%;background:${color}"></div></div>
      <div class="ranking-value">${fmt(row.ops_per_sec)}</div>
    </div>`;
  }).join('');

  const oracleMode = oracle ? oracle.mode : best.mode;
  const oracleTps = oracle ? Number(oracle.ops_per_sec || 0) : best.ops_per_sec;
  const efficiency = oracleTps > 0 ? displayTps / oracleTps : 0;
  const alignment = adaptiveMode && oracleMode && adaptiveMode === oracleMode ? 'matched' : 'diverged';
  foot.textContent =
    `oracle ${oracleMode || 'n/a'} | adaptive ${adaptiveMode || 'n/a'} ${alignment} | live/oracle ${fmt(Math.max(0, efficiency) * 100)}%`;
}

function heatmapColor(ratio) {
  const bounded = Math.max(0, Math.min(1, Number(ratio || 0)));
  const alpha = 0.10 + bounded * 0.82;
  return `rgba(240,179,91,${alpha.toFixed(3)})`;
}

function renderModeHeatmap(w, activeMode = '') {
  const host = document.getElementById('modeHeatmap');
  if (!host) return;
  if (!comparisonData || !comparisonData.enabled) {
    host.style.gridTemplateColumns = '160px 1fr';
    host.innerHTML = '<div class="heatmap-corner">mode</div><div class="heatmap-axis">comparison disabled</div>';
    return;
  }
  if (!comparisonData.ready || !oracleTrace.length) {
    host.style.gridTemplateColumns = '160px 1fr';
    host.innerHTML = '<div class="heatmap-corner">mode</div><div class="heatmap-axis">collecting fixed-mode references</div>';
    return;
  }

  const cursor = comparisonCursor(w);
  const activeKey = `${Number(cursor.phase_index || 0)}:${pointBucket(cursor)}`;
  const columns = oracleTrace.slice();
  host.style.gridTemplateColumns = `160px repeat(${columns.length}, 22px)`;
  let html = '<div class="heatmap-corner">mode</div>';
  html += columns.map(point =>
    `<div class="heatmap-axis" title="phase ${Number(point.phase_index || 0) + 1}, bucket ${pointBucket(point) + 1}">P${Number(point.phase_index || 0) + 1}</div>`
  ).join('');

  modes.forEach(([mode, label]) => {
    const trace = modeTrace(mode);
    const displayActiveMode = activeMode || (lastAdaptivePoint() ? lastAdaptivePoint().mode : '');
    html += `<div class="heatmap-label${mode === displayActiveMode ? ' active' : ''}">${label}</div>`;
    html += columns.map(column => {
      const point = trace ? nearestTracePoint(trace.points || [], column) : null;
      const value = point ? Number(point.ops_per_sec || 0) : 0;
      const oracleValue = Math.max(1, Number(column.ops_per_sec || 0));
      const ratio = Math.max(0, Math.min(1, value / oracleValue));
      const cellKey = `${Number(column.phase_index || 0)}:${pointBucket(column)}`;
      const oracleClass = column.mode === mode ? ' oracle' : '';
      const activeClass = cellKey === activeKey ? ' active-phase' : '';
      const title = `${label} | P${Number(column.phase_index || 0) + 1}.${pointBucket(column) + 1} | ${fmt(value)} ops/sec | ${fmt(ratio * 100)}% of oracle`;
      return `<div class="heatmap-cell${oracleClass}${activeClass}" title="${title}" style="background:${heatmapColor(ratio)}"></div>`;
    }).join('');
  });
  host.innerHTML = html;
}

function renderComparison(w, a, currentTps) {
  recordAdaptiveTps(w, a, currentTps);
  renderOracleState(w, a, currentTps);
  renderModeRanking(w, a, currentTps);
  renderModeHeatmap(w, a.current_mode || '');
  renderComparisonLegend(a.current_mode || '');
  drawComparisonChart(a.current_mode || '');
  drawMiniModeCharts();
}

function renderComparisonLegend(activeMode = '') {
  const host = document.getElementById('comparisonLegend');
  if (!host) return;
  const activeLabel = modes.find(([mode]) => mode === activeMode)?.[1] || 'selected mode';
  const activeColor = modeColors[activeMode] || '#99d6ff';
  host.innerHTML = [
    '<span class="legend-chip"><span class="legend-swatch"></span>adaptive live</span>',
    '<span class="legend-chip"><span class="legend-swatch dashed" style="color:#f0b35b"></span>TPS oracle</span>',
    `<span class="legend-chip"><span class="legend-swatch" style="background:${activeColor}"></span>fixed ${activeLabel}</span>`
  ].join('');
}

async function loadComparison() {
  if (comparisonFetchPending) return;
  comparisonFetchPending = true;
  try {
    const r = await fetch('/comparison', { cache: 'no-store' });
    comparisonData = await r.json();
    rebuildOracleTrace();
    renderComparisonLegend();
    drawComparisonChart();
    renderModeHeatmap({ phase_index: 0, phase_bucket: 0, phase_progress: 0, running: false });
    drawMiniModeCharts();
  } catch (e) {
    comparisonData = { enabled: false, ready: false, status: 'unavailable', modes: [] };
  } finally {
    comparisonFetchPending = false;
  }
}

function drawSignal(canvasId, key, title, unit, color) {
  const c = document.getElementById(canvasId);
  const ctx = c.getContext('2d');
  ctx.clearRect(0, 0, c.width, c.height);
  ctx.fillStyle = '#020303';
  ctx.fillRect(0, 0, c.width, c.height);

  const top = 34, bottom = 28, left = 70, right = 16;
  const h = c.height - top - bottom;
  const w = c.width - left - right;
  const values = hist.map(p => p[key] || 0);
  const [mn, mx] = niceRange(values);
  ctx.strokeStyle = '#171c23';
  ctx.lineWidth = 1;
  for (let i = 0; i <= 4; i++) {
    const y = top + i * h / 4;
    ctx.beginPath(); ctx.moveTo(left, y); ctx.lineTo(left + w, y); ctx.stroke();
    const val = mx - (mx - mn) * i / 4;
    ctx.fillStyle = '#56606d';
    ctx.font = '11px system-ui';
    ctx.fillText(unitFmt(val, unit), 8, y + 4);
  }
  for (let i = 0; i <= 5; i++) {
    const x = left + i * w / 5;
    ctx.beginPath(); ctx.moveTo(x, top); ctx.lineTo(x, top + h); ctx.stroke();
  }
  ctx.fillStyle = '#f5f5f1';
  ctx.font = '12px system-ui';
  ctx.fillText(title.toUpperCase(), left, 19);
  ctx.fillStyle = '#8e98a5';
  ctx.fillText(`${unit} | min ${unitFmt(mn, unit)} | max ${unitFmt(mx, unit)}`, left + 150, 19);
  ctx.beginPath();
  ctx.strokeStyle = color;
  ctx.lineWidth = 2.4;
  hist.forEach((p, i) => {
    const x = left + i * (w / Math.max(1, maxN - 1));
    const v = ((p[key] || 0) - mn) / (mx - mn);
    const y = top + h - Math.max(0, Math.min(1, v)) * h;
    if (i) ctx.lineTo(x, y); else ctx.moveTo(x, y);
  });
  ctx.stroke();
  const last = hist[hist.length - 1];
  if (last) {
    const v = ((last[key] || 0) - mn) / (mx - mn);
    const x = left + (hist.length - 1) * (w / Math.max(1, maxN - 1));
    const y = top + h - Math.max(0, Math.min(1, v)) * h;
    ctx.fillStyle = color;
    ctx.beginPath(); ctx.arc(x, y, 4, 0, Math.PI * 2); ctx.fill();
    ctx.fillStyle = '#f5f5f1';
    ctx.font = '13px system-ui';
    ctx.fillText(`current ${unitFmt(last[key], unit)}`, left, c.height - 8);
  }
}

function drawCharts() {
  drawSignal('opsChart', 'ops', 'throughput', 'ops/sec', '#f5f5f1');
  drawSignal('liveChart', 'live', 'live memory', 'KB', '#8e98a5');
  drawSignal('mappedChart', 'mapped', 'mapped memory', 'KB', '#f0b35b');
  drawSignal('ratioChart', 'ratio', 'mapped / live', 'ratio', '#99d6ff');
}

function phaseCells(index, count) {
  const n = Math.max(1, Math.min(24, count || 1));
  let html = '';
  for (let i = 0; i < n; i++) {
    const active = i <= index * n / Math.max(1, count);
    html += `<div class="phase-cell ${active ? 'active' : ''}"></div>`;
  }
  document.getElementById('phaseCells').innerHTML = html;
}

function renderTimeline(events) {
  const list = (events || []).slice(-10).reverse();
  const html = list.map(e => {
    const from = e.current_mode || 'n/a';
    const to = e.candidate_mode || 'n/a';
    const title = e.switched ? `${from} -> ${to}` : `${from} held`;
    const backend = e.selector_backend ? `${e.selector_backend} selector` : 'selector';
    const model = e.model_candidate ? `model ${e.model_candidate} (${fmt(e.model_confidence)})` : '';
    const baseline = e.selector_backend === 'rule' && e.rule_candidate
      ? `rule ${e.rule_candidate}`
      : '';
    const top = dominantFeature(e);
    const signal = top ? `${top.def.short} ${fmt(top.value)}` : 'no signal';
    return `<div class="event ${e.switched ? 'switched' : ''}">
      <div class="event-title">${title}</div>
      <div class="event-reason">${backend} | ${fmt(e.reason)} | ${signal}</div>
      <div class="event-reason">${baseline ? baseline + ' | ' : ''}${model}</div>
      <div class="event-value">${importantValue(e)}</div>
    </div>`;
  }).join('');
  document.getElementById('modeTimeline').innerHTML = html || '<div class="event"><div class="event-title">waiting for selector window</div></div>';
}

function renderDecision(e) {
  if (!e) return;
  const title = e.switched
    ? `${e.current_mode} -> ${e.candidate_mode}`
    : `${e.current_mode} retained`;
  const backend = e.selector_backend || 'rule';
  const modelPart = e.model_candidate
    ? ` | model ${e.model_candidate}, confidence ${fmt(e.model_confidence)}`
    : '';
  const top = dominantFeature(e);
  const signalPart = top ? ` | dominant ${top.def.short} ${fmt(top.value)}` : '';
  document.getElementById('decisionSummary').textContent =
    `${title} because ${e.reason} (${backend})${modelPart}${signalPart}`;
  document.getElementById('featureBars').innerHTML = featureDefs.slice(0, 5).map(def => {
    const value = featureValue(e, def.key);
    const pct = featureBarPct(def, value);
    const scale = def.scale === 'log' ? ' log' : '';
    return `<div class="feature-row"><span>${def.label}</span><div class="feature-track" title="${def.label}${scale}"><div class="feature-fill" style="width:${pct}%"></div></div><span>${fmt(value)}</span></div>`;
  }).join('');
}

function renderInternalMap(a, w, lastEvent) {
  const adaptiveOnline = !!(a && a.current_mode);
  const current = adaptiveOnline ? a.current_mode : 'n/a';
  const profile = policyProfiles[current] || {
    intent: adaptiveOnline ? 'custom mode policy' : 'non-adaptive strategy',
    release: adaptiveOnline ? 'mode release decision' : 'strategy-owned free path',
    chips: adaptiveOnline ? ['policy', 'shared substrate'] : ['viewer only']
  };
  const storageAlloc = numericArray(a.storage_allocs, 3);
  const storageFree = numericArray(a.storage_frees, 3);
  const storageRequested = numericArray(a.storage_requested_bytes, 3);
  const storageUsable = numericArray(a.storage_usable_bytes, 3);
  const poolHits = numericArray(a.pool_hits, 3);
  const poolMisses = numericArray(a.pool_misses, 3);
  const modeAlloc = numericArray(a.mode_alloc_count, modes.length);
  const modeLive = numericArray(a.mode_live_bytes, modes.length);
  const totalStorageAlloc = Math.max(1, sum(storageAlloc));
  const totalPool = sum(poolHits) + sum(poolMisses);
  const hitRate = totalPool ? sum(poolHits) / totalPool : 0;
  const activeIdx = modeIndex[current] ?? -1;
  const activeModeAllocs = activeIdx >= 0 ? modeAlloc[activeIdx] : 0;
  const activeModeLive = activeIdx >= 0 ? modeLive[activeIdx] : 0;
  const remoteRatio = Number(a.remote_free_ratio || 0);
  const releasedBytes = Number(a.release_unmapped_bytes || 0);
  const mappedBytes = Number(a.mapped_bytes || 0);
  const safetyErrors = Number(a.invalid_free_count || 0)
    + Number(a.double_free_count || 0)
    + Number(a.header_corruption_count || 0);
  const selectorConfidence = lastEvent ? Number(lastEvent.model_confidence || 0) : 0;
  const releasePressure = releasedBytes + mappedBytes > 0
    ? releasedBytes / (releasedBytes + mappedBytes)
    : 0;

  const chipHtml = profile.chips
    .map(chip => `<span class="node-chip on">${chip}</span>`)
    .join('');

  setText('substrateHealth', adaptiveOnline ? 'adaptive substrate online' : 'generic workload view');
  document.getElementById('substrateHealth').classList.toggle('hot', adaptiveOnline);
  setText('apiFlow', `${fmt(w.ops || 0)} ops | ${fmt(w.allocs || 0)} allocs`);
  setText('headerFlow', adaptiveOnline
    ? `mode_id ${current} | live ${fmt(w.live_objects || 0)}`
    : 'strategy-owned metadata');
  setText('policyMode', current);
  setText('policyIntent', profile.intent);
  document.getElementById('policyChips').innerHTML = chipHtml;
  setText('releaseAction', profile.release);
  setText('releaseStats', `${bytes(releasedBytes)} returned | retired ${fmt(a.retired_mode_count || 0)}`);
  setNodeState('policyNode', adaptiveOnline ? 'active' : '');
  setNodeState('releaseNode', current === 'compact_rss' || releasePressure > 0.08 ? 'warn' : '');

  const storageNodes = [
    ['sizeClass', 'sizeClassNode', 'SizeClass', 0],
    ['span', 'spanNode', 'Span', 1],
    ['direct', 'directNode', 'DirectMap', 2]
  ];
  storageNodes.forEach(([prefix, nodeId, label, idx]) => {
    const share = storageAlloc[idx] / totalStorageAlloc;
    const util = storageUsable[idx] > 0
      ? Math.min(1, storageRequested[idx] / storageUsable[idx])
      : 0;
    const reuse = poolHits[idx] + poolMisses[idx] > 0
      ? poolHits[idx] / (poolHits[idx] + poolMisses[idx])
      : 0;
    setText(`${prefix}Value`, `${fmt(storageAlloc[idx])} allocs`);
    setText(`${prefix}Caption`, `${label} share ${fmt(share * 100)}% | reuse ${fmt(reuse * 100)}% | fit ${fmt(util * 100)}%`);
    setMeter(`${prefix}Meter`, share * 100);
    const directHot = idx === 2 && (current === 'large_object' || current === 'hardened_debug');
    const spanHot = idx === 1 && current === 'fragmentation_stable';
    const sizeHot = idx === 0 && (current === 'throughput_cache' || current === 'balanced');
    setNodeState(nodeId, share > 0.42 || directHot || spanHot || sizeHot ? 'hot' : '');
  });

  setText('cacheValue', `${fmt(hitRate * 100)}% hit`);
  setMeter('cacheMeter', hitRate * 100);
  setNodeState('cacheNode', current === 'throughput_cache' || hitRate > 0.45 ? 'hot' : '');

  setText('remoteValue', `${fmt(remoteRatio * 100)}% | ${fmt(w.remote_frees || 0)} frees`);
  setMeter('remoteMeter', pct(remoteRatio, 0.35));
  setNodeState('remoteNode', current === 'cross_thread' || remoteRatio > 0.10 ? 'hot' : '');

  setText('reclaimValue', bytes(releasedBytes));
  setText('reclaimCaption', `${fmt(a.released_pages || 0)} pages, ${fmt(a.released_spans || 0)} spans released`);
  setMeter('reclaimMeter', pct(releasePressure, 0.35));
  setNodeState('reclaimNode', current === 'compact_rss' || releasePressure > 0.08 ? 'warn' : '');

  setText('debugValue', `${fmt(safetyErrors)} errors`);
  setMeter('debugMeter', Math.min(100, safetyErrors * 12));
  setNodeState('debugNode', safetyErrors > 0 ? 'alert' : current === 'hardened_debug' ? 'hot' : '');

  const selectorText = lastEvent
    ? `${lastEvent.selector_backend || 'selector'} -> ${lastEvent.candidate_mode || 'n/a'}`
    : 'waiting';
  setText('selectorValue', selectorText);
  setText('selectorCaption', lastEvent
    ? `${lastEvent.switched ? 'switched' : 'held'} | ${lastEvent.reason || 'no reason'} | active allocs ${fmt(activeModeAllocs)}`
    : `active live bytes ${bytes(activeModeLive)}`);
  setMeter('selectorMeter', selectorConfidence ? selectorConfidence * 100 : (adaptiveOnline ? 35 : 0));
  setNodeState('selectorNode', lastEvent && lastEvent.switched ? 'active' : adaptiveOnline ? 'hot' : '');
}

async function poll() {
  try {
    const r = await fetch('/snapshot', { cache: 'no-store' });
    const j = await r.json();
    const w = j.workload || {};
    const a = j.adaptive || {};
    const g = j.generated || {};

    document.getElementById('runState').textContent = w.running ? 'running' : 'finished';
    document.getElementById('runState').style.background = w.running ? '#f5f5f1' : '#07090c';
    document.getElementById('runState').style.color = w.running ? '#030405' : '#8e98a5';
    document.getElementById('missionTime').textContent = missionTime(w.elapsed_ms);
    document.getElementById('currentMode').textContent = a.current_mode || 'n/a';
    document.getElementById('modeTransition').textContent = `previous ${a.previous_mode || 'n/a'} | switches ${fmt(a.mode_switches)}`;
    renderModeRail(a.current_mode, a.previous_mode);
    document.getElementById('phaseName').textContent = `${w.phase_index + 1}/${w.phase_count} ${w.phase || '-'}`;
    document.getElementById('phaseFill').style.width = `${Math.max(0, Math.min(100, (w.phase_progress || 0) * 100))}%`;
    phaseCells(w.phase_index || 0, w.phase_count || 1);

    const currentTps = Number(w.window_ops_per_sec || w.ops_per_sec || 0);
    document.getElementById('ops').textContent = fmt(currentTps);
    document.getElementById('liveBytes').textContent = fmt(w.live_bytes);
    document.getElementById('mappedLive').textContent = fmt(a.mapped_live_ratio);
    document.getElementById('validationErrors').textContent = fmt(g.validation_errors || 0);

    const raw = {
      ops: currentTps,
      live: (w.live_bytes || 0) / 1024,
      mapped: (a.mapped_bytes || 0) / 1024,
      ratio: a.mapped_live_ratio || 0
    };
    if (!smoothSample) {
      smoothSample = raw;
    } else {
      const alpha = 0.28;
      smoothSample = {
        ops: smoothSample.ops * (1 - alpha) + raw.ops * alpha,
        live: smoothSample.live * (1 - alpha) + raw.live * alpha,
        mapped: smoothSample.mapped * (1 - alpha) + raw.mapped * alpha,
        ratio: smoothSample.ratio * (1 - alpha) + raw.ratio * alpha
      };
    }
    hist.push(smoothSample);
    if (hist.length > maxN) hist.shift();
    drawCharts();

    renderDecision(j.selector_last_window);
    renderTimeline(j.selector_events || []);
    renderInternalMap(a, w, j.selector_last_window);
    renderComparison(w, a, currentTps);

    document.getElementById('workloadTable').innerHTML = rows([
      ['strategy', w.strategy],
      ['template', w.template],
      ['alloc/free/realloc', `${fmt(w.allocs)} / ${fmt(w.frees)} / ${fmt(w.reallocs)}`],
      ['remote frees', w.remote_frees],
      ['requested bytes', w.requested_bytes],
      ['peak live bytes', w.peak_live_bytes],
      ['validation checks', g.validation_checks || 0]
    ]);
    document.getElementById('adaptiveTable').innerHTML = rows([
      ['stream-large >256 KiB', a.large_bytes_ratio],
      ['remote free ratio', a.remote_free_ratio],
      ['fragmentation', a.fragmentation_estimate],
      ['slow path ratio', a.slow_path_ratio],
      ['size entropy', a.size_entropy],
      ['invalid/double free', `${fmt(a.invalid_free_count || 0)} / ${fmt(a.double_free_count || 0)}`]
    ]);
  } catch (e) {
    document.getElementById('decisionSummary').textContent = 'Telemetry link interrupted.';
  }
}

setInterval(poll, 300);
initModeRail();
loadComparison();
poll();
