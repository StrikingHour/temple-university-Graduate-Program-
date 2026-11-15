# CIS5512 Lab4 - Toy Active Content Addressable Network

Student: Md. Mizanur Rahman | Fall 2026

Read `report.pdf` for implementation, measurements and AI investigation.
`ai_log.md` is a condensed account of the actual dialogue, not a verbatim export.
All successful experimental result logs are retained; source backup and empty temporary files were removed for clean submission.

## Build

On tec2, enter this extracted folder and run `make`. The root Makefile rebuilds
netlib, tsh, tools and matrix, including both sequential baselines. GCC, ar and
make are required. Python 3 is needed only for `make test` (small local functional
tests); it is not needed to run the C programs. Original legacy warnings are
recorded in evidence/review_build_*.txt. Compilation succeeded despite these.
The tests pick an ephemeral local port; assignment demonstrations use your TUID port.

## Run daemon (PuTTY window 1)

From this project root, where `$PWD` is the extracted project directory:

```bash
export TSHPORT=3848
export TSHHOST=127.0.0.1
export MSH_PATH="$PWD/tsh/myshell"
cd tsh
./tsh "$TSHPORT"
```

3848 is the port used in the submitted session. It must correspond to the
student's TUID last four digits. Set MSH_PATH again after moving/renaming the
project. Do not reuse an absolute path to an old lab4 folder.

## Tester and shell API (PuTTY window 2)

From the project root:

```bash
export TSHPORT=3848
export TSHHOST=127.0.0.1
./tsh/tshtest "$TSHPORT"
```

Menu: 1 PUT; 2 GET; 3 READ; 4 stop daemon; 5 SHELL; 6 quit tester;
7 PS; 8 KILL; 9 KILLALL. Menu 3 displays and sends TSH_OP_READ.
Legacy pattern matching is POSIX basic regex: use
`^name$` for exact matching or `^prefix.*$` for a prefix. Do not use a bare `*`.
The legacy tester accepts textual tuples; matrix binary data use tuplelib.

```bash
./tools/tshctl shell "/bin/echo Hello"
./tools/tshctl shell "/bin/sleep 30" --bg
./tools/tshctl ps
./tools/tshctl kill PID
./tools/tshctl killall
```

Replace PID with a tracked job PID from PS. Output from the command appears in
the daemon terminal, not streamed back to tshctl. Foreground returns completion
status; background returns acceptance. PS lists shell wrapper job PIDs, which
can differ from worker process PIDs. KILL and KILLALL signal tracked groups only.

## Matrix example

```bash
cd tools
./launch ../matrix/worker 4 example01
cd ../matrix
./client 8 2 4 example01
```

Usage: launch PROGRAM P [args...]; client N G P RUN_ID; worker RUN_ID.
Worker and client run IDs and P must agree. Use a new ID each time.
The client collects results, sends stop tasks, and collects worker completion
records. Check PS after each run. Wait for all jobs to finish before restarting
the daemon between large runs. Current tuple API callbacks are loopback-only;
this project implements the same-tec2 lab setting, not general multi-node ACAN.
Launch requests travel over the daemon socket; binaries must exist on that node.
No CPU pinning is performed. Only simple whitespace-free launcher arguments are
supported by the current shell parser.

## Sequential baseline

```bash
cd matrix
./seq_kij 256
./seq_kij 5000
./seq_ikj 5000
```

Each command performs one timed multiplication. Three runs of each N=5000
baseline are included. Both binaries derive from seq_kij.c; the ikj build uses
-DBASELINE_IKJ. Both use -O3, matching the worker build. Baselines adapt the Lab1
order to double and the same random input as the parallel client. See report for
timing boundaries and the distinction between order-matched and Lab1 comparisons.

## Evidence and limitations

results/: 21 successful N=5000 parallel logs, six sequential logs, N=256 full
verification, CPU/compiler inventory and summaries. Empty failed-run and backup
files were removed; failed attempts are described in the AI log.
`results/analysis.csv` contains regenerated statistics. `evidence/review_smoke.txt` records AI-side local functional checks,
not extra tec2 benchmark measurements. Review build logs contain local paths.

This is a trusted lab prototype: no authentication, replication or worker-crash
recovery; native ABI payloads; blocking operations can wait indefinitely. The
interactive tester now bounds tuple names and text lengths but is not fully
hardened for untrusted input. Regex allocations/legacy retained tuple state can
accumulate; restart the idle daemon between large benchmark runs.
Do not terminate the daemon with active jobs; SIGINT is not guaranteed to run
its atexit cleanup. KILLALL and confirm zero jobs first if abandoning a run.
