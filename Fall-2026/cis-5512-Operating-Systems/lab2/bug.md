### Bug 1

One of the most common bugs is forgetting to close unused pipe description 

That cause 'wc' or 'grep' to wait forever

## Report

Because the process is still waiting for 'EOF' but 
some process still has the pipe write-end open. 