#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
EXTERNAL_DIR="${EXTERNAL_DIR:-$ROOT_DIR/external}"
RESULTS_DIR="${RESULTS_DIR:-$ROOT_DIR/results/external}"
MY_MALLOC_SO="${MY_MALLOC_SO:-$BUILD_DIR/libmy_ptmalloc.so}"
JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}"

mkdir -p "$EXTERNAL_DIR" "$RESULTS_DIR"

usage() {
  cat <<'USAGE'
usage: scripts/external_bench.sh COMMAND [options]

commands:
  setup-mimalloc-bench
      Clone or update daanx/mimalloc-bench under external/.

  build-mimalloc-bench [bench|alloc|all]
      Run mimalloc-bench build-bench-env.sh. Default: bench.

  run-mimalloc-bench [bench-name ...]
      Run selected mimalloc-bench tests with glibc sys, my_malloc hybrid,
      and my_malloc ptmalloc modes. Default tests: larson alloc-test cscratch.

  run-glibc-benchtests GLIBC_SRC [bench-name ...]
      Run malloc-related glibc benchtests from an existing glibc source/build tree.
      This script does not download glibc. Default names are malloc-thread,
      malloc-simple, malloc-tcache if present in that source tree.

  run-real-apps
      Run installed real-application smoke workloads with LD_PRELOAD where tools
      are available. Missing tools are reported and skipped.

  run-all
      Run setup-mimalloc-bench, run-mimalloc-bench, run-glibc-benchtests if
      GLIBC_SRC is set, and run-real-apps.

environment:
  BUILD_DIR         default: ./build
  EXTERNAL_DIR      default: ./external
  RESULTS_DIR       default: ./results/external
  MY_MALLOC_SO      default: ./build/libmy_ptmalloc.so
  JOBS              default: online CPU count
  GLIBC_SRC         optional path for run-all glibc benchtests
  MIMALLOC_TESTS    optional space-separated list for run-all/run-mimalloc-bench

examples:
  scripts/external_bench.sh setup-mimalloc-bench
  scripts/external_bench.sh build-mimalloc-bench bench
  scripts/external_bench.sh run-mimalloc-bench larson alloc-test cscratch
  GLIBC_SRC=/path/to/glibc scripts/external_bench.sh run-glibc-benchtests "$GLIBC_SRC"
  scripts/external_bench.sh run-real-apps
USAGE
}

need_file() {
  local path="$1"
  if [[ ! -e "$path" ]]; then
    echo "missing required file: $path" >&2
    exit 2
  fi
}

need_my_malloc() {
  need_file "$MY_MALLOC_SO"
}

timestamp() {
  date +"%Y%m%d-%H%M%S"
}

log_path() {
  local name="$1"
  echo "$RESULTS_DIR/${name}-$(timestamp).log"
}

run_logged() {
  local name="$1"
  shift
  local out
  out="$(log_path "$name")"
  echo "==> $*" | tee "$out"
  "$@" 2>&1 | tee -a "$out"
  echo "wrote $out"
}

setup_mimalloc_bench() {
  local dir="$EXTERNAL_DIR/mimalloc-bench"
  if [[ -d "$dir/.git" ]]; then
    run_logged "mimalloc-bench-update" git -C "$dir" pull --ff-only
  else
    run_logged "mimalloc-bench-clone" git clone https://github.com/daanx/mimalloc-bench "$dir"
  fi
}

build_mimalloc_bench() {
  local mode="${1:-bench}"
  local dir="$EXTERNAL_DIR/mimalloc-bench"
  need_file "$dir/build-bench-env.sh"
  local out
  out="$(log_path "mimalloc-bench-build-$mode")"
  echo "==> cd $dir && ./build-bench-env.sh $mode" | tee "$out"
  if (
    cd "$dir"
    ./build-bench-env.sh "$mode"
  ) 2>&1 | tee -a "$out"; then
    echo "wrote $out"
    return 0
  fi
  if [[ "$mode" != "bench" ]]; then
    echo "mimalloc-bench build failed; fallback build is only supported for mode=bench" | tee -a "$out"
    return 1
  fi
  echo "official mimalloc-bench build failed; trying local core-benchmark fallback" | tee -a "$out"
  (
    cd "$dir"
    mkdir -p bench/shbench
    if [[ ! -f bench/shbench/sh6bench-new.c ]]; then
      printf '#include <stdio.h>\nint main(void){ puts("sh6bench stub: skipped by my_malloc integration"); return 0; }\n' > bench/shbench/sh6bench-new.c
    fi
    if [[ ! -f bench/shbench/sh8bench-new.c ]]; then
      printf '#include <stdio.h>\nint main(void){ puts("sh8bench stub: skipped by my_malloc integration"); return 0; }\n' > bench/shbench/sh8bench-new.c
    fi
    cmake -B out/bench -S bench -DCMAKE_BUILD_TYPE=Release
    cmake --build out/bench --parallel "$JOBS"
  ) 2>&1 | tee -a "$out"
  echo "wrote $out"
}

find_mimalloc_bench_dir() {
  local dir="$EXTERNAL_DIR/mimalloc-bench/out/bench"
  if [[ ! -d "$dir" ]]; then
    echo "mimalloc-bench output directory not found: $dir" >&2
    echo "run: scripts/external_bench.sh build-mimalloc-bench bench" >&2
    exit 2
  fi
  echo "$dir"
}

run_mimalloc_one() {
  local bench_dir="$1"
  local allocator_label="$2"
  local benchmark="$3"
  local out
  out="$(log_path "mimalloc-${allocator_label}-${benchmark}")"
  echo "==> mimalloc-bench allocator=$allocator_label benchmark=$benchmark" | tee "$out"
  local env_args=()
  case "$allocator_label" in
    glibc) env_args=(SYSMALLOC=1);;
    my-hybrid) env_args=(LD_PRELOAD="$MY_MALLOC_SO" MY_MALLOC_MODE=hybrid);;
    my-ptmalloc) env_args=(LD_PRELOAD="$MY_MALLOC_SO" MY_MALLOC_MODE=ptmalloc);;
    *) echo "unknown allocator label: $allocator_label" | tee -a "$out"; return 0;;
  esac

  local cmd=()
  local stdin_file=""
  case "$benchmark" in
    larson) cmd=(./larson 5 8 1000 5000 100 4141 "$JOBS");;
    larson-sized) cmd=(./larson-sized 5 8 1000 5000 100 4141 "$JOBS");;
    alloc-test) cmd=(./alloc-test "$JOBS");;
    cscratch|cache-scratch) cmd=(./cache-scratch "$JOBS" 1000 1 2000000 "$JOBS");;
    cthrash|cache-thrash) cmd=(./cache-thrash "$JOBS" 1000 1 2000000 "$JOBS");;
    xmalloc-test) cmd=(./xmalloc-test -w "$JOBS" -t 5 -s 64);;
    glibc-simple) cmd=(./glibc-simple);;
    glibc-thread) cmd=(./glibc-thread "$JOBS");;
    malloc-large) cmd=(./malloc-large);;
    mstress) cmd=(./mstress "$JOBS" 50 25);;
    mleak) cmd=(./mleak 50);;
    rptest) cmd=(./rptest "$JOBS" 0 1 2 500 1000 100 8 16000);;
    cfrac) cmd=(./cfrac 17545186520507317056371138836327483792789528);;
    espresso) cmd=(./espresso ../../bench/espresso/largest.espresso);;
    barnes)
      cmd=(./barnes)
      stdin_file="$bench_dir/../../bench/barnes/input"
      ;;
    *)
      echo "unknown or unsupported direct mimalloc-bench test: $benchmark" | tee -a "$out"
      echo "supported: larson larson-sized alloc-test cscratch cthrash xmalloc-test glibc-simple glibc-thread malloc-large mstress mleak rptest cfrac espresso barnes" | tee -a "$out"
      return 0
      ;;
  esac

  set +e
  (
    cd "$bench_dir"
    if [[ -n "$stdin_file" ]]; then
      /usr/bin/time -f "elapsed=%E peak_rss_kb=%M user=%U sys=%S major_faults=%F minor_faults=%R" \
        env "${env_args[@]}" "${cmd[@]}" < "$stdin_file"
    else
      /usr/bin/time -f "elapsed=%E peak_rss_kb=%M user=%U sys=%S major_faults=%F minor_faults=%R" \
        env "${env_args[@]}" "${cmd[@]}"
    fi
  ) 2>&1 | tee -a "$out"
  local status=${PIPESTATUS[0]}
  set -e
  echo "exit_status=$status" | tee -a "$out"
  echo "wrote $out"
}

run_mimalloc_bench() {
  need_my_malloc
  local bench_dir
  bench_dir="$(find_mimalloc_bench_dir)"
  local tests=("$@")
  if [[ ${#tests[@]} -eq 0 ]]; then
    if [[ -n "${MIMALLOC_TESTS:-}" ]]; then
      read -r -a tests <<< "$MIMALLOC_TESTS"
    else
      tests=(larson alloc-test cscratch)
    fi
  fi

  for test_name in "${tests[@]}"; do
    if [[ "$test_name" == "cache-scratch" ]]; then
      test_name="cscratch"
    fi
    run_mimalloc_one "$bench_dir" "glibc" "$test_name"
    run_mimalloc_one "$bench_dir" "my-hybrid" "$test_name"
    run_mimalloc_one "$bench_dir" "my-ptmalloc" "$test_name"
  done
}

run_glibc_benchtests() {
  local glibc_src="${1:-${GLIBC_SRC:-}}"
  if [[ -z "$glibc_src" ]]; then
    echo "GLIBC_SRC is required for glibc benchtests" >&2
    exit 2
  fi
  shift || true
  need_my_malloc
  need_file "$glibc_src/benchtests"

  local benches=("$@")
  if [[ ${#benches[@]} -eq 0 ]]; then
    benches=(malloc-thread malloc-simple malloc-tcache)
  fi

  for bench in "${benches[@]}"; do
    if ! find "$glibc_src/benchtests" -maxdepth 2 -name "bench-${bench}*" | grep -q .; then
      echo "skip missing glibc benchtest: $bench"
      continue
    fi
    run_logged "glibc-benchtest-${bench}-glibc" make -C "$glibc_src" "bench-${bench}"
    run_logged "glibc-benchtest-${bench}-my-hybrid" env LD_PRELOAD="$MY_MALLOC_SO" MY_MALLOC_MODE=hybrid make -C "$glibc_src" "bench-${bench}"
    run_logged "glibc-benchtest-${bench}-my-ptmalloc" env LD_PRELOAD="$MY_MALLOC_SO" MY_MALLOC_MODE=ptmalloc make -C "$glibc_src" "bench-${bench}"
  done
}

run_real_apps() {
  need_my_malloc
  local out
  out="$(log_path "real-apps")"
  echo "==> real application smoke benchmarks" | tee "$out"

  if command -v sqlite3 >/dev/null 2>&1; then
    echo "== sqlite3 ==" | tee -a "$out"
    env LD_PRELOAD="$MY_MALLOC_SO" MY_MALLOC_MODE=hybrid sqlite3 :memory: \
      "create table t(x text); with recursive c(i) as (select 1 union all select i+1 from c where i<20000) insert into t select printf('%08x', i) from c; select count(*), max(x) from t;" \
      2>&1 | tee -a "$out"
  else
    echo "skip sqlite3: not installed" | tee -a "$out"
  fi

  if command -v clang++ >/dev/null 2>&1; then
    echo "== clang++ ==" | tee -a "$out"
    env LD_PRELOAD="$MY_MALLOC_SO" MY_MALLOC_MODE=hybrid clang++ -std=c++17 -O2 -fsyntax-only "$ROOT_DIR/tools/bench_runner.cpp" \
      -I"$ROOT_DIR/include" -I"$ROOT_DIR/tools" 2>&1 | tee -a "$out"
  else
    echo "skip clang++: not installed" | tee -a "$out"
  fi

  if command -v lua >/dev/null 2>&1; then
    echo "== lua ==" | tee -a "$out"
    env LD_PRELOAD="$MY_MALLOC_SO" MY_MALLOC_MODE=hybrid lua -e 'local t={}; for i=1,200000 do t[i]=tostring(i)..":"..tostring(i*i) end; print(#t,t[199999])' \
      2>&1 | tee -a "$out"
  else
    echo "skip lua: not installed" | tee -a "$out"
  fi

  if command -v z3 >/dev/null 2>&1; then
    echo "== z3 ==" | tee -a "$out"
    printf '(set-logic QF_LIA)\n(declare-const x Int)\n(assert (> x 10))\n(check-sat)\n' |
      env LD_PRELOAD="$MY_MALLOC_SO" MY_MALLOC_MODE=hybrid z3 -in 2>&1 | tee -a "$out"
  else
    echo "skip z3: not installed" | tee -a "$out"
  fi

  if command -v redis-server >/dev/null 2>&1; then
    echo "redis-server detected, but no server benchmark is run by default; use redis-benchmark in your environment." | tee -a "$out"
  else
    echo "skip redis-server: not installed" | tee -a "$out"
  fi

  echo "wrote $out"
}

run_all() {
  setup_mimalloc_bench
  if [[ -d "$EXTERNAL_DIR/mimalloc-bench/out/bench" ]]; then
    run_mimalloc_bench
  else
    echo "mimalloc-bench is cloned but not built; run build-mimalloc-bench before run-all can execute it" >&2
  fi
  if [[ -n "${GLIBC_SRC:-}" ]]; then
    run_glibc_benchtests "$GLIBC_SRC"
  else
    echo "skip glibc benchtests: GLIBC_SRC not set"
  fi
  run_real_apps
}

cmd="${1:-}"
case "$cmd" in
  setup-mimalloc-bench)
    shift
    setup_mimalloc_bench "$@"
    ;;
  build-mimalloc-bench)
    shift
    build_mimalloc_bench "$@"
    ;;
  run-mimalloc-bench)
    shift
    run_mimalloc_bench "$@"
    ;;
  run-glibc-benchtests)
    shift
    run_glibc_benchtests "$@"
    ;;
  run-real-apps)
    shift
    run_real_apps "$@"
    ;;
  run-all)
    shift
    run_all "$@"
    ;;
  ""|--help|-h|help)
    usage
    ;;
  *)
    echo "unknown command: $cmd" >&2
    usage >&2
    exit 2
    ;;
esac
