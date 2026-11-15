"""CIS 5511 - Order-m B-tree of integers.

The order m is the maximum number of children per node, as defined in
Professor Wang's B-tree lecture. Run this file to reproduce the m=4 and m=5
tests in the assignment.
"""

from bisect import bisect_left


class BTreeNode:
    def __init__(self, keys=None, children=None):
        self.keys = [] if keys is None else keys
        self.children = [] if children is None else children

    def is_leaf(self):
        return len(self.children) == 0


class BTree:
    def __init__(self, m):
        if not isinstance(m, int) or isinstance(m, bool) or m < 3:
            raise ValueError("B-tree order m must be an integer >= 3")
        self.m = m
        self.max_keys = m - 1
        self.min_keys = (m + 1) // 2 - 1  # ceil(m/2)-1
        self.root = BTreeNode()

    def search(self, value):
        node = self.root
        while True:
            i = bisect_left(node.keys, value)
            if i < len(node.keys) and node.keys[i] == value:
                return True
            if node.is_leaf():
                return False
            node = node.children[i]

    def insert(self, value):
        if self.search(value):
            return False  # Ignore duplicate integers.
        split = self._insert(self.root, value)
        if split is not None:
            promoted, right = split
            self.root = BTreeNode([promoted], [self.root, right])
        return True

    def _insert(self, node, value):
        i = bisect_left(node.keys, value)
        if node.is_leaf():
            node.keys.insert(i, value)
        else:
            split = self._insert(node.children[i], value)
            if split is not None:
                promoted, right = split
                node.keys.insert(i, promoted)
                node.children.insert(i + 1, right)

        if len(node.keys) <= self.max_keys:
            return None

        # There are now m keys. With even m, use the upper middle key.
        middle = len(node.keys) // 2
        promoted = node.keys[middle]
        right = BTreeNode(node.keys[middle + 1:], node.children[middle + 1:])
        node.keys = node.keys[:middle]
        node.children = node.children[:middle + 1]
        return promoted, right

    def delete(self, value):
        if not self.search(value):
            return False
        self._delete(self.root, value)
        if len(self.root.keys) == 0 and not self.root.is_leaf():
            self.root = self.root.children[0]
        return True

    def _delete(self, node, value):
        i = bisect_left(node.keys, value)
        if i < len(node.keys) and node.keys[i] == value:
            if node.is_leaf():
                node.keys.pop(i)
            else:
                # Lecture convention: replace an internal key by its successor.
                successor_node = node.children[i + 1]
                while not successor_node.is_leaf():
                    successor_node = successor_node.children[0]
                successor = successor_node.keys[0]
                node.keys[i] = successor
                self._delete(node.children[i + 1], successor)
                self._repair_underflow(node, i + 1)
        else:
            self._delete(node.children[i], value)
            self._repair_underflow(node, i)

    def _repair_underflow(self, parent, i):
        child = parent.children[i]
        if len(child.keys) >= self.min_keys:
            return

        # Borrow from the right sibling first, if it has an extra key.
        if i + 1 < len(parent.children):
            right = parent.children[i + 1]
            if len(right.keys) > self.min_keys:
                child.keys.append(parent.keys[i])
                parent.keys[i] = right.keys.pop(0)
                if not right.is_leaf():
                    child.children.append(right.children.pop(0))
                return

        # Otherwise borrow from the left sibling if possible.
        if i > 0:
            left = parent.children[i - 1]
            if len(left.keys) > self.min_keys:
                child.keys.insert(0, parent.keys[i - 1])
                parent.keys[i - 1] = left.keys.pop()
                if not left.is_leaf():
                    child.children.insert(0, left.children.pop())
                return

        # Both siblings are at minimum occupancy: merge through parent.
        if i + 1 < len(parent.children):
            right = parent.children.pop(i + 1)
            child.keys.append(parent.keys.pop(i))
            child.keys.extend(right.keys)
            child.children.extend(right.children)
        else:
            left = parent.children[i - 1]
            left.keys.append(parent.keys.pop(i - 1))
            left.keys.extend(child.keys)
            left.children.extend(child.children)
            parent.children.pop(i)

    def inorder(self):
        result = []

        def walk(node):
            if node.is_leaf():
                result.extend(node.keys)
            else:
                for i, key in enumerate(node.keys):
                    walk(node.children[i])
                    result.append(key)
                walk(node.children[-1])

        walk(self.root)
        return result

    def display(self):
        """Print the nodes at each level, from left to right."""
        if not self.root.keys:
            print("Level 0: []")
            return
        level = [self.root]
        depth = 0
        while level:
            print(f"Level {depth}: " + " | ".join(
                "[" + ", ".join(map(str, node.keys)) + "]" for node in level
            ))
            level = [child for node in level for child in node.children]
            depth += 1

    def validate(self):
        """Raise AssertionError if any B-tree invariant has been broken."""
        leaf_depths = set()

        def check(node, low, high, depth, root=False):
            assert len(node.keys) <= self.max_keys
            assert root or len(node.keys) >= self.min_keys
            if root and node.children:
                assert len(node.keys) >= 1
            assert all(low < key < high for key in node.keys)
            assert all(node.keys[i] < node.keys[i + 1]
                       for i in range(len(node.keys) - 1))
            if node.is_leaf():
                leaf_depths.add(depth)
                return
            assert len(node.children) == len(node.keys) + 1
            bounds = [low] + node.keys + [high]
            for i, child in enumerate(node.children):
                check(child, bounds[i], bounds[i + 1], depth + 1)

        check(self.root, float("-inf"), float("inf"), 0, root=True)
        assert len(leaf_depths) == 1


def run_assignment_test(m):
    tree = BTree(m)
    print(f"========== B-tree order m = {m} ==========")
    print(f"Created empty B-tree({m}).")
    for value in range(1, 21):
        tree.insert(value)
        tree.validate()
        assert tree.inorder() == list(range(1, value + 1))
    print("Called insert(i) for i = 1, 2, ..., 20.")
    print("Tree after inserting 1 through 20:")
    tree.display()

    for value in range(2, 21, 2):
        tree.delete(value)
        tree.validate()
        assert tree.inorder() == [n for n in range(1, 21)
                                  if n % 2 == 1 or n > value]
    print("Called delete(i) for i = 2, 4, ..., 20.")
    print("Tree after deleting the even integers:")
    tree.display()
    print("Remaining keys:", tree.inorder())
    print("Validation: passed after every operation.")
    print()


if __name__ == "__main__":
    run_assignment_test(4)
    run_assignment_test(5)
