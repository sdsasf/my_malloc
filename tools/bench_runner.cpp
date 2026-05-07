#include "strategy_loader.h"
#include "my_ptmalloc/adaptive_allocator.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/resource.h>
#include <thread>
#include <time.h>
#include <unistd.h>
#include <vector>

using my_ptmalloc::StrategyDescriptor;
using my_ptmalloc::tools::LoadedStrategy;
using my_ptmalloc::tools::load_strategy;
using my_ptmalloc::tools::unload_strategy;

struct BenchResult {
    std::string name;
    double ms;
    size_t ops;
    size_t peak_rss_kb;
    std::string extra_json;
};

struct BenchConfig {
    std::string strategy = "hybrid";
    std::string profile = "micro";
    std::vector<std::string> benches;
    bool json = false;
    size_t size = 64;
    size_t min_size = 16;
    size_t max_size = 8192;
    int iters = 1000000;
    int random_iters = 200000;
    int slots = 4096;
    int batch = 512;
    int rounds = 1000;
    int threads = 4;
    int repeats = 1;
    unsigned seed = 12345;
    std::string workload_template = "adaptive_mix";
    bool workload_realtime = false;
    int phase_ms = 10000;
    int target_ops_per_sec = 50000;
    int phase_repeat = 1;
    int telemetry_port = 0;
    int telemetry_hold_ms = 300000;
};

struct RepeatSummary {
    double mean;
    double median;
    double p95;
    double stddev;
    double min;
    double max;
};

static double now_ms() {
    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static size_t peak_rss_kb() {
    rusage ru{};
    getrusage(RUSAGE_SELF, &ru);
    return static_cast<size_t>(ru.ru_maxrss);
}

struct WorkloadSnapshot {
    char strategy[32]{};
    char benchmark[64]{};
    char template_name[64]{};
    char phase[64]{};
    int phase_index = 0;
    int phase_count = 0;
    uint64_t ops = 0;
    uint64_t allocs = 0;
    uint64_t frees = 0;
    uint64_t reallocs = 0;
    uint64_t remote_frees = 0;
    uint64_t requested_bytes = 0;
    uint64_t live_objects = 0;
    uint64_t live_bytes = 0;
    uint64_t peak_live_bytes = 0;
    double elapsed_ms = 0.0;
    double phase_elapsed_ms = 0.0;
    double phase_duration_ms = 0.0;
    double phase_progress = 0.0;
    bool running = false;
};

std::mutex g_workload_snapshot_mutex;
WorkloadSnapshot g_workload_snapshot;
double g_workload_start_ms = 0.0;

static void set_workload_identity(const char* strategy,
                                  const char* benchmark,
                                  const char* template_name) {
    std::lock_guard<std::mutex> lock(g_workload_snapshot_mutex);
    std::snprintf(g_workload_snapshot.strategy, sizeof(g_workload_snapshot.strategy), "%s", strategy);
    std::snprintf(g_workload_snapshot.benchmark, sizeof(g_workload_snapshot.benchmark), "%s", benchmark);
    std::snprintf(g_workload_snapshot.template_name, sizeof(g_workload_snapshot.template_name), "%s", template_name);
}

static void update_workload_snapshot(const char* phase,
                                     int phase_index,
                                     int phase_count,
                                     uint64_t ops,
                                     uint64_t allocs,
                                     uint64_t frees,
                                     uint64_t reallocs,
                                     uint64_t remote_frees,
                                     uint64_t requested_bytes,
                                     uint64_t live_objects,
                                     uint64_t live_bytes,
                                     uint64_t peak_live_bytes,
                                     double phase_elapsed_ms,
                                     double phase_duration_ms,
                                     bool running) {
    std::lock_guard<std::mutex> lock(g_workload_snapshot_mutex);
    std::snprintf(g_workload_snapshot.phase, sizeof(g_workload_snapshot.phase), "%s", phase);
    g_workload_snapshot.phase_index = phase_index;
    g_workload_snapshot.phase_count = phase_count;
    g_workload_snapshot.ops = ops;
    g_workload_snapshot.allocs = allocs;
    g_workload_snapshot.frees = frees;
    g_workload_snapshot.reallocs = reallocs;
    g_workload_snapshot.remote_frees = remote_frees;
    g_workload_snapshot.requested_bytes = requested_bytes;
    g_workload_snapshot.live_objects = live_objects;
    g_workload_snapshot.live_bytes = live_bytes;
    g_workload_snapshot.peak_live_bytes = peak_live_bytes;
    g_workload_snapshot.elapsed_ms = g_workload_start_ms > 0.0 ? now_ms() - g_workload_start_ms : 0.0;
    g_workload_snapshot.phase_elapsed_ms = phase_elapsed_ms;
    g_workload_snapshot.phase_duration_ms = phase_duration_ms;
    g_workload_snapshot.phase_progress = phase_duration_ms > 0.0
        ? std::min(1.0, phase_elapsed_ms / phase_duration_ms)
        : 0.0;
    g_workload_snapshot.running = running;
}

static WorkloadSnapshot workload_snapshot_copy() {
    std::lock_guard<std::mutex> lock(g_workload_snapshot_mutex);
    return g_workload_snapshot;
}

static void touch_bytes(void* p, size_t size, unsigned char value) {
    if (!p) return;
    std::memset(p, value, std::min<size_t>(size, 64));
}

static std::string telemetry_snapshot_json() {
    WorkloadSnapshot w = workload_snapshot_copy();
    std::ostringstream os;
    double ops_sec = w.elapsed_ms > 0.0 ? static_cast<double>(w.ops) / (w.elapsed_ms / 1000.0) : 0.0;
    os << "{\"workload\":{\"strategy\":\"" << w.strategy
       << "\",\"benchmark\":\"" << w.benchmark
       << "\",\"template\":\"" << w.template_name
       << "\",\"phase\":\"" << w.phase
       << "\",\"phase_index\":" << w.phase_index
       << ",\"phase_count\":" << w.phase_count
       << ",\"running\":" << (w.running ? "true" : "false")
       << ",\"elapsed_ms\":" << w.elapsed_ms
       << ",\"ops\":" << w.ops
       << ",\"ops_per_sec\":" << ops_sec
       << ",\"allocs\":" << w.allocs
       << ",\"frees\":" << w.frees
       << ",\"reallocs\":" << w.reallocs
       << ",\"remote_frees\":" << w.remote_frees
       << ",\"requested_bytes\":" << w.requested_bytes
       << ",\"live_objects\":" << w.live_objects
       << ",\"live_bytes\":" << w.live_bytes
       << ",\"peak_live_bytes\":" << w.peak_live_bytes
       << ",\"phase_elapsed_ms\":" << w.phase_elapsed_ms
       << ",\"phase_duration_ms\":" << w.phase_duration_ms
       << ",\"phase_progress\":" << w.phase_progress
       << ",\"peak_rss_kb\":" << peak_rss_kb()
       << "}";
    if (std::strcmp(w.strategy, "adaptive") == 0) {
        auto s = my_ptmalloc::adaptive_stats_snapshot();
        os << ",\"adaptive\":{\"current_mode\":\"" << my_ptmalloc::adaptive_mode_name(s.current_mode)
           << "\",\"previous_mode\":\"" << my_ptmalloc::adaptive_mode_name(s.previous_mode)
           << "\",\"mode_switches\":" << s.mode_switches
           << ",\"mapped_bytes\":" << s.mapped_bytes
           << ",\"live_bytes\":" << s.live_bytes
           << ",\"mapped_live_ratio\":" << s.mapped_live_ratio
           << ",\"remote_free_ratio\":" << s.remote_free_ratio
           << ",\"size_entropy\":" << s.size_entropy
           << ",\"large_bytes_ratio\":" << s.large_bytes_ratio
           << ",\"fragmentation_estimate\":" << s.fragmentation_estimate
           << ",\"slow_path_ratio\":" << s.slow_path_ratio
           << ",\"double_free_count\":" << s.double_free_count
           << ",\"invalid_free_count\":" << s.invalid_free_count
           << "}";
    }
    os << "}";
    return os.str();
}

static const char* telemetry_html() {
    return R"HTML(<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>adaptive workload viewer</title>
<style>
:root{color-scheme:dark;--bg:#050505;--panel:#0b0c0e;--panel2:#101113;--line:#2b2f35;--line2:#59616b;--text:#f7f7f3;--muted:#a4abb3;--amber:#f0b35b;--gold:#f5d07a;--blue:#a9d8ff;--green:#d7f2dc;--red:#ff7d70}
*{box-sizing:border-box}body{font-family:system-ui,-apple-system,Segoe UI,sans-serif;margin:0;background:#050505;color:var(--text)}
header{padding:18px 24px;background:#050505;border-bottom:1px solid var(--line);letter-spacing:.04em}
h1{font-size:18px;margin:0;font-weight:650}.sub{color:var(--muted);font-size:12px;margin-top:5px;text-transform:uppercase}
.wrap{padding:16px;max-width:1320px;margin:0 auto}.grid{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:12px}
.card{background:#0b0c0e;border:1px solid var(--line);border-radius:4px;padding:14px;box-shadow:0 18px 38px rgba(0,0,0,.28)}
.hero{grid-column:span 4;display:grid;grid-template-columns:1.25fr 1fr;gap:18px;align-items:center}
.label{color:var(--muted);font-size:11px;text-transform:uppercase;letter-spacing:.08em}.value{font-size:24px;margin-top:5px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;font-variant-numeric:tabular-nums}
.big{font-size:36px}.pill{display:inline-block;border:1px solid var(--line2);border-radius:2px;padding:5px 10px;color:#f7f7f3;background:#070707;font-size:12px;margin-right:6px;text-transform:uppercase}
.phase-track{height:14px;background:#060606;border:1px solid var(--line2);border-radius:2px;overflow:hidden;margin-top:12px}.phase-fill{height:100%;width:0;background:#f7f7f3;box-shadow:0 0 18px rgba(255,255,255,.24)}
.phase-row{display:grid;grid-template-columns:repeat(6,1fr);gap:5px;margin-top:12px}.phase-cell{height:8px;border-radius:2px;background:#15171a;border:1px solid #2b2f35}.phase-cell.active{background:#f7f7f3;border-color:#f7f7f3}
canvas{width:100%;height:300px;background:#030303;border:1px solid var(--line);border-radius:4px}
.wide{grid-column:span 4}.span2{grid-column:span 2}.legend{display:flex;gap:16px;align-items:center;flex-wrap:wrap;color:var(--muted);font-size:12px;margin-top:10px}.dot{display:inline-block;width:10px;height:10px;border-radius:50%;margin-right:5px}.hint{margin-left:auto;color:#657789}
table{width:100%;border-collapse:collapse}td{padding:7px 0;border-bottom:1px solid #172332}td:first-child{color:var(--muted);text-transform:uppercase;font-size:11px;letter-spacing:.05em}td:last-child{text-align:right;color:#e8f3ff;font-variant-numeric:tabular-nums}
@media(max-width:800px){.grid{grid-template-columns:1fr}.hero,.wide,.span2{grid-column:span 1}.hero{grid-template-columns:1fr}.value{font-size:20px}.big{font-size:28px}}
</style>
</head>
<body>
<header><h1>Adaptive Workload Telemetry</h1><div class="sub">Realtime generated load, mode selection, and memory substrate signals</div></header>
<div class="wrap"><div class="grid">
<div class="card hero">
  <div>
    <div><span id="running" class="pill">starting</span><span id="modepill" class="pill">mode -</span></div>
    <div class="label" style="margin-top:14px">current phase</div>
    <div id="phase" class="value big">-</div>
    <div class="phase-track"><div id="phasebar" class="phase-fill"></div></div>
    <div id="phases" class="phase-row"></div>
  </div>
  <div>
    <div class="label">strategy</div><div id="strategy" class="value">-</div>
    <div class="label" style="margin-top:14px">template</div><div id="template" class="value">-</div>
  </div>
</div>
<div class="card"><div class="label">ops/sec</div><div id="ops" class="value">0</div></div>
<div class="card"><div class="label">live bytes</div><div id="live" class="value">0</div></div>
<div class="card"><div class="label">current / previous mode</div><div id="modes" class="value">-</div></div>
<div class="card"><div class="label">peak RSS KB</div><div id="rss" class="value">0</div></div>
<div class="card wide"><canvas id="chart" width="1180" height="300"></canvas><div class="legend"><span><i class="dot" style="background:#f7f7f3"></i>throughput trend</span><span><i class="dot" style="background:#a4abb3"></i>live memory trend</span><span><i class="dot" style="background:#f0b35b"></i>mapped memory trend</span><span class="hint">normalized with headroom so shape is visible</span></div></div>
<div class="card span2"><table id="workloadMetrics"></table></div>
<div class="card span2"><table id="adaptiveMetrics"></table></div>
</div>
</div>
<script>
const hist=[]; const maxN=240;
function fmt(n){if(n===undefined||n===null)return 'n/a';if(typeof n==='boolean')return n?'yes':'no';if(typeof n==='string')return n;if(!Number.isFinite(Number(n)))return String(n);return Number(n).toLocaleString(undefined,{maximumFractionDigits:2});}
function draw(){
 const c=document.getElementById('chart'),ctx=c.getContext('2d');ctx.clearRect(0,0,c.width,c.height);
 ctx.fillStyle='#030303';ctx.fillRect(0,0,c.width,c.height);
 const top=30,bottom=26,left=48,right=16,h=c.height-top-bottom,w=c.width-left-right;
 ctx.strokeStyle='#1d2024';ctx.lineWidth=1;
 for(let i=0;i<=4;i++){const y=top+i*h/4;ctx.beginPath();ctx.moveTo(left,y);ctx.lineTo(left+w,y);ctx.stroke();}
 for(let i=0;i<=6;i++){const x=left+i*w/6;ctx.beginPath();ctx.moveTo(x,top);ctx.lineTo(x,top+h);ctx.stroke();}
 ctx.fillStyle='#a4abb3';ctx.font='12px system-ui';ctx.fillText('NORMALIZED TELEMETRY TRENDS',left,18);
 function range(key){const vals=hist.map(p=>p[key]||0);let mn=Math.min(...vals),mx=Math.max(...vals);if(!Number.isFinite(mn)||!Number.isFinite(mx)||mx<=mn){mn=0;mx=Math.max(1,mx||1);}return [mn,mx];}
 function line(key,color){const [mn,mx]=range(key);ctx.beginPath();ctx.strokeStyle=color;ctx.lineWidth=2.5;hist.forEach((p,i)=>{const x=left+i*(w/Math.max(1,maxN-1));const v=((p[key]||0)-mn)/(mx-mn);const y=top+h-(0.08+v*0.84)*h;if(i)ctx.lineTo(x,y);else ctx.moveTo(x,y);});ctx.stroke();}
 line('mapped','#f0b35b');line('live','#a4abb3');line('ops','#f7f7f3');
 const last=hist[hist.length-1]; if(last){ctx.fillStyle='#a4abb3';ctx.fillText('latest ops '+fmt(last.ops)+' | live '+fmt(last.live)+' KB | mapped '+fmt(last.mapped)+' KB',left,c.height-8);}
}
function phaseCells(index,count){
 let html=''; const n=Math.max(1,Math.min(24,count||1));
 for(let i=0;i<n;i++){const active=i<=index*n/Math.max(1,count);html+='<div class="phase-cell '+(active?'active':'')+'"></div>';}
 document.getElementById('phases').innerHTML=html;
}
function rows(items){return items.map(x=>'<tr><td>'+x[0]+'</td><td>'+fmt(x[1])+'</td></tr>').join('');}
function setRunPill(running){
 const el=document.getElementById('running');el.textContent=running?'running':'finished';el.style.background=running?'#f7f7f3':'#070707';el.style.color=running?'#050505':'#a4abb3';
}
async function poll(){
 try{
  const r=await fetch('/snapshot',{cache:'no-store'}); const j=await r.json(); const w=j.workload, a=j.adaptive||{};
  document.getElementById('strategy').textContent=w.strategy;
  document.getElementById('template').textContent=w.template;
  document.getElementById('modepill').textContent=a.current_mode?'mode '+a.current_mode:'mode n/a';
  setRunPill(!!w.running);
  document.getElementById('phase').textContent=(w.phase_index+1)+'/'+w.phase_count+' '+w.phase;
  document.getElementById('phasebar').style.width=Math.max(0,Math.min(100,(w.phase_progress||0)*100))+'%';
  phaseCells(w.phase_index,w.phase_count);
  document.getElementById('ops').textContent=fmt(w.ops_per_sec);
  document.getElementById('live').textContent=fmt(w.live_bytes);
  document.getElementById('modes').textContent=(a.current_mode||'n/a')+' / '+(a.previous_mode||'n/a');
  document.getElementById('rss').textContent=fmt(w.peak_rss_kb);
  hist.push({ops:w.ops_per_sec||0,live:(w.live_bytes||0)/1024,mapped:(a.mapped_bytes||0)/1024}); if(hist.length>maxN)hist.shift(); draw();
  document.getElementById('workloadMetrics').innerHTML=rows([['phase elapsed ms',w.phase_elapsed_ms],['phase duration ms',w.phase_duration_ms],['alloc/free/realloc',w.allocs+' / '+w.frees+' / '+w.reallocs],['remote frees',w.remote_frees],['requested bytes',w.requested_bytes],['peak live bytes',w.peak_live_bytes]]);
  document.getElementById('adaptiveMetrics').innerHTML=rows([['mode switches',a.mode_switches],['mapped/live',a.mapped_live_ratio],['remote ratio',a.remote_free_ratio],['large ratio',a.large_bytes_ratio],['fragmentation',a.fragmentation_estimate],['slow path',a.slow_path_ratio],['safety errors',(a.invalid_free_count||0)+' / '+(a.double_free_count||0)]]);
 }catch(e){}
}
setInterval(poll,250); poll();
</script>
</body>
</html>)HTML";
}

class TelemetryServer {
public:
    bool start(int port) {
        if (port <= 0) return true;
        fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (fd_ < 0) return false;
        int yes = 1;
        setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = htons(static_cast<uint16_t>(port));
        if (bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 || listen(fd_, 8) != 0) {
            close(fd_);
            fd_ = -1;
            return false;
        }
        running_.store(true, std::memory_order_relaxed);
        thread_ = std::thread([this] { run(); });
        return true;
    }

    void stop() {
        running_.store(false, std::memory_order_relaxed);
        if (fd_ >= 0) {
            shutdown(fd_, SHUT_RDWR);
        }
        if (thread_.joinable()) thread_.join();
        if (fd_ >= 0) close(fd_);
        fd_ = -1;
    }

    ~TelemetryServer() { stop(); }

private:
    void run() {
        while (running_.load(std::memory_order_relaxed)) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(fd_, &readfds);
            timeval tv{0, 200000};
            int ready = select(fd_ + 1, &readfds, nullptr, nullptr, &tv);
            if (ready <= 0) continue;
            int client = accept(fd_, nullptr, nullptr);
            if (client < 0) continue;
            handle_client(client);
            close(client);
        }
    }

    void handle_client(int client) {
        std::string req;
        char buf[1024];
        while (req.find("\r\n\r\n") == std::string::npos && req.size() < 8192) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(client, &readfds);
            timeval tv{0, 200000};
            int ready = select(client + 1, &readfds, nullptr, nullptr, &tv);
            if (ready <= 0) break;
            ssize_t n = recv(client, buf, sizeof(buf), 0);
            if (n <= 0) break;
            req.append(buf, static_cast<size_t>(n));
        }
        if (req.empty()) return;
        bool snapshot = req.compare(0, 13, "GET /snapshot") == 0;
        bool favicon = req.compare(0, 16, "GET /favicon.ico") == 0;
        if (favicon) {
            const char* response =
                "HTTP/1.1 204 No Content\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";
            send_all(client, response, std::strlen(response));
            return;
        }
        std::string body = snapshot ? telemetry_snapshot_json() : std::string(telemetry_html());
        const char* type = snapshot ? "application/json" : "text/html; charset=utf-8";
        char header[256];
        std::snprintf(header, sizeof(header),
                      "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nCache-Control: no-store\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                      type, body.size());
        send_all(client, header, std::strlen(header));
        send_all(client, body.data(), body.size());
    }

    static void send_all(int fd, const char* data, size_t size) {
        size_t sent = 0;
        while (sent < size) {
            ssize_t n = send(fd, data + sent, size - sent, MSG_NOSIGNAL);
            if (n <= 0) return;
            sent += static_cast<size_t>(n);
        }
    }

    int fd_ = -1;
    std::atomic<bool> running_{false};
    std::thread thread_;
};

static BenchResult same_size(const StrategyDescriptor& s, size_t size, int iters) {
    volatile unsigned char acc = 0;
    double start = now_ms();
    for (int i = 0; i < iters; ++i) {
        void* p = s.vtable.allocate(size);
        touch_bytes(p, size, 0xCD);
        if (p) acc += *static_cast<unsigned char*>(p);
        s.vtable.deallocate(p);
    }
    double end = now_ms();
    char name[64];
    std::snprintf(name, sizeof(name), "same_size_%zu", size);
    return {name, end - start, static_cast<size_t>(iters), peak_rss_kb()};
}

static BenchResult batch_workload(const StrategyDescriptor& s, int batch, int rounds) {
    double start = now_ms();
    for (int r = 0; r < rounds; ++r) {
        std::vector<void*> ptrs(batch);
        for (int i = 0; i < batch; ++i) {
            size_t size = 128 + i % 512;
            ptrs[i] = s.vtable.allocate(size);
            touch_bytes(ptrs[i], size, 0xEF);
        }
        for (void* p : ptrs) s.vtable.deallocate(p);
    }
    double end = now_ms();
    return {"batch", end - start, static_cast<size_t>(batch) * rounds, peak_rss_kb()};
}

static BenchResult random_workload(const StrategyDescriptor& s,
                                   int iters,
                                   int slots,
                                   size_t min_size,
                                   size_t max_size,
                                   unsigned seed) {
    std::vector<void*> ptrs(slots, nullptr);
    std::mt19937 rng(seed);
    std::uniform_int_distribution<size_t> size_dist(min_size, max_size);
    std::uniform_int_distribution<int> action_dist(0, 4);
    double start = now_ms();
    for (int i = 0; i < iters; ++i) {
        int idx = static_cast<int>(rng() % slots);
        int action = action_dist(rng);
        if (!ptrs[idx] || action <= 2) {
            void* p = s.vtable.allocate(size_dist(rng));
            touch_bytes(p, 16, 0xAB);
            if (ptrs[idx]) s.vtable.deallocate(ptrs[idx]);
            ptrs[idx] = p;
        } else if (action == 3) {
            ptrs[idx] = s.vtable.reallocate(ptrs[idx], size_dist(rng));
        } else {
            s.vtable.deallocate(ptrs[idx]);
            ptrs[idx] = nullptr;
        }
    }
    for (void* p : ptrs) if (p) s.vtable.deallocate(p);
    double end = now_ms();
    return {"random", end - start, static_cast<size_t>(iters), peak_rss_kb()};
}

static BenchResult fragmentation_workload(const StrategyDescriptor& s,
                                          int iters,
                                          int slots,
                                          size_t min_size,
                                          size_t max_size,
                                          unsigned seed) {
    std::vector<void*> ptrs(slots, nullptr);
    std::vector<size_t> sizes(slots, 0);
    std::mt19937 rng(seed);
    std::uniform_int_distribution<size_t> size_dist(min_size, max_size);
    for (int i = 0; i < slots; ++i) {
        sizes[i] = size_dist(rng);
        ptrs[i] = s.vtable.allocate(sizes[i]);
        touch_bytes(ptrs[i], sizes[i], 0xA5);
    }

    double start = now_ms();
    for (int i = 0; i < iters; ++i) {
        int idx = static_cast<int>(rng() % slots);
        size_t next_size = size_dist(rng);
        void* next = s.vtable.reallocate(ptrs[idx], next_size);
        if (next) {
            ptrs[idx] = next;
            sizes[idx] = next_size;
            touch_bytes(ptrs[idx], sizes[idx], 0x5A);
        } else {
            s.vtable.deallocate(ptrs[idx]);
            ptrs[idx] = nullptr;
            sizes[idx] = 0;
        }
    }
    for (void* p : ptrs) if (p) s.vtable.deallocate(p);
    double end = now_ms();
    return {"fragmentation", end - start, static_cast<size_t>(iters), peak_rss_kb()};
}

static BenchResult cross_thread_free_workload(const StrategyDescriptor& s,
                                              int threads,
                                              int per_thread,
                                              size_t min_size,
                                              size_t max_size,
                                              unsigned seed) {
    std::vector<std::vector<void*>> ptrs(static_cast<size_t>(threads));
    for (auto& v : ptrs) v.resize(static_cast<size_t>(per_thread), nullptr);

    double start = now_ms();
    std::vector<std::thread> producers;
    for (int t = 0; t < threads; ++t) {
        producers.emplace_back([&, t] {
            std::mt19937 rng(seed + static_cast<unsigned>(t));
            std::uniform_int_distribution<size_t> size_dist(min_size, max_size);
            for (int i = 0; i < per_thread; ++i) {
                size_t size = size_dist(rng);
                ptrs[static_cast<size_t>(t)][static_cast<size_t>(i)] = s.vtable.allocate(size);
                touch_bytes(ptrs[static_cast<size_t>(t)][static_cast<size_t>(i)], size, 0xC3);
            }
        });
    }
    for (auto& th : producers) th.join();

    std::vector<std::thread> consumers;
    for (int t = 0; t < threads; ++t) {
        consumers.emplace_back([&, t] {
            int victim = (t + threads - 1) % threads;
            for (void* p : ptrs[static_cast<size_t>(victim)]) {
                s.vtable.deallocate(p);
            }
        });
    }
    for (auto& th : consumers) th.join();
    double end = now_ms();
    return {"cross_thread_free", end - start, static_cast<size_t>(threads) * per_thread * 2, peak_rss_kb()};
}

static BenchResult latency_sample_workload(const StrategyDescriptor& s,
                                           size_t size,
                                           int iters,
                                           int sample_every) {
    std::vector<double> samples;
    samples.reserve(static_cast<size_t>(iters / sample_every + 1));
    double start = now_ms();
    for (int i = 0; i < iters; ++i) {
        double op_start = 0;
        if (i % sample_every == 0) op_start = now_ms();
        void* p = s.vtable.allocate(size);
        touch_bytes(p, size, 0x9C);
        s.vtable.deallocate(p);
        if (i % sample_every == 0) samples.push_back(now_ms() - op_start);
    }
    double end = now_ms();
    std::sort(samples.begin(), samples.end());
    double p99_us = samples.empty() ? 0 : samples[static_cast<size_t>(samples.size() * 99 / 100)] * 1000.0;
    char name[96];
    std::snprintf(name, sizeof(name), "latency_sample_%zu_p99us_%.2f", size, p99_us);
    return {name, end - start, static_cast<size_t>(iters), peak_rss_kb()};
}

static BenchResult phase_changing_workload(const StrategyDescriptor& s,
                                           int iters,
                                           int slots,
                                           unsigned seed) {
    std::vector<void*> ptrs(static_cast<size_t>(slots), nullptr);
    std::mt19937 rng(seed);
    double start = now_ms();

    for (int i = 0; i < iters; ++i) {
        void* p = s.vtable.allocate(32 + static_cast<size_t>(i % 8) * 16);
        touch_bytes(p, 64, 0x11);
        s.vtable.deallocate(p);
    }
    std::uniform_int_distribution<size_t> medium_size(1024, 64 * 1024);
    for (int i = 0; i < iters / 2; ++i) {
        int idx = static_cast<int>(rng() % slots);
        if (ptrs[idx]) s.vtable.deallocate(ptrs[idx]);
        ptrs[idx] = s.vtable.allocate(medium_size(rng));
        touch_bytes(ptrs[idx], 128, 0x22);
    }
    for (int i = 0; i < std::max(1, iters / 32); ++i) {
        void* p = s.vtable.allocate(128 * 1024 + static_cast<size_t>(i % 8) * 64 * 1024);
        touch_bytes(p, 256, 0x33);
        s.vtable.deallocate(p);
    }
    for (int i = 0; i < slots; ++i) {
        if (!ptrs[static_cast<size_t>(i)]) {
            ptrs[static_cast<size_t>(i)] = s.vtable.allocate(256 + static_cast<size_t>(i % 64) * 16);
        }
    }
    for (void* p : ptrs) if (p) s.vtable.deallocate(p);

    double end = now_ms();
    return {"phase_changing", end - start, static_cast<size_t>(iters * 2 + slots), peak_rss_kb()};
}

static BenchResult large_streaming_workload(const StrategyDescriptor& s, int iters) {
    double start = now_ms();
    for (int i = 0; i < iters; ++i) {
        size_t size = 128 * 1024 + static_cast<size_t>(i % 16) * 64 * 1024;
        void* p = s.vtable.allocate(size);
        touch_bytes(p, size, 0x42);
        s.vtable.deallocate(p);
    }
    double end = now_ms();
    return {"large_streaming", end - start, static_cast<size_t>(iters), peak_rss_kb()};
}

static BenchResult debug_safety_workload(const StrategyDescriptor& s, int iters, bool exercise_invalid_free) {
    double start = now_ms();
    for (int i = 0; i < iters; ++i) {
        void* p = s.vtable.allocate(64 + static_cast<size_t>(i % 8) * 16);
        touch_bytes(p, 64, 0xD5);
        s.vtable.deallocate(p);
        if (exercise_invalid_free && i % 64 == 0) {
            s.vtable.deallocate(p);
        }
    }
    double end = now_ms();
    return {"debug_safety", end - start, static_cast<size_t>(iters), peak_rss_kb()};
}

enum class GeneratedPhaseKind {
    SmallChurn,
    FragmentationDrift,
    RemoteFree,
    LargeBurst,
    PeakRelease,
    LatencyLoop,
};

struct GeneratedPhase {
    const char* name;
    GeneratedPhaseKind kind;
    int weight;
};

struct GeneratedMetrics {
    uint64_t ops = 0;
    uint64_t allocs = 0;
    uint64_t frees = 0;
    uint64_t reallocs = 0;
    uint64_t remote_frees = 0;
    uint64_t requested_bytes = 0;
    uint64_t live_objects = 0;
    uint64_t live_bytes = 0;
    uint64_t peak_live_bytes = 0;
};

static void metrics_alloc(GeneratedMetrics& m, size_t size) {
    m.ops++;
    m.allocs++;
    m.requested_bytes += size;
    m.live_objects++;
    m.live_bytes += size;
    m.peak_live_bytes = std::max(m.peak_live_bytes, m.live_bytes);
}

static void metrics_free(GeneratedMetrics& m, size_t size, bool remote = false) {
    m.ops++;
    m.frees++;
    if (remote) m.remote_frees++;
    if (m.live_objects > 0) m.live_objects--;
    m.live_bytes = m.live_bytes > size ? m.live_bytes - size : 0;
}

static void metrics_realloc(GeneratedMetrics& m, size_t old_size, size_t new_size) {
    m.ops++;
    m.reallocs++;
    m.requested_bytes += new_size;
    m.live_bytes = m.live_bytes > old_size ? m.live_bytes - old_size : 0;
    m.live_bytes += new_size;
    m.peak_live_bytes = std::max(m.peak_live_bytes, m.live_bytes);
}

static std::vector<GeneratedPhase> generated_template(const std::string& name) {
    if (name == "throughput_churn") {
        return {{"small_churn", GeneratedPhaseKind::SmallChurn, 8},
                {"latency_loop", GeneratedPhaseKind::LatencyLoop, 2}};
    }
    if (name == "remote_queue") {
        return {{"small_churn", GeneratedPhaseKind::SmallChurn, 2},
                {"remote_free", GeneratedPhaseKind::RemoteFree, 8}};
    }
    if (name == "large_burst") {
        return {{"small_churn", GeneratedPhaseKind::SmallChurn, 2},
                {"large_burst", GeneratedPhaseKind::LargeBurst, 8}};
    }
    if (name == "rss_peak_release") {
        return {{"peak_release", GeneratedPhaseKind::PeakRelease, 8},
                {"small_churn", GeneratedPhaseKind::SmallChurn, 2}};
    }
    if (name == "fragmentation_drift") {
        return {{"fragmentation_drift", GeneratedPhaseKind::FragmentationDrift, 10}};
    }
    if (name == "latency_loop") {
        return {{"latency_loop", GeneratedPhaseKind::LatencyLoop, 10}};
    }
    return {{"small_churn", GeneratedPhaseKind::SmallChurn, 2},
            {"fragmentation_drift", GeneratedPhaseKind::FragmentationDrift, 3},
            {"remote_free", GeneratedPhaseKind::RemoteFree, 2},
            {"large_burst", GeneratedPhaseKind::LargeBurst, 2},
            {"peak_release", GeneratedPhaseKind::PeakRelease, 2},
            {"latency_loop", GeneratedPhaseKind::LatencyLoop, 1}};
}

static void publish_generated(const GeneratedPhase& phase,
                              int phase_index,
                              int phase_count,
                              const GeneratedMetrics& m,
                              double phase_elapsed_ms = 0.0,
                              double phase_duration_ms = 0.0,
                              bool running = true) {
    update_workload_snapshot(phase.name, phase_index, phase_count, m.ops, m.allocs, m.frees,
                             m.reallocs, m.remote_frees, m.requested_bytes, m.live_objects,
                             m.live_bytes, m.peak_live_bytes, phase_elapsed_ms,
                             phase_duration_ms, running);
}

static void generated_small_churn(const StrategyDescriptor& s,
                                  int ops,
                                  GeneratedMetrics& m,
                                  const GeneratedPhase& phase,
                                  int phase_index,
                                  int phase_count) {
    for (int i = 0; i < ops; ++i) {
        size_t size = 32 + static_cast<size_t>(i % 8) * 16;
        void* p = s.vtable.allocate(size);
        touch_bytes(p, size, 0x21);
        metrics_alloc(m, size);
        s.vtable.deallocate(p);
        metrics_free(m, size);
        if ((i & 1023) == 0) publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);
    }
}

static void generated_small_churn_chunk(const StrategyDescriptor& s,
                                        int ops,
                                        GeneratedMetrics& m) {
    for (int i = 0; i < ops; ++i) {
        size_t size = 32 + static_cast<size_t>((m.ops + static_cast<uint64_t>(i)) % 8) * 16;
        void* p = s.vtable.allocate(size);
        touch_bytes(p, size, 0x31);
        metrics_alloc(m, size);
        s.vtable.deallocate(p);
        metrics_free(m, size);
    }
}

static void generated_latency_loop(const StrategyDescriptor& s,
                                   int ops,
                                   GeneratedMetrics& m,
                                   const GeneratedPhase& phase,
                                   int phase_index,
                                   int phase_count) {
    for (int i = 0; i < ops; ++i) {
        void* p = s.vtable.allocate(64);
        touch_bytes(p, 64, 0x22);
        metrics_alloc(m, 64);
        s.vtable.deallocate(p);
        metrics_free(m, 64);
        if ((i & 2047) == 0) publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);
    }
}

static void generated_latency_loop_chunk(const StrategyDescriptor& s,
                                         int ops,
                                         GeneratedMetrics& m) {
    for (int i = 0; i < ops; ++i) {
        void* p = s.vtable.allocate(64);
        touch_bytes(p, 64, 0x32);
        metrics_alloc(m, 64);
        s.vtable.deallocate(p);
        metrics_free(m, 64);
    }
}

static void generated_fragmentation(const StrategyDescriptor& s,
                                    int ops,
                                    int slots,
                                    unsigned seed,
                                    GeneratedMetrics& m,
                                    const GeneratedPhase& phase,
                                    int phase_index,
                                    int phase_count) {
    slots = std::max(16, slots);
    std::vector<void*> ptrs(static_cast<size_t>(slots), nullptr);
    std::vector<size_t> sizes(static_cast<size_t>(slots), 0);
    std::mt19937 rng(seed + 17);
    std::uniform_int_distribution<size_t> size_dist(128, 96 * 1024);
    for (int i = 0; i < slots; ++i) {
        sizes[static_cast<size_t>(i)] = size_dist(rng);
        ptrs[static_cast<size_t>(i)] = s.vtable.allocate(sizes[static_cast<size_t>(i)]);
        touch_bytes(ptrs[static_cast<size_t>(i)], sizes[static_cast<size_t>(i)], 0x23);
        metrics_alloc(m, sizes[static_cast<size_t>(i)]);
    }
    for (int i = 0; i < ops; ++i) {
        size_t idx = static_cast<size_t>(rng() % static_cast<unsigned>(slots));
        size_t old_size = sizes[idx];
        size_t next_size = size_dist(rng);
        void* next = s.vtable.reallocate(ptrs[idx], next_size);
        if (next) {
            ptrs[idx] = next;
            sizes[idx] = next_size;
            touch_bytes(next, next_size, 0x24);
            metrics_realloc(m, old_size, next_size);
        }
        if ((i & 511) == 0) publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);
    }
    for (int i = 0; i < slots; ++i) {
        if (ptrs[static_cast<size_t>(i)]) {
            s.vtable.deallocate(ptrs[static_cast<size_t>(i)]);
            metrics_free(m, sizes[static_cast<size_t>(i)]);
        }
    }
}

static void generated_large_burst(const StrategyDescriptor& s,
                                  int ops,
                                  GeneratedMetrics& m,
                                  const GeneratedPhase& phase,
                                  int phase_index,
                                  int phase_count) {
    for (int i = 0; i < ops; ++i) {
        size_t size = 64 * 1024 + static_cast<size_t>(i % 16) * 64 * 1024;
        void* p = s.vtable.allocate(size);
        touch_bytes(p, size, 0x25);
        metrics_alloc(m, size);
        s.vtable.deallocate(p);
        metrics_free(m, size);
        if ((i & 127) == 0) publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);
    }
}

static void generated_large_burst_chunk(const StrategyDescriptor& s,
                                        int ops,
                                        GeneratedMetrics& m) {
    for (int i = 0; i < ops; ++i) {
        size_t size = 64 * 1024 + static_cast<size_t>((m.allocs + static_cast<uint64_t>(i)) % 16) * 64 * 1024;
        void* p = s.vtable.allocate(size);
        touch_bytes(p, size, 0x35);
        metrics_alloc(m, size);
        s.vtable.deallocate(p);
        metrics_free(m, size);
    }
}

static void generated_peak_release(const StrategyDescriptor& s,
                                   int ops,
                                   int slots,
                                   GeneratedMetrics& m,
                                   const GeneratedPhase& phase,
                                   int phase_index,
                                   int phase_count) {
    slots = std::max(16, std::min(slots, ops));
    std::vector<void*> ptrs(static_cast<size_t>(slots), nullptr);
    std::vector<size_t> sizes(static_cast<size_t>(slots), 0);
    for (int i = 0; i < slots; ++i) {
        sizes[static_cast<size_t>(i)] = 1024 + static_cast<size_t>(i % 64) * 1024;
        ptrs[static_cast<size_t>(i)] = s.vtable.allocate(sizes[static_cast<size_t>(i)]);
        touch_bytes(ptrs[static_cast<size_t>(i)], sizes[static_cast<size_t>(i)], 0x26);
        metrics_alloc(m, sizes[static_cast<size_t>(i)]);
        if ((i & 255) == 0) publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);
    }
    for (int i = 0; i < slots; ++i) {
        s.vtable.deallocate(ptrs[static_cast<size_t>(i)]);
        metrics_free(m, sizes[static_cast<size_t>(i)]);
        if ((i & 255) == 0) publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);
    }
}

static void generated_remote_free(const StrategyDescriptor& s,
                                  int ops,
                                  int threads,
                                  unsigned seed,
                                  GeneratedMetrics& m,
                                  const GeneratedPhase& phase,
                                  int phase_index,
                                  int phase_count) {
    threads = std::max(2, threads);
    int per_thread = std::max(1, ops / threads);
    std::vector<std::vector<void*>> ptrs(static_cast<size_t>(threads));
    std::vector<std::vector<size_t>> sizes(static_cast<size_t>(threads));
    for (int t = 0; t < threads; ++t) {
        ptrs[static_cast<size_t>(t)].resize(static_cast<size_t>(per_thread), nullptr);
        sizes[static_cast<size_t>(t)].resize(static_cast<size_t>(per_thread), 0);
    }
    std::vector<GeneratedMetrics> local(static_cast<size_t>(threads));
    std::vector<std::thread> producers;
    for (int t = 0; t < threads; ++t) {
        producers.emplace_back([&, t] {
            std::mt19937 rng(seed + static_cast<unsigned>(t) * 131);
            std::uniform_int_distribution<size_t> size_dist(64, 4096);
            for (int i = 0; i < per_thread; ++i) {
                size_t size = size_dist(rng);
                sizes[static_cast<size_t>(t)][static_cast<size_t>(i)] = size;
                ptrs[static_cast<size_t>(t)][static_cast<size_t>(i)] = s.vtable.allocate(size);
                touch_bytes(ptrs[static_cast<size_t>(t)][static_cast<size_t>(i)], size, 0x27);
                metrics_alloc(local[static_cast<size_t>(t)], size);
            }
        });
    }
    for (auto& th : producers) th.join();
    for (const auto& lm : local) {
        m.ops += lm.ops;
        m.allocs += lm.allocs;
        m.requested_bytes += lm.requested_bytes;
        m.live_objects += lm.live_objects;
        m.live_bytes += lm.live_bytes;
        m.peak_live_bytes = std::max(m.peak_live_bytes, m.live_bytes);
    }
    publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);

    std::vector<std::thread> consumers;
    for (int t = 0; t < threads; ++t) {
        consumers.emplace_back([&, t] {
            int victim = (t + threads - 1) % threads;
            for (int i = 0; i < per_thread; ++i) {
                s.vtable.deallocate(ptrs[static_cast<size_t>(victim)][static_cast<size_t>(i)]);
            }
        });
    }
    for (auto& th : consumers) th.join();
    for (int t = 0; t < threads; ++t) {
        for (int i = 0; i < per_thread; ++i) {
            metrics_free(m, sizes[static_cast<size_t>(t)][static_cast<size_t>(i)], true);
        }
    }
    publish_generated(phase, phase_index, phase_count, m, 0.0, 0.0, true);
}

static std::string generated_extra_json(const std::string& templ, const GeneratedMetrics& m) {
    char buf[512];
    std::snprintf(buf, sizeof(buf),
                  "\"generated\":{\"template\":\"%s\",\"allocs\":%llu,\"frees\":%llu,"
                  "\"reallocs\":%llu,\"remote_frees\":%llu,\"requested_bytes\":%llu,"
                  "\"peak_live_bytes\":%llu}",
                  templ.c_str(),
                  static_cast<unsigned long long>(m.allocs),
                  static_cast<unsigned long long>(m.frees),
                  static_cast<unsigned long long>(m.reallocs),
                  static_cast<unsigned long long>(m.remote_frees),
                  static_cast<unsigned long long>(m.requested_bytes),
                  static_cast<unsigned long long>(m.peak_live_bytes));
    return buf;
}

struct RealtimePhaseState {
    std::vector<void*> ptrs;
    std::vector<size_t> sizes;
    std::mt19937 rng;
    bool initialized = false;

    explicit RealtimePhaseState(unsigned seed) : rng(seed) {}
};

static void generated_fragmentation_realtime_chunk(const StrategyDescriptor& s,
                                                   int ops,
                                                   int slots,
                                                   GeneratedMetrics& m,
                                                   RealtimePhaseState& state) {
    slots = std::max(16, slots);
    if (!state.initialized) {
        state.ptrs.assign(static_cast<size_t>(slots), nullptr);
        state.sizes.assign(static_cast<size_t>(slots), 0);
        std::uniform_int_distribution<size_t> size_dist(128, 96 * 1024);
        for (int i = 0; i < slots; ++i) {
            size_t size = size_dist(state.rng);
            state.sizes[static_cast<size_t>(i)] = size;
            state.ptrs[static_cast<size_t>(i)] = s.vtable.allocate(size);
            touch_bytes(state.ptrs[static_cast<size_t>(i)], size, 0x41);
            metrics_alloc(m, size);
        }
        state.initialized = true;
    }
    std::uniform_int_distribution<size_t> size_dist(128, 96 * 1024);
    for (int i = 0; i < ops; ++i) {
        size_t idx = static_cast<size_t>(state.rng() % state.ptrs.size());
        size_t old_size = state.sizes[idx];
        size_t next_size = size_dist(state.rng);
        void* next = s.vtable.reallocate(state.ptrs[idx], next_size);
        if (next) {
            state.ptrs[idx] = next;
            state.sizes[idx] = next_size;
            touch_bytes(next, next_size, 0x42);
            metrics_realloc(m, old_size, next_size);
        }
    }
}

static void generated_peak_release_realtime_chunk(const StrategyDescriptor& s,
                                                  int ops,
                                                  int slots,
                                                  GeneratedMetrics& m,
                                                  RealtimePhaseState& state) {
    slots = std::max(16, slots);
    if (!state.initialized) {
        state.ptrs.assign(static_cast<size_t>(slots), nullptr);
        state.sizes.assign(static_cast<size_t>(slots), 0);
        state.initialized = true;
    }
    for (int i = 0; i < ops; ++i) {
        size_t idx = static_cast<size_t>((m.ops + static_cast<uint64_t>(i)) % state.ptrs.size());
        if (!state.ptrs[idx]) {
            size_t size = 1024 + (idx % 64) * 1024;
            state.sizes[idx] = size;
            state.ptrs[idx] = s.vtable.allocate(size);
            touch_bytes(state.ptrs[idx], size, 0x43);
            metrics_alloc(m, size);
        } else {
            s.vtable.deallocate(state.ptrs[idx]);
            metrics_free(m, state.sizes[idx]);
            state.ptrs[idx] = nullptr;
            state.sizes[idx] = 0;
        }
    }
}

static void generated_remote_realtime_chunk(const StrategyDescriptor& s,
                                            int ops,
                                            int threads,
                                            GeneratedMetrics& m,
                                            RealtimePhaseState& state) {
    threads = std::max(2, threads);
    int count = std::max(1, ops / 2);
    std::vector<void*> ptrs(static_cast<size_t>(count), nullptr);
    std::vector<size_t> sizes(static_cast<size_t>(count), 0);
    std::uniform_int_distribution<size_t> size_dist(64, 4096);
    std::thread producer([&] {
        for (int i = 0; i < count; ++i) {
            sizes[static_cast<size_t>(i)] = size_dist(state.rng);
            ptrs[static_cast<size_t>(i)] = s.vtable.allocate(sizes[static_cast<size_t>(i)]);
            touch_bytes(ptrs[static_cast<size_t>(i)], sizes[static_cast<size_t>(i)], 0x44);
        }
    });
    producer.join();
    for (int i = 0; i < count; ++i) metrics_alloc(m, sizes[static_cast<size_t>(i)]);
    std::thread consumer([&] {
        for (void* p : ptrs) s.vtable.deallocate(p);
    });
    consumer.join();
    for (int i = 0; i < count; ++i) metrics_free(m, sizes[static_cast<size_t>(i)], true);
}

static void cleanup_realtime_state(const StrategyDescriptor& s,
                                   GeneratedMetrics& m,
                                   RealtimePhaseState& state) {
    for (size_t i = 0; i < state.ptrs.size(); ++i) {
        if (state.ptrs[i]) {
            s.vtable.deallocate(state.ptrs[i]);
            metrics_free(m, state.sizes[i]);
            state.ptrs[i] = nullptr;
            state.sizes[i] = 0;
        }
    }
}

static void run_realtime_phase_chunk(const StrategyDescriptor& s,
                                     const BenchConfig& cfg,
                                     const GeneratedPhase& phase,
                                     int ops,
                                     GeneratedMetrics& m,
                                     RealtimePhaseState& state) {
    switch (phase.kind) {
        case GeneratedPhaseKind::SmallChurn:
            generated_small_churn_chunk(s, ops, m);
            break;
        case GeneratedPhaseKind::FragmentationDrift:
            generated_fragmentation_realtime_chunk(s, ops, cfg.slots, m, state);
            break;
        case GeneratedPhaseKind::RemoteFree:
            generated_remote_realtime_chunk(s, ops, cfg.threads, m, state);
            break;
        case GeneratedPhaseKind::LargeBurst:
            generated_large_burst_chunk(s, std::max(1, ops / 8), m);
            break;
        case GeneratedPhaseKind::PeakRelease:
            generated_peak_release_realtime_chunk(s, ops, cfg.slots, m, state);
            break;
        case GeneratedPhaseKind::LatencyLoop:
            generated_latency_loop_chunk(s, ops, m);
            break;
    }
}

static void run_generated_realtime(const StrategyDescriptor& s,
                                   const BenchConfig& cfg,
                                   const std::vector<GeneratedPhase>& phases,
                                   GeneratedMetrics& m) {
    int phase_count = static_cast<int>(phases.size()) * std::max(1, cfg.phase_repeat);
    int phase_index = 0;
    int tick_ms = std::max(10, std::min(100, cfg.phase_ms));
    int ops_per_tick = std::max(1, cfg.target_ops_per_sec * tick_ms / 1000);
    for (int repeat = 0; repeat < std::max(1, cfg.phase_repeat); ++repeat) {
        for (size_t i = 0; i < phases.size(); ++i, ++phase_index) {
            const GeneratedPhase& phase = phases[i];
            RealtimePhaseState state(cfg.seed + static_cast<unsigned>(phase_index) * 977u);
            double phase_start = now_ms();
            while (true) {
                double elapsed = now_ms() - phase_start;
                if (elapsed >= cfg.phase_ms) break;
                double tick_start = now_ms();
                run_realtime_phase_chunk(s, cfg, phase, ops_per_tick, m, state);
                double after_ops = now_ms();
                double phase_elapsed = after_ops - phase_start;
                publish_generated(phase, phase_index, phase_count, m,
                                  phase_elapsed, static_cast<double>(cfg.phase_ms), true);
                double spent = now_ms() - tick_start;
                double remaining = static_cast<double>(cfg.phase_ms) - (now_ms() - phase_start);
                double sleep_ms = std::min(static_cast<double>(tick_ms) - spent, remaining);
                if (sleep_ms > 0.0) {
                    usleep(static_cast<useconds_t>(sleep_ms * 1000.0));
                }
            }
            cleanup_realtime_state(s, m, state);
            publish_generated(phase, phase_index, phase_count, m,
                              static_cast<double>(cfg.phase_ms), static_cast<double>(cfg.phase_ms), true);
        }
    }
}

static BenchResult generated_workload(const StrategyDescriptor& s,
                                      const BenchConfig& cfg) {
    std::vector<GeneratedPhase> phases = generated_template(cfg.workload_template);
    int total_weight = 0;
    for (const auto& p : phases) total_weight += p.weight;
    total_weight = std::max(1, total_weight);
    GeneratedMetrics m;
    set_workload_identity(s.name, "generated_workload", cfg.workload_template.c_str());
    g_workload_start_ms = now_ms();
    int total_phase_count = static_cast<int>(phases.size()) * std::max(1, cfg.phase_repeat);
    update_workload_snapshot("starting", 0, total_phase_count, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0.0, cfg.workload_realtime ? static_cast<double>(cfg.phase_ms) : 0.0, true);
    double start = now_ms();
    if (cfg.workload_realtime) {
        run_generated_realtime(s, cfg, phases, m);
    } else {
        for (size_t i = 0; i < phases.size(); ++i) {
            const GeneratedPhase& phase = phases[i];
            int ops = std::max(1, cfg.iters * phase.weight / total_weight);
            publish_generated(phase, static_cast<int>(i), static_cast<int>(phases.size()), m, 0.0, 0.0, true);
            switch (phase.kind) {
                case GeneratedPhaseKind::SmallChurn:
                    generated_small_churn(s, ops, m, phase, static_cast<int>(i), static_cast<int>(phases.size()));
                    break;
                case GeneratedPhaseKind::FragmentationDrift:
                    generated_fragmentation(s, ops, cfg.slots, cfg.seed, m, phase,
                                            static_cast<int>(i), static_cast<int>(phases.size()));
                    break;
                case GeneratedPhaseKind::RemoteFree:
                    generated_remote_free(s, ops, cfg.threads, cfg.seed, m, phase,
                                          static_cast<int>(i), static_cast<int>(phases.size()));
                    break;
                case GeneratedPhaseKind::LargeBurst:
                    generated_large_burst(s, std::max(1, ops / 8), m, phase,
                                          static_cast<int>(i), static_cast<int>(phases.size()));
                    break;
                case GeneratedPhaseKind::PeakRelease:
                    generated_peak_release(s, ops, cfg.slots, m, phase,
                                           static_cast<int>(i), static_cast<int>(phases.size()));
                    break;
                case GeneratedPhaseKind::LatencyLoop:
                    generated_latency_loop(s, ops, m, phase, static_cast<int>(i), static_cast<int>(phases.size()));
                    break;
            }
        }
    }
    double end = now_ms();
    update_workload_snapshot("finished", total_phase_count, total_phase_count,
                             m.ops, m.allocs, m.frees, m.reallocs, m.remote_frees,
                             m.requested_bytes, m.live_objects, m.live_bytes, m.peak_live_bytes,
                             0.0, 0.0, false);
    return {"generated_workload", end - start, static_cast<size_t>(m.ops), peak_rss_kb(),
            generated_extra_json(cfg.workload_template, m)};
}

static RepeatSummary summarize(const std::vector<double>& values) {
    std::vector<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    double sum = 0.0;
    for (double v : sorted) sum += v;
    double mean = sorted.empty() ? 0.0 : sum / static_cast<double>(sorted.size());
    double var = 0.0;
    for (double v : sorted) {
        double d = v - mean;
        var += d * d;
    }
    if (!sorted.empty()) var /= static_cast<double>(sorted.size());
    size_t p95_index = sorted.empty() ? 0 : std::min(sorted.size() - 1, sorted.size() * 95 / 100);
    return RepeatSummary{
        mean,
        sorted.empty() ? 0.0 : sorted[sorted.size() / 2],
        sorted.empty() ? 0.0 : sorted[p95_index],
        std::sqrt(var),
        sorted.empty() ? 0.0 : sorted.front(),
        sorted.empty() ? 0.0 : sorted.back(),
    };
}

static bool strategy_is_adaptive(const char* strategy) {
    return std::strcmp(strategy, "adaptive") == 0;
}

static void print_json(const char* strategy, const BenchResult& r) {
    double ops = r.ops / (r.ms / 1000.0);
    std::printf("{\"strategy\":\"%s\",\"benchmark\":\"%s\",\"ops_per_sec\":%.0f,\"ms\":%.3f,\"peak_rss_kb\":%zu",
                strategy, r.name.c_str(), ops, r.ms, r.peak_rss_kb);
    if (strategy_is_adaptive(strategy)) {
        my_ptmalloc::AdaptiveStatsSnapshot s = my_ptmalloc::adaptive_stats_snapshot();
        std::printf(",\"adaptive\":{\"current_mode\":\"%s\",\"active_mode\":\"%s\","
                    "\"previous_mode\":\"%s\",\"mode_switches\":%llu,"
                    "\"retired_mode_count\":%llu,\"mapped_bytes\":%lld,"
                    "\"live_bytes\":%lld,\"mapped_live_ratio\":%.3f,"
                    "\"remote_free_ratio\":%.3f,\"size_entropy\":%.3f,"
                    "\"large_bytes_ratio\":%.3f,\"fragmentation_estimate\":%.3f,"
                    "\"slow_path_ratio\":%.3f,\"double_free_count\":%llu,"
                    "\"invalid_free_count\":%llu,"
                    "\"empty_pages\":%llu,\"empty_spans\":%llu,\"released_pages\":%llu,"
                    "\"released_spans\":%llu,\"release_unmapped_bytes\":%llu,"
                    "\"mode_alloc_count\":[%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu],"
                    "\"mode_free_count\":[%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu],"
                    "\"mode_live_bytes\":[%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld],"
                    "\"mode_mapped_bytes\":[%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld],"
                    "\"storage_allocs\":[%llu,%llu,%llu],\"storage_frees\":[%llu,%llu,%llu],"
                    "\"pool_hits\":[%llu,%llu,%llu],\"pool_misses\":[%llu,%llu,%llu]}",
                    my_ptmalloc::adaptive_mode_name(s.current_mode),
                    my_ptmalloc::adaptive_mode_name(s.active_mode),
                    my_ptmalloc::adaptive_mode_name(s.previous_mode),
                    static_cast<unsigned long long>(s.mode_switches),
                    static_cast<unsigned long long>(s.retired_mode_count),
                    static_cast<long long>(s.mapped_bytes),
                    static_cast<long long>(s.live_bytes),
                    s.mapped_live_ratio,
                    s.remote_free_ratio,
                    s.size_entropy,
                    s.large_bytes_ratio,
                    s.fragmentation_estimate,
                    s.slow_path_ratio,
                    static_cast<unsigned long long>(s.double_free_count),
                    static_cast<unsigned long long>(s.invalid_free_count),
                    static_cast<unsigned long long>(s.empty_pages),
                    static_cast<unsigned long long>(s.empty_spans),
                    static_cast<unsigned long long>(s.released_pages),
                    static_cast<unsigned long long>(s.released_spans),
                    static_cast<unsigned long long>(s.release_unmapped_bytes),
                    static_cast<unsigned long long>(s.mode_alloc_count[0]),
                    static_cast<unsigned long long>(s.mode_alloc_count[1]),
                    static_cast<unsigned long long>(s.mode_alloc_count[2]),
                    static_cast<unsigned long long>(s.mode_alloc_count[3]),
                    static_cast<unsigned long long>(s.mode_alloc_count[4]),
                    static_cast<unsigned long long>(s.mode_alloc_count[5]),
                    static_cast<unsigned long long>(s.mode_alloc_count[6]),
                    static_cast<unsigned long long>(s.mode_alloc_count[7]),
                    static_cast<unsigned long long>(s.mode_free_count[0]),
                    static_cast<unsigned long long>(s.mode_free_count[1]),
                    static_cast<unsigned long long>(s.mode_free_count[2]),
                    static_cast<unsigned long long>(s.mode_free_count[3]),
                    static_cast<unsigned long long>(s.mode_free_count[4]),
                    static_cast<unsigned long long>(s.mode_free_count[5]),
                    static_cast<unsigned long long>(s.mode_free_count[6]),
                    static_cast<unsigned long long>(s.mode_free_count[7]),
                    static_cast<long long>(s.mode_live_bytes[0]),
                    static_cast<long long>(s.mode_live_bytes[1]),
                    static_cast<long long>(s.mode_live_bytes[2]),
                    static_cast<long long>(s.mode_live_bytes[3]),
                    static_cast<long long>(s.mode_live_bytes[4]),
                    static_cast<long long>(s.mode_live_bytes[5]),
                    static_cast<long long>(s.mode_live_bytes[6]),
                    static_cast<long long>(s.mode_live_bytes[7]),
                    static_cast<long long>(s.mode_mapped_bytes[0]),
                    static_cast<long long>(s.mode_mapped_bytes[1]),
                    static_cast<long long>(s.mode_mapped_bytes[2]),
                    static_cast<long long>(s.mode_mapped_bytes[3]),
                    static_cast<long long>(s.mode_mapped_bytes[4]),
                    static_cast<long long>(s.mode_mapped_bytes[5]),
                    static_cast<long long>(s.mode_mapped_bytes[6]),
                    static_cast<long long>(s.mode_mapped_bytes[7]),
                    static_cast<unsigned long long>(s.storage[0].alloc_count),
                    static_cast<unsigned long long>(s.storage[1].alloc_count),
                    static_cast<unsigned long long>(s.storage[2].alloc_count),
                    static_cast<unsigned long long>(s.storage[0].free_count),
                    static_cast<unsigned long long>(s.storage[1].free_count),
                    static_cast<unsigned long long>(s.storage[2].free_count),
                    static_cast<unsigned long long>(s.storage[0].pool_hits),
                    static_cast<unsigned long long>(s.storage[1].pool_hits),
                    static_cast<unsigned long long>(s.storage[2].pool_hits),
                    static_cast<unsigned long long>(s.storage[0].pool_misses),
                    static_cast<unsigned long long>(s.storage[1].pool_misses),
                    static_cast<unsigned long long>(s.storage[2].pool_misses));
    }
    if (!r.extra_json.empty()) {
        std::printf(",%s", r.extra_json.c_str());
    }
    std::printf("}\n");
}

static void print_usage(const char* argv0) {
    std::printf(
        "usage: %s [options]\n"
        "\n"
        "options:\n"
        "  --strategy NAME       hybrid, ptmalloc, tcmalloc_like, jemalloc_like,\n"
        "                        mimalloc_like, adaptive, libc, or plugin:path.so\n"
        "  --profile NAME        smoke, micro, stress, all (default: micro)\n"
        "  --bench NAME          add one benchmark; may be repeated\n"
        "                        same_size, same_size_64, same_size_256, batch, random,\n"
        "                        fragmentation, cross_thread_free, latency_sample,\n"
        "                        phase_changing, throughput_server, realtime_latency,\n"
        "                        memory_constrained, producer_consumer, large_streaming,\n"
        "                        debug_safety, generated_workload\n"
        "  --json                print one JSON object per result\n"
        "  --repeats N           repeat each benchmark and report aggregate stats\n"
        "  --iters N             iterations for same-size/fragmentation/latency tests\n"
        "  --random-iters N      iterations for random workload\n"
        "  --size N              size for same_size and latency_sample\n"
        "  --min-size N          random/cross-thread minimum allocation size\n"
        "  --max-size N          random/cross-thread maximum allocation size\n"
        "  --slots N             live pointer slots for random/fragmentation\n"
        "  --batch N             batch size for batch/cross-thread tests\n"
        "  --rounds N            batch rounds\n"
        "  --threads N           threads for cross_thread_free\n"
        "  --workload-template N adaptive_mix, throughput_churn, remote_queue,\n"
        "                        large_burst, rss_peak_release, fragmentation_drift,\n"
        "                        latency_loop\n"
        "  --workload-realtime   run generated_workload by wall-clock phase time\n"
        "  --phase-ms N          realtime generated_workload phase duration (default: 10000)\n"
        "  --target-ops-per-sec N realtime generated_workload throttle target (default: 50000)\n"
        "  --phase-repeat N      realtime generated_workload template repetitions\n"
        "  --telemetry-port N    serve lightweight local workload UI on 127.0.0.1:N\n"
        "  --telemetry-hold-ms N keep telemetry UI alive after benchmarks finish\n"
        "                        when --telemetry-port is enabled (default: 300000; 0 exits immediately)\n"
        "  --seed N              deterministic RNG seed\n"
        "  --help                show this help\n",
        argv0);
}

static bool parse_int_arg(const char* value, int& out) {
    char* end = nullptr;
    long v = std::strtol(value, &end, 10);
    if (!end || *end != '\0' || v <= 0) return false;
    out = static_cast<int>(v);
    return true;
}

static bool parse_nonnegative_int_arg(const char* value, int& out) {
    char* end = nullptr;
    long v = std::strtol(value, &end, 10);
    if (!end || *end != '\0' || v < 0) return false;
    out = static_cast<int>(v);
    return true;
}

static bool parse_size_arg(const char* value, size_t& out) {
    char* end = nullptr;
    unsigned long long v = std::strtoull(value, &end, 10);
    if (!end || *end != '\0' || v == 0) return false;
    out = static_cast<size_t>(v);
    return true;
}

static bool parse_args(int argc, char** argv, BenchConfig& cfg) {
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        auto need_value = [&](const char* name) -> const char* {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "%s needs a value\n", name);
                return nullptr;
            }
            return argv[++i];
        };

        if (std::strcmp(arg, "--strategy") == 0) {
            const char* v = need_value(arg);
            if (!v) return false;
            cfg.strategy = v;
        } else if (std::strcmp(arg, "--profile") == 0) {
            const char* v = need_value(arg);
            if (!v) return false;
            cfg.profile = v;
        } else if (std::strcmp(arg, "--bench") == 0) {
            const char* v = need_value(arg);
            if (!v) return false;
            cfg.benches.emplace_back(v);
        } else if (std::strcmp(arg, "--json") == 0) {
            cfg.json = true;
        } else if (std::strcmp(arg, "--iters") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.iters)) return false;
        } else if (std::strcmp(arg, "--random-iters") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.random_iters)) return false;
        } else if (std::strcmp(arg, "--size") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_size_arg(v, cfg.size)) return false;
        } else if (std::strcmp(arg, "--min-size") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_size_arg(v, cfg.min_size)) return false;
        } else if (std::strcmp(arg, "--max-size") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_size_arg(v, cfg.max_size)) return false;
        } else if (std::strcmp(arg, "--slots") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.slots)) return false;
        } else if (std::strcmp(arg, "--batch") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.batch)) return false;
        } else if (std::strcmp(arg, "--rounds") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.rounds)) return false;
        } else if (std::strcmp(arg, "--threads") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.threads)) return false;
        } else if (std::strcmp(arg, "--workload-template") == 0 ||
                   std::strcmp(arg, "--template") == 0) {
            const char* v = need_value(arg);
            if (!v) return false;
            cfg.workload_template = v;
        } else if (std::strcmp(arg, "--workload-realtime") == 0) {
            cfg.workload_realtime = true;
        } else if (std::strcmp(arg, "--phase-ms") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.phase_ms)) return false;
        } else if (std::strcmp(arg, "--target-ops-per-sec") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.target_ops_per_sec)) return false;
        } else if (std::strcmp(arg, "--phase-repeat") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.phase_repeat)) return false;
        } else if (std::strcmp(arg, "--telemetry-port") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.telemetry_port)) return false;
        } else if (std::strcmp(arg, "--telemetry-hold-ms") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_nonnegative_int_arg(v, cfg.telemetry_hold_ms)) return false;
        } else if (std::strcmp(arg, "--repeats") == 0) {
            const char* v = need_value(arg);
            if (!v || !parse_int_arg(v, cfg.repeats)) return false;
        } else if (std::strcmp(arg, "--seed") == 0) {
            const char* v = need_value(arg);
            size_t parsed = 0;
            if (!v || !parse_size_arg(v, parsed)) return false;
            cfg.seed = static_cast<unsigned>(parsed);
        } else if (std::strcmp(arg, "--help") == 0) {
            print_usage(argv[0]);
            std::exit(0);
        } else {
            std::fprintf(stderr, "unknown option: %s\n", arg);
            return false;
        }
    }
    if (cfg.min_size > cfg.max_size) {
        std::fprintf(stderr, "--min-size must be <= --max-size\n");
        return false;
    }
    return true;
}

static std::vector<std::string> profile_benches(const std::string& profile) {
    if (profile == "smoke") return {"same_size_64", "random"};
    if (profile == "micro") return {"same_size_64", "same_size_256", "batch", "random"};
    if (profile == "stress") return {"random", "fragmentation", "cross_thread_free"};
    if (profile == "all") {
        return {"same_size_64", "same_size_256", "same_size", "batch", "random",
                "fragmentation", "cross_thread_free", "latency_sample", "phase_changing"};
    }
    std::fprintf(stderr, "unknown profile '%s'\n", profile.c_str());
    return {};
}

static bool run_one(const StrategyDescriptor& s,
                    const BenchConfig& cfg,
                    const std::string& name,
                    BenchResult& out) {
    if (name == "same_size") {
        out = same_size(s, cfg.size, cfg.iters);
        return true;
    }
    if (name == "same_size_64") {
        out = same_size(s, 64, cfg.iters);
        return true;
    }
    if (name == "same_size_256") {
        out = same_size(s, 256, cfg.iters);
        return true;
    }
    if (name == "batch") {
        out = batch_workload(s, cfg.batch, cfg.rounds);
        return true;
    }
    if (name == "random") {
        out = random_workload(s, cfg.random_iters, cfg.slots, cfg.min_size, cfg.max_size, cfg.seed);
        return true;
    }
    if (name == "fragmentation") {
        out = fragmentation_workload(s, cfg.iters, cfg.slots, cfg.min_size, cfg.max_size, cfg.seed);
        return true;
    }
    if (name == "cross_thread_free") {
        out = cross_thread_free_workload(s, cfg.threads, cfg.batch, cfg.min_size, cfg.max_size, cfg.seed);
        return true;
    }
    if (name == "latency_sample") {
        out = latency_sample_workload(s, cfg.size, cfg.iters, 100);
        return true;
    }
    if (name == "phase_changing") {
        out = phase_changing_workload(s, cfg.iters, cfg.slots, cfg.seed);
        return true;
    }
    if (name == "throughput_server") {
        out = batch_workload(s, cfg.batch * 2, cfg.rounds);
        out.name = "throughput_server";
        return true;
    }
    if (name == "realtime_latency") {
        out = latency_sample_workload(s, cfg.size, cfg.iters, 50);
        out.name = "realtime_latency";
        return true;
    }
    if (name == "memory_constrained") {
        out = phase_changing_workload(s, cfg.iters, cfg.slots, cfg.seed);
        out.name = "memory_constrained";
        return true;
    }
    if (name == "producer_consumer") {
        out = cross_thread_free_workload(s, cfg.threads, cfg.batch, cfg.min_size, cfg.max_size, cfg.seed);
        out.name = "producer_consumer";
        return true;
    }
    if (name == "large_streaming") {
        out = large_streaming_workload(s, std::max(1, cfg.iters / 8));
        return true;
    }
    if (name == "debug_safety") {
        out = debug_safety_workload(s, std::max(1, cfg.iters / 16), strategy_is_adaptive(s.name));
        return true;
    }
    if (name == "generated_workload") {
        out = generated_workload(s, cfg);
        return true;
    }
    std::fprintf(stderr, "unknown benchmark '%s'\n", name.c_str());
    return false;
}

int main(int argc, char** argv) {
    BenchConfig cfg;
    if (!parse_args(argc, argv, cfg)) {
        print_usage(argv[0]);
        return 2;
    }

    LoadedStrategy loaded;
    if (!load_strategy(cfg.strategy.c_str(), loaded)) return 2;
    if (loaded.desc.vtable.init) loaded.desc.vtable.init();

    TelemetryServer telemetry_server;
    if (cfg.telemetry_port > 0) {
        set_workload_identity(loaded.desc.name, "idle", cfg.workload_template.c_str());
        if (!telemetry_server.start(cfg.telemetry_port)) {
            std::fprintf(stderr, "failed to start telemetry server on 127.0.0.1:%d\n", cfg.telemetry_port);
            unload_strategy(loaded);
            return 2;
        }
        if (!cfg.json) {
            std::printf("telemetry UI: http://127.0.0.1:%d/\n", cfg.telemetry_port);
        }
    }

    std::vector<std::string> benches = cfg.benches.empty() ? profile_benches(cfg.profile) : cfg.benches;
    if (benches.empty()) {
        unload_strategy(loaded);
        return 2;
    }

    if (!cfg.json) {
        std::printf("=== strategy: %s ===\n", loaded.desc.name);
        std::printf("profile=%s iters=%d random_iters=%d size=%zu range=%zu..%zu slots=%d batch=%d rounds=%d threads=%d repeats=%d seed=%u\n",
                    cfg.profile.c_str(), cfg.iters, cfg.random_iters, cfg.size, cfg.min_size,
                    cfg.max_size, cfg.slots, cfg.batch, cfg.rounds, cfg.threads, cfg.repeats, cfg.seed);
    }

    for (const auto& bench : benches) {
        std::vector<double> ops_values;
        BenchResult last;
        for (int rep = 0; rep < cfg.repeats; ++rep) {
            BenchResult r;
            if (!run_one(loaded.desc, cfg, bench, r)) {
                unload_strategy(loaded);
                return 2;
            }
            last = r;
            ops_values.push_back(r.ops / (r.ms / 1000.0));
            if (cfg.json && cfg.repeats == 1) {
                print_json(loaded.desc.name, r);
            }
        }
        if (cfg.repeats > 1) {
            RepeatSummary summary = summarize(ops_values);
            if (cfg.json) {
                std::printf("{\"strategy\":\"%s\",\"benchmark\":\"%s\",\"repeats\":%d,"
                            "\"ops_per_sec_mean\":%.0f,\"ops_per_sec_median\":%.0f,"
                            "\"ops_per_sec_p95\":%.0f,\"ops_per_sec_stddev\":%.0f,"
                            "\"ops_per_sec_min\":%.0f,\"ops_per_sec_max\":%.0f,"
                            "\"last_ms\":%.3f,\"peak_rss_kb\":%zu",
                            loaded.desc.name, last.name.c_str(), cfg.repeats,
                            summary.mean, summary.median, summary.p95, summary.stddev,
                            summary.min, summary.max, last.ms, last.peak_rss_kb);
                if (strategy_is_adaptive(loaded.desc.name)) {
                    auto s = my_ptmalloc::adaptive_stats_snapshot();
                    std::printf(",\"adaptive\":{\"current_mode\":\"%s\",\"active_mode\":\"%s\","
                                "\"previous_mode\":\"%s\",\"mode_switches\":%llu,"
                                "\"retired_mode_count\":%llu,\"mapped_bytes\":%lld,"
                                "\"live_bytes\":%lld,\"mapped_live_ratio\":%.3f,"
                                "\"remote_free_ratio\":%.3f,\"size_entropy\":%.3f,"
                                "\"large_bytes_ratio\":%.3f,\"fragmentation_estimate\":%.3f,"
                                "\"slow_path_ratio\":%.3f,\"double_free_count\":%llu,"
                                "\"invalid_free_count\":%llu}",
                                my_ptmalloc::adaptive_mode_name(s.current_mode),
                                my_ptmalloc::adaptive_mode_name(s.active_mode),
                                my_ptmalloc::adaptive_mode_name(s.previous_mode),
                                static_cast<unsigned long long>(s.mode_switches),
                                static_cast<unsigned long long>(s.retired_mode_count),
                                static_cast<long long>(s.mapped_bytes),
                                static_cast<long long>(s.live_bytes),
                                s.mapped_live_ratio,
                                s.remote_free_ratio,
                                s.size_entropy,
                                s.large_bytes_ratio,
                                s.fragmentation_estimate,
                                s.slow_path_ratio,
                                static_cast<unsigned long long>(s.double_free_count),
                                static_cast<unsigned long long>(s.invalid_free_count));
                }
                std::printf("}\n");
            } else {
                std::printf("  %-16s mean=%10.0f median=%10.0f p95=%10.0f stddev=%8.0f min=%10.0f max=%10.0f peak=%zuKB\n",
                            last.name.c_str(), summary.mean, summary.median, summary.p95,
                            summary.stddev, summary.min, summary.max, last.peak_rss_kb);
            }
        } else if (!cfg.json) {
            double ops = ops_values.empty() ? 0.0 : ops_values.front();
            std::printf("  %-16s %10.0f ops/sec  %7.2f ms  peak=%zuKB\n",
                        last.name.c_str(), ops, last.ms, last.peak_rss_kb);
        }
    }

    if (cfg.telemetry_port > 0 && cfg.telemetry_hold_ms > 0 && !cfg.json) {
        std::printf("telemetry UI remains available for %.1f seconds; press Ctrl+C to stop earlier\n",
                    static_cast<double>(cfg.telemetry_hold_ms) / 1000.0);
        std::fflush(stdout);
        int remaining = cfg.telemetry_hold_ms;
        while (remaining > 0) {
            int chunk = std::min(remaining, 1000);
            usleep(static_cast<useconds_t>(chunk) * 1000);
            remaining -= chunk;
        }
    }

    unload_strategy(loaded);
    telemetry_server.stop();
    return 0;
}
