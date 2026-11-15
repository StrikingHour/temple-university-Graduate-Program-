"""Shortest-processing-time scheduling using Merge Sort.

Input: n on the first line, then n nonnegative integer processing times.
Run: python task_scheduling.py
Author: Md. Mizanur Rahman
AI assistance: ChatGPT assisted with implementation and test design.
"""

import sys


def merge_sort(tasks):
    """Sort (processing_time, task_number) pairs; task number breaks ties."""
    if len(tasks) <= 1:
        return tasks[:]
    mid = len(tasks) // 2
    left = merge_sort(tasks[:mid])
    right = merge_sort(tasks[mid:])
    result = []
    i = j = 0
    while i < len(left) and j < len(right):
        if left[i] <= right[j]:
            result.append(left[i])
            i += 1
        else:
            result.append(right[j])
            j += 1
    result.extend(left[i:])
    result.extend(right[j:])
    return result


def schedule_tasks(processing_times):
    """Return (order, completion, total); completion[i] belongs to task i+1."""
    if not processing_times:
        raise ValueError("At least one task is required.")
    if any(p < 0 for p in processing_times):
        raise ValueError("Processing times must be nonnegative.")
    tasks = merge_sort([(p, i + 1) for i, p in enumerate(processing_times)])
    elapsed = total = 0
    order = []
    completion = [0] * len(tasks)
    for p, task in tasks:
        elapsed += p
        completion[task - 1] = elapsed
        total += elapsed
        order.append(task)
    return order, completion, total


def main():
    try:
        n = int(input())
        times = list(map(int, input().split()))
        if n <= 0 or len(times) != n:
            raise ValueError("Enter n > 0 and exactly n processing times.")
        order, completion, total = schedule_tasks(times)
    except (ValueError, EOFError) as error:
        print(f"Error: {error}", file=sys.stderr)
        return 1
    print("Execution order:", " -> ".join(f"a{i}" for i in order))
    print("Task Processing_time Completion_time")
    for task in order:
        print(f"a{task} {times[task - 1]} {completion[task - 1]}")
    print(f"Total completion time: {total}")
    print(f"Average completion time: {total / n:.2f}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
