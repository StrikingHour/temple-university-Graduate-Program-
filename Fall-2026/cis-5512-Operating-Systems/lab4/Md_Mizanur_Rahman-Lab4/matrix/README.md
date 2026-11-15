# Matrix stage for Lab4

Requires the previously installed shell, process-operations, API-library and launcher updates.
This update adds include/tuplelib.h, netlib/tuplelib.c, updates netlib/makefile,
and adds matrix/{client.c,worker.c,common.h,makefile}.

Build from lab4:
    make -C netlib
    make -C matrix

Start your daemon with TSHPORT, TSHHOST=127.0.0.1 and an absolute MSH_PATH
exported BEFORE launching tsh. For this single-machine implementation,
TSHHOST must be 127.0.0.1 or localhost. Callback listeners bind to loopback.

In a second PuTTY session, export the SAME TSHPORT and TSHHOST, then:
    cd ~/public_html/lab4/tools
    ./launch ../matrix/worker 4 mat01
    cd ../matrix
    ./client 8 2 4 mat01

Arguments: client N G P RUN_ID; worker RUN_ID.
N:1..5000; G:1..N; P:1..64. Limit experiments to the resources assigned by
instructor. RUN_ID:1..40 letters/digits/underscores. Use a NEW run ID per run.
The run ID and P must agree between launcher and client. Only one client per run.

Algorithm:
- Reproducible pseudorandom integer-valued double matrices A and B.
- B is a READ tuple (broadcast); workers read it once.
- G-row stripes of A are uniquely named task tuples; GET claims a task.
- Workers compute i-k-j multiplication, then PUT a result with its row offset.
- Client GETs every result and assembles C, including a final partial stripe.
- After all results arrive, client sends P distinct stop tasks and waits for P
  completion records. Some workers may perform zero tasks when P exceeds work.
- Workers share all available CPUs under normal OS scheduling; launch does not
  pin a worker to a particular core.

Verification is independent i-j-k: every cell for N<=256, 100 deterministic
sampled cells otherwise. Integer-valued inputs have exactly representable sums
at the supported sizes, so comparison is exact. Sampled PASS is NOT proof of
full large-matrix correctness. Exit code 0: verification passed; 1: failure;
2: invalid CLI arguments.

parallel_seconds starts before B/config/task distribution and ends after all
results are collected. It excludes random generation, verification, initial
process startup and shutdown acknowledgments. It includes tuple communication
and worker B reads. These are local development timings, not tec2 benchmarks.
For final reporting, document timing boundaries and compare to your Lab1 best
sequential order under matching optimization/data conditions.

Toy limitations:
- The original tuple protocol sends native structs. Payloads are native doubles
  and uint32_t; intended for processes on this same machine, not mixed platforms.
- Blocking operations deliberately wait indefinitely for missing tuples/workers.
  There is no retry/recovery protocol in this matrix stage. If a worker fails,
  stop this run, kill its tracked jobs and restart your own daemon before retrying.
- B/config tuples and legacy retrieval backups remain in the daemon. Use a fresh
  daemon between large benchmark runs to release them. A unique run ID prevents
  matching old work, but does not itself free memory.
- Peak memory includes client matrices, each worker's B, daemon tuples and
  results. Start with small tests; do not run N=5000 yet.

Suggested tests (each with a fresh ID):
    launch ../matrix/worker 4 mat01 ; client 8 2 4 mat01
    launch ../matrix/worker 3 mat02 ; client 7 3 3 mat02
    launch ../matrix/worker 4 mat03 ; client 5 5 4 mat03
The commands above are conceptual pairs in tools/ and matrix/ respectively.
Do not paste them as one line from one directory.
