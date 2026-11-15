"""CIS 5511 - Homework 8: Longest Common Substring using dynamic programming.

Run `python lcstr.py` to display nine reproducible test cases, including
the complete DP table for each case. Run `python lcstr.py --save` to also
regenerate the accompanying lcstr_output.txt file.
"""

from argparse import ArgumentParser
from pathlib import Path


def longest_common_substring(x, y):
    """Return (substring, length, start_x, start_y, table).

    table[i][j] stores the length of the longest common *suffix* of
    x[:i] and y[:j]. A mismatch resets this value to zero.
    When longest matches tie, choose the earliest start in x, then in y.
    Indices are zero-based; a missing match has positions None, None.
    """
    n, m = len(x), len(y)
    table = [[0] * (m + 1) for _ in range(n + 1)]
    best_length = 0
    best_start_x = None
    best_start_y = None

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            if x[i - 1] == y[j - 1]:
                table[i][j] = table[i - 1][j - 1] + 1
                length = table[i][j]
                start_x, start_y = i - length, j - length

                if (length > best_length or
                        (length == best_length and length > 0 and
                         (start_x, start_y) <
                         (best_start_x, best_start_y))):
                    best_length = length
                    best_start_x = start_x
                    best_start_y = start_y
            # On a mismatch table[i][j] remains zero.

    substring = (x[best_start_x:best_start_x + best_length]
                 if best_length else "")
    return substring, best_length, best_start_x, best_start_y, table


def format_dp_table(x, y, table):
    """Print all (len(x)+1)*(len(y)+1) entries with prefix-index labels."""
    column_labels = ["0:-"] + [f"{j}:{char}" for j, char in enumerate(y, 1)]
    row_labels = ["0:-"] + [f"{i}:{char}" for i, char in enumerate(x, 1)]
    row_width = max(len(label) for label in row_labels)
    cell_width = max(3, *(len(label) for label in column_labels),
                     *(len(str(value)) for row in table for value in row))

    lines = [" " * (row_width + 2) +
             " ".join(f"{label:>{cell_width}}" for label in column_labels)]
    for label, row in zip(row_labels, table):
        lines.append(f"{label:>{row_width}}  " +
                     " ".join(f"{value:>{cell_width}}" for value in row))
    return "\n".join(lines)


def check_result(x, y, result):
    """Check the DP recurrence and returned positions for this test case."""
    substring, length, start_x, start_y, table = result
    assert len(table) == len(x) + 1
    assert all(len(row) == len(y) + 1 for row in table)
    for i in range(len(x) + 1):
        for j in range(len(y) + 1):
            expected = (table[i - 1][j - 1] + 1
                        if i and j and x[i - 1] == y[j - 1] else 0)
            assert table[i][j] == expected
    assert length == max(max(row) for row in table)
    if length:
        assert x[start_x:start_x + length] == substring
        assert y[start_y:start_y + length] == substring
    else:
        assert (substring, start_x, start_y) == ("", None, None)


# (title, first string, second string, expected substring, positions)
TEST_CASES = [
    ("Partial overlap", "ABABC", "BABCA", "BABC", (1, 0)),
    ("Subsequence versus substring", "ABCDEF", "ACE", "A", (0, 0)),
    ("Long repeated overlap", "BANANA", "ANANAS", "ANANA", (1, 0)),
    ("Multiple optimal substrings", "ABABA", "BABAB", "ABAB", (0, 1)),
    ("No common character", "ABC", "XYZ", "", (None, None)),
    ("Empty first string", "", "DATA", "", (None, None)),
    ("Identical strings", "NETWORK", "NETWORK", "NETWORK", (0, 0)),
    ("Empty second string", "DATA", "", "", (None, None)),
    ("Both strings empty", "", "", "", (None, None)),
]


def make_test_output():
    lines = [
        "CIS 5511 - Programming Techniques",
        "Homework 8: Longest Common Substring (LCStr)",
        "",
        "TEST PROCEDURE",
        'Run "python lcstr.py". The nine cases below run automatically.',
        'Run "python lcstr.py --save" to regenerate this text file.',
        "Indices are zero-based. A dash labels an empty prefix in the DP table.",
        "The entry D[i][j] is the common-suffix length for X[:i], Y[:j].",
        "For ties, choose the earliest start in X, then the earliest in Y.",
        "",
    ]

    for case_number, (title, x, y, expected_text, expected_starts) in enumerate(
            TEST_CASES, 1):
        result = longest_common_substring(x, y)
        substring, length, start_x, start_y, table = result
        check_result(x, y, result)
        assert substring == expected_text
        assert (start_x, start_y) == expected_starts
        lines.extend([
            "=" * 64,
            f"CASE {case_number}: {title}",
            f"Input X: {x!r}",
            f"Input Y: {y!r}",
            f"Longest common substring: {substring!r}",
            f"Length: {length}",
            f"Start in X: {start_x if start_x is not None else 'None'}",
            f"Start in Y: {start_y if start_y is not None else 'None'}",
            "DP table D (including zero-initialized row and column):",
            format_dp_table(x, y, table),
            "",
        ])

    lines.append("All nine test cases and DP recurrence checks passed.")
    return "\n".join(lines) + "\n"


def main():
    parser = ArgumentParser(description="Longest common substring DP tests")
    parser.add_argument("--save", action="store_true",
                        help="also save the complete results as lcstr_output.txt")
    arguments = parser.parse_args()
    output = make_test_output()
    if arguments.save:
        Path(__file__).with_name("lcstr_output.txt").write_text(
            output, encoding="utf-8")
    print(output, end="")


if __name__ == "__main__":
    main()
