# AI-Assisted Development: Question and Response Log

CIS5512 LAB4  /  DEVELOPMENT RECORD

Md. Mizanur Rahman | Fall 2026

## Purpose and provenance

This log documents how AI was used to understand, implement, test and evaluate the toy ACAN project. It contains twenty-five entries arranged in development order. It is an expanded, edited summary of the available dialogue, not a verbatim chat export.

Each entry explicitly labels the question as verbatim, paraphrased or reconstructed. Reconstructed questions express the issue addressed in a development stage; they are not claimed to be prompts typed by the student. Response summaries synthesize the assistance provided. Evidence distinguishes student-run tec2 output from later AI-side local review. No new benchmark results are invented.

| Field | What it records |
| --- | --- |
| Question / prompt | The original wording when marked verbatim; otherwise a labelled summary of the question or issue. |
| AI response summary | The main explanation, implementation approach or troubleshooting advice. |
| Action / evidence | The actual command output, file inspection or local check supporting the entry. |
| Learning point | A technical principle illustrated by the work, not an invented statement of personal experience. |

## Development sequence

01-03: task and workflow. 04-05: tuple semantics. 06-09: shell behavior and evidence. 10-13: integration, process management and APIs. 14-18: matrix experiments and troubleshooting. 19-22: sequential baselines and analysis. 23-25: review, documentation and final corrections.

Related files: report.pdf, results/, evidence/review_smoke.txt and tests/. Student-run benchmark results are identified separately from AI-side local smoke tests. The student should verify this edited record against their experience before submission; include the original conversation if the instructor specifically requests verbatim dialogue.

QUESTIONS 01-03  /  DEVELOPMENT LOG

## 01. Understanding the assignment

Question type: Paraphrased from the assignment-opening request

What does Lab4 require, and how should I divide the work into manageable stages?

**AI response summary**

The task was organized into extending the tuple daemon, adding shell and process operations, building application APIs, implementing a launcher, and completing a matrix experiment. The report and AI investigation were treated as deliverables alongside the C programs.

**Action / evidence**

The project eventually contained separate include, netlib, tsh, tools, matrix and results directories. Final archive review checked these components against the supplied assignment.

**Learning point**

A project plan must cover both implementation and the evidence needed to explain and evaluate it.

## 02. Working with PuTTY and Notepad++

Question type: Verbatim user prompt

I want to do this assignment on putty.exe and notepad ++. give me response based on that

**AI response summary**

Instructions were adapted to editing/uploading through Notepad++ and NppFTP, while compiling and executing commands in PuTTY on tec2. Separate terminal sessions were used for the long-running daemon and the tester or client.

**Action / evidence**

The student supplied tec2 terminal output throughout development. The final README records the environment variables and directory-specific commands needed to reproduce the workflow.

**Learning point**

A successful file transfer, a successful compilation and a running service are three separate states to verify.

## 03. Incremental instructions

Question type: Verbatim user prompt

give me step by step instructions for first phase first

**AI response summary**

The work proceeded through small stages instead of installing every feature at once: first tuple behavior, then non-interactive shell execution, process operations, library calls, launcher and matrix programs.

**Action / evidence**

The conversation contains repeated command output followed by requests for the next step. Small matrix tests preceded the N=5000 experiments.

**Learning point**

Checking one stage before adding the next makes it easier to associate failures with a specific change.

QUESTIONS 04-06  /  DEVELOPMENT LOG

## 04. PUT and retrieval semantics

Question type: Reconstructed question from uploaded tuple-test screenshots

How can I confirm that PUT, READ and GET have different effects on the tuple space?

**AI response summary**

PUT inserts a named value. READ returns a matching tuple while leaving it available; GET returns and removes it. A useful test is two READs followed by GET, then another request that must wait until a new PUT.

**Action / evidence**

The supplied screenshots showed named values such as hello and Ready. Final local API tests confirmed repeated READ, destructive GET and delivery after a later PUT; their output is in evidence/review_smoke.txt.

**Learning point**

Receiving the expected value once is insufficient: the test must also check whether the tuple remains available.

## 05. Pending status versus final delivery

Question type: Reconstructed question from the status-402 screenshots

Why does a request show status 0 and error 402, yet later receive a tuple?

**AI response summary**

In this toy protocol, a missing matching tuple can cause the daemon to queue the request and send an initial not-available response. The requester then waits on a callback socket. A later PUT can satisfy the queued request.

**Action / evidence**

The screenshots show the initial status before eventual delivery. Current local tests independently exercised pending GET and pending READ, including retention after READ.

**Learning point**

A status value must be interpreted in the protocol state in which it occurs; it is not automatically the final result.

## 06. Shell command mode

Question type: Reconstructed question from submitted shell test output

How can my existing interactive shell execute one command for the daemon?

**AI response summary**

The shell was extended with a -c mode so the daemon can invoke myshell -c COMMAND without entering the interactive prompt. This reuses the shell implementation for commands, pipes and redirection.

**Action / evidence**

The student showed Hello_from_myshell, the expected pipe count of 6, and a file containing First then Second after > and >>. Interactive pwd, help and exit also remained usable.

**Learning point**

A reusable execution path supports both an interactive interface and a service-driven interface.

QUESTIONS 07-09  /  DEVELOPMENT LOG

## 07. Propagating command failures

Question type: Reconstructed question from the exit-status test output

Which exit statuses should the daemon receive when commands or pipelines fail?

**AI response summary**

The command-mode shell should propagate the command status. A missing executable returns 127, a failed command returns a nonzero value, and the tested pipeline convention uses the last stage status.

**Action / evidence**

The student supplied true=0, false=1, missing executable=127, failed cd=1, false|true=0, true|false=1 and invalid trailing pipe=1.

**Learning point**

Printing an error message is not enough: callers need a machine-readable failure status.

## 08. Source file versus executable

Question type: Reconstructed question from a terminal error

Why did ./myshell.c fail with Permission denied while ./myshell worked?

**AI response summary**

The .c file is source code. GCC produces an executable named myshell, and that is the program to run. Adding execute permission to C source is not the appropriate fix.

**Action / evidence**

The student corrected ./myshell.c to ./myshell -c and obtained the expected exit status from /bin/false.

**Learning point**

Build commands and run commands refer to different files; preserving their distinction avoids misleading permission fixes.

## 09. Preserving test evidence

Question type: Verbatim user prompt

everything was correct according to your instruction. but i didn't take screenshot. what can i do now?

**AI response summary**

The evidence strategy used reproducible commands and saved text outputs. Later benchmark instructions used tee so results could be reviewed without relying only on screenshots.

**Action / evidence**

The submitted results directory contains raw logs, CPU/compiler inventory and verification output. The final review also added explicitly labelled local functional evidence.

**Learning point**

Evidence should be captured during a test and identified by its source; a later summary should not invent missing screenshots.

QUESTIONS 10-12  /  DEVELOPMENT LOG

## 10. Integrating shell and tuple operations

Question type: Reconstructed question from the next-step requests during integration

How can shell execution be added without blocking all tuple-space activity?

**AI response summary**

The integration adds shell opcodes and a job table. Foreground requests retain their connection until completion; the main loop polls child status and continues serving requests. Background requests return after launch acceptance.

**Action / evidence**

The submitted shell_service.c implements this behavior. Final local review checked foreground success/failure and background execution through the API.

**Learning point**

Foreground waiting by one client does not require a synchronous wait that stops the entire daemon.

## 11. Process IDs and management

Question type: Reconstructed question from the process-operations stage

How should PS, KILL and KILLALL manage programs launched through the shell?

**AI response summary**

The daemon tracks its shell jobs and assigns process groups. PS reports active wrapper-job PIDs. KILL targets a tracked group; KILLALL targets all tracked groups. A worker PID printed by the program can differ from the wrapper PID.

**Action / evidence**

Local review passed background PS, KILL, unknown-PID rejection, KILLALL and subsequent reaping. These checks are functional review results, not performance measurements.

**Learning point**

Ownership and process-group boundaries prevent a management command from acting on unrelated jobs.

## 12. Reusable library and launcher

Question type: Reconstructed question from the API and launcher stages

Why create tshlib.c and launch.c instead of putting socket code in every application?

**AI response summary**

The shell API centralizes connection handling, wire encoding and replies in libsng.a. The launcher then repeatedly requests background jobs, validates its arguments and attempts rollback of already acknowledged jobs if a later request fails.

**Action / evidence**

The archive contains tshlib.c, tshlib.h, launch.c and their makefiles. Source review and functional checks verified the launcher/API integration.

**Learning point**

A library separates application intent from transport details; launch acceptance must still be distinguished from successful execution.

QUESTIONS 13-15  /  DEVELOPMENT LOG

## 13. Invalid launcher arguments

Question type: Paraphrased from the invalid-input results and next-step question

The launcher rejects P=0 and a missing worker. Are these tests behaving correctly, and what comes next?

**AI response summary**

These were expected validation failures. The next stage was to connect the tested launcher to the matrix worker rather than treating the rejected inputs as bugs.

**Action / evidence**

The student reported P must be an integer from 1 to 64 and program path: No such file or directory. The final local review reproduced both rejection cases.

**Learning point**

Negative tests establish that invalid requests fail clearly before partial work is launched.

## 14. Matrix dataflow

Question type: Reconstructed question from the matrix-stage continuation request

How should client.c and worker.c coordinate using PUT, READ and GET?

**AI response summary**

The client publishes B for non-destructive READ, partitions A into G-row task tuples, and collects result tuples. Workers GET tasks dynamically, multiply stripes, PUT results and stop after receiving dedicated stop tasks.

**Action / evidence**

Local full-verification tests included a partial final stripe and more workers than tasks. The student later supplied N=256 full verification and N=5000 sampled checks.

**Learning point**

Task consumption distributes ownership, while a retained tuple supports broadcasting shared data.

## 15. Designing the granularity study

Question type: Paraphrased from "goto next steps" after matrix setup

Which values of N, P and G should I use to study granularity?

**AI response summary**

The experiment fixed N=5000 and P=16, varied G across 50, 100, 200, 313 and 625, and repeated each setting three times. Values 312 and 314 were added near the best initial mean.

**Action / evidence**

Twenty-one successful parallel logs cover seven G values. G=313 produces sixteen tasks, whereas G=625 produces eight.

**Learning point**

Changing one factor while keeping workload and worker count fixed makes the comparison interpretable.

QUESTIONS 16-18  /  DEVELOPMENT LOG

## 16. Daemon connection failure

Question type: Paraphrased from the Connection refused output and request for directions

Both launch and client report Connection refused. What should I do now?

**AI response summary**

The advice was to verify that the daemon was running on the same host/port, set the environment consistently, check tshctl ps, and only then launch workers and client. The failed run was excluded and a fresh run ID was used.

**Action / evidence**

The failed attempt contacted 127.0.0.1:3848 and accepted zero jobs. Later g312 logs contain successful completion and verification.

**Learning point**

Check the service endpoint before changing the matrix algorithm; a transport failure is not a computation error.

## 17. Capturing failures with tee

Question type: Reconstructed question from the connection-failure recovery instructions

How can I save logs without hiding a client failure?

**AI response summary**

The revised command uses 2>&1 to capture stderr and set -o pipefail so the pipeline does not appear successful solely because tee completed. Exit status is inspected immediately after the command.

**Action / evidence**

Subsequent instructions included these settings for benchmark and sequential runs. The original failed file may lack its stderr because the first command captured stdout only.

**Learning point**

Log completeness and shell pipeline status are part of reliable testing, not just presentation.

## 18. Incomplete repeated trials

Question type: Paraphrased from the partial G=312/G=314 results

I have two runs for G=312 and one for G=314. What remains?

**AI response summary**

The response identified exactly three missing runs: g312_r3, g314_r2 and g314_r3. Existing successful logs were retained instead of restarting the whole experiment.

**Action / evidence**

The student then supplied all six nearby-G timings with sampled PASS. The final archive contains three runs for every selected G.

**Learning point**

Count successful observations per configuration before computing comparisons; do not silently average uneven trial sets.

QUESTIONS 19-21  /  DEVELOPMENT LOG

## 19. Recovering compilation settings

Question type: Verbatim user prompt

i don't remember the compilation command. if you need it need i can find it from the github folder

**AI response summary**

After inspecting kij.c, AI explained the N and RUNS macros and initially proposed a sample build. The later Lab1 archive supplied the actual historical generator command, which had no -O flag.

**Action / evidence**

Lab1/Part1/generator.sh compiles with -DN=$N -DRUNS=$RUNS. Its sizes were 1000, 2000 and 3000, with three repetitions. The proposed example was not misreported as the historical command.

**Learning point**

Configuration should be recovered from scripts or logs rather than inferred from memory.

## 20. Establishing the Lab1 baseline

Question type: Paraphrased from the uploaded "lab1 complete code"

Use my complete Lab1 archive to identify the baseline and prepare the next step.

**AI response summary**

The archive established that kij was fastest at N=1000 and N=3000, while ikj was slightly faster at N=2000. The new baseline preserved kij but matched the Lab4 double type, input generator and -O3 optimization.

**Action / evidence**

The student ran the adapted baseline three times at N=5000. The mean was 158.186233 seconds; original historical timings were not substituted for new measurements.

**Learning point**

A fair adaptation must disclose what changed, especially data type, input generation and compiler flags.

## 21. Avoiding a misleading speedup

Question type: Reconstructed question from baseline-comparison guidance

Why also measure sequential i-k-j when Lab1 used k-i-j?

**AI response summary**

The parallel worker uses ikj, so a kij-only ratio mixes loop-order effects with parallel execution. An ikj sequential build provides a same-order control. Checksum and verification also make use of the computed result explicit.

**Action / evidence**

Three ikj runs averaged 126.508117 seconds. All six sequential runs had checksum 3125109885182 and sampled PASS. The two baseline comparisons are reported separately.

**Learning point**

A convincing speedup claim identifies the baseline and avoids attributing every improvement to parallelism.

QUESTIONS 22-25  /  DEVELOPMENT LOG

## 22. Interpreting final measurements

Question type: Paraphrased from the final sequential-result submission

Here are all sequential timings and checksums. How do they compare with the parallel results?

**AI response summary**

The best tested parallel mean was 36.122096 seconds at G=313. Ratios were 4.379x versus kij and 3.502x versus ikj; efficiencies normalized by sixteen workers were 27.37% and 21.89%.

**Action / evidence**

The calculations were regenerated from the uploaded raw logs. The report notes that parallel timing includes distribution and collection, whereas sequential timing covers the multiplication kernel.

**Learning point**

Ratios need timing definitions and uncertainty context; efficiency is not the same as measured CPU utilization.

## 23. Completion and submission review

Question type: Verbatim user prompt

is my assignment complete? if so then what i need to submit?

**AI response summary**

The response distinguished completed implementation/experiments from the remaining report and packaging work. The student uploaded lab4_review.tar for checking sources, build rules and results before a named tar submission was assembled.

**Action / evidence**

Final review rebuilt main programs, exercised functional tests, documented legacy warnings and corrected the hardware description to eight physical cores/sixteen logical CPUs using cpu_info.txt.

**Learning point**

Final review should test reproducibility and reconcile documentation with the actual evidence, rather than only check that files exist.

## 24. Improving the AI investigation

Question type: Paraphrased from the current revision request

Improve the report and AI log; include many questions and a response summary for each.

**AI response summary**

The revision organizes twenty-four entries into question, response summary, evidence and learning point. It distinguishes verbatim prompts from paraphrased or reconstructed questions so an expanded log does not falsely become a transcript.

**Action / evidence**

This entry describes the current documentation revision, not an additional historical experiment. Source code and numerical measurements are unchanged.

**Learning point**

AI assistance should be documented transparently: explain what it contributed, how it was checked and where its claims were limited.

## 25. Fixing interactive tester issues and cleaning the archive

Question type: Verbatim user prompt (November 14, 2026)

if need modify this ai log. also how can i modify the 3rd and 4th issue?

**AI response summary**

The reviewer identified that the READ menu operation used the correct opcode but displayed a misleading GET label. The review also found unbounded text input in the interactive tester and extra backup or empty files in the archive. AI proposed a minimal correction, input bounds for the interactive tester, and a clean-up that retains actual experimental logs.

**Action / evidence**

A revised tester now labels READ and GET separately, bounds tuple names/patterns and textual PUT lengths, and releases a previously unfreed PUT buffer. The archived root Makefile smoke-test suite was run again after these changes. Empty logs and source backup copies were removed from the clean archive; N=5000 benchmark raw logs and the experiment's measured results were retained. These are post-experiment documentation and tester fixes, not revised experimental measurements.

**Learning point**

Separating a user-interface bug from a protocol bug prevents unnecessary changes to working tuple semantics. Retain reproducibility evidence while removing temporary submission clutter.
