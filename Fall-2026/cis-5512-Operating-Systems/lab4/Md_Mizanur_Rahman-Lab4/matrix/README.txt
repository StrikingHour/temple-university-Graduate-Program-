Lab4 sequential baseline, adapted from uploaded Lab1/Part1/kij.c.

Build on tec2 in lab4/matrix:
gcc -O3 -Wall -Wextra -std=c11 seq_kij.c -lm -o seq_kij
./seq_kij 8
./seq_kij 256
./seq_kij 5000

One invocation = one timed multiplication. Run 5000 three separate times.
No daemon/workers required. Stop other personal benchmarks during measurement.
Heap double arrays and random generator match current Lab4 client.c, including
seed 123456789, LCG constants 1664525 and 1013904223, modulo 11, interleaved A/B.
Timing excludes allocation, initialization, checksum and verification. Parallel
client time includes distribution, worker computation and result collection;
it excludes input initialization, process launch and shutdown acknowledgments.
Thus comparison measures a sequential kernel versus tuple-based completion time.
Report this scope explicitly. This is an adapted Lab1 baseline, not the unchanged
integer program or its old timing results.

The original Lab1 generator.sh compiled without an optimization flag:
gcc -DN=$N -DRUNS=$RUNS -o "$order" "$order.c"
Sizes were 1000, 2000, 3000; RUNS=3.
Saved results: kij fastest for 1000 and 3000; ikj fastest for 2000.
At N=3000 kij=84.45096078 s and ikj=84.76372079 s. Small differences do not
establish a universal fastest order. Historical machine/compiler are unverified.

The default baseline retains k-i-j, while current workers use i-k-j.
The resulting speedup includes loop-order/cache effects as well as parallelism.
For an additional same-order control (recommended), compile the identical file:
gcc -O3 -Wall -Wextra -std=c11 -DBASELINE_IKJ seq_kij.c -lm -o seq_ikj
./seq_ikj 5000
Report kij and ikj baselines separately; do not silently substitute one for the
other. Inputs, output checksum and verification should match for both builds.

Verification: full independent i-j-k check up to N=256; 100 deterministic cells
above that. The checksum reads the entire result but is not a correctness proof.
The 0..10 integer-valued doubles and supported sizes make sums exact. No fast-math
or architecture-specific flags added. Original uploaded Lab1 files unchanged.
