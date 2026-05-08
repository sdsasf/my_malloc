const hist = [];
const maxN = 260;
let smoothSample = null;
const modes = [
  ['balanced', 'balanced'],
  ['throughput_cache', 'throughput'],
  ['deterministic_latency', 'latency'],
  ['compact_rss', 'compact'],
  ['fragmentation_stable', 'fragment'],
  ['cross_thread', 'x-thread'],
  ['large_object', 'large'],
  ['hardened_debug', 'debug']
];
let lastMode = '';

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

function missionTime(ms) {
  const total = Math.max(0, Number(ms || 0) / 1000);
  const min = Math.floor(total / 60);
  const sec = total - min * 60;
  return `T+${String(min).padStart(2, '0')}:${sec.toFixed(1).padStart(4, '0')}`;
}

function rows(items) {
  return items.map(x => `<tr><td>${x[0]}</td><td>${fmt(x[1])}</td></tr>`).join('');
}

function featureValue(e, key) {
  return Math.max(0, Number((e && e[key]) || 0));
}

function importantValue(e) {
  if (!e) return '';
  const map = {
    large_bytes_ratio: ['large bytes', e.large_bytes_ratio],
    remote_free_ratio: ['remote free', e.remote_free_ratio],
    mapped_live_ratio: ['mapped/live', e.mapped_live_ratio],
    slow_path_ratio: ['slow path', e.slow_path_ratio],
    fragmentation: ['fragmentation', e.fragmentation_estimate],
    cache_reuse: ['cache hit', e.cache_hit_rate]
  };
  const row = map[e.reason] || ['mapped/live', e.mapped_live_ratio];
  return `${row[0]} ${fmt(row[1])}`;
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
    return `<div class="event ${e.switched ? 'switched' : ''}">
      <div class="event-title">${title}</div>
      <div class="event-reason">${backend} | reason ${fmt(e.reason)}</div>
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
  document.getElementById('decisionSummary').textContent =
    `${title} because ${e.reason} (${backend})${modelPart}`;
  const features = [
    ['large bytes', featureValue(e, 'large_bytes_ratio'), 1],
    ['remote free', featureValue(e, 'remote_free_ratio'), 1],
    ['mapped/live', featureValue(e, 'mapped_live_ratio'), 12],
    ['slow path', featureValue(e, 'slow_path_ratio'), 1],
    ['fragmentation', featureValue(e, 'fragmentation_estimate'), 1]
  ];
  document.getElementById('featureBars').innerHTML = features.map(([name, value, max]) => {
    const pct = Math.max(0, Math.min(100, value / max * 100));
    return `<div class="feature-row"><span>${name}</span><div class="feature-track"><div class="feature-fill" style="width:${pct}%"></div></div><span>${fmt(value)}</span></div>`;
  }).join('');
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

    document.getElementById('ops').textContent = fmt(w.ops_per_sec);
    document.getElementById('liveBytes').textContent = fmt(w.live_bytes);
    document.getElementById('mappedLive').textContent = fmt(a.mapped_live_ratio);
    document.getElementById('validationErrors').textContent = fmt(g.validation_errors || 0);

    const raw = {
      ops: w.ops_per_sec || 0,
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
      ['large bytes ratio', a.large_bytes_ratio],
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
poll();
