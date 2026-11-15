"""PT_06: extend the PT_05 word BST into a parent-free AVL tree.

Run with Python 3.10+: python word_avl.py
The five tree_* operations and command interface follow PT_05.
Duplicates are ignored. Empty height is -1; leaf height is 0.
Search returns 0/1 membership; insert/delete return membership BEFORE update.
"""
from dataclasses import dataclass


def normalize(word):
    if not isinstance(word, str) or not word or not word.isascii() or not word.isalpha():
        raise ValueError("Enter one English word using letters A-Z only.")
    return word.lower()


@dataclass(slots=True)
class Node:
    key: str
    left: "Node | None" = None
    right: "Node | None" = None
    height: int = 0


class WordAVL:
    def __init__(self):
        self.root = None

    @staticmethod
    def _height(node):
        return -1 if node is None else node.height

    @classmethod
    def _update_height(cls, node):
        node.height = 1 + max(cls._height(node.left), cls._height(node.right))

    @classmethod
    def _balance(cls, node):
        return 0 if node is None else cls._height(node.left) - cls._height(node.right)

    @classmethod
    def _rotate_right(cls, node):
        new_root = node.left
        middle = new_root.right
        new_root.right = node
        node.left = middle
        # The new root's height depends on the demoted node's updated height.
        cls._update_height(node)
        cls._update_height(new_root)
        return new_root

    @classmethod
    def _rotate_left(cls, node):
        new_root = node.right
        middle = new_root.left
        new_root.left = node
        node.right = middle
        cls._update_height(node)
        cls._update_height(new_root)
        return new_root

    @classmethod
    def _rebalance(cls, node):
        cls._update_height(node)
        if cls._balance(node) > 1:
            if cls._balance(node.left) < 0:  # LR
                node.left = cls._rotate_left(node.left)
            return cls._rotate_right(node)  # LL or second half of LR
        if cls._balance(node) < -1:
            if cls._balance(node.right) > 0:  # RL
                node.right = cls._rotate_right(node.right)
            return cls._rotate_left(node)  # RR or second half of RL
        # A heavy child with balance 0 uses a single rotation after deletion.
        return node

    def tree_search(self, word):
        key = normalize(word)
        node = self.root
        while node is not None:
            if key == node.key:
                return 1
            node = node.left if key < node.key else node.right
        return 0

    @classmethod
    def _insert(cls, node, key):
        """Return (new local root, membership before insertion)."""
        if node is None:
            return Node(key), 0
        if key < node.key:
            node.left, previous = cls._insert(node.left, key)
        elif key > node.key:
            node.right, previous = cls._insert(node.right, key)
        else:
            return node, 1  # Ignore the duplicate: no counter increment.
        if previous == 1:
            return node, 1  # Duplicate changes neither structure nor height.
        return cls._rebalance(node), previous

    def tree_insert(self, word):
        key = normalize(word)
        self.root, previous = self._insert(self.root, key)
        return previous

    @classmethod
    def _delete(cls, node, key):
        """Return (new local root, membership before deletion)."""
        if node is None:
            return None, 0
        if key < node.key:
            node.left, previous = cls._delete(node.left, key)
        elif key > node.key:
            node.right, previous = cls._delete(node.right, key)
        else:
            previous = 1
            if node.left is None:
                return node.right, previous
            if node.right is None:
                return node.left, previous
            successor = node.right
            while successor.left is not None:
                successor = successor.left
            node.key = successor.key
            node.right, _ = cls._delete(node.right, successor.key)
        if previous == 0:
            return node, 0  # Absent key: nothing changed.
        # Rebalance at EVERY ancestor, even after a repair deeper in the tree.
        return cls._rebalance(node), previous

    def tree_delete(self, word):
        key = normalize(word)
        self.root, previous = self._delete(self.root, key)
        return previous

    def tree_walk(self):
        # Keep the PT_05 iterative inorder traversal; counts are no longer stored.
        stack, node = [], self.root
        while node is not None or stack:
            while node is not None:
                stack.append(node)
                node = node.left
            node = stack.pop()
            print(node.key)
            node = node.right

    def tree_print(self):
        # Keep the PT_05 Root/L/R layout; add height and balance annotations.
        if self.root is None:
            print("(empty)")
            return
        stack = [(self.root, 0, "Root")]
        while stack:
            node, depth, label = stack.pop()
            detail = "-" if node is None else (
                f"{node.key} [h={node.height}, bf={self._balance(node)}]"
            )
            print("  " * depth + f"{label}: " + detail)
            if node is not None:
                stack.append((node.right, depth + 1, "R"))
                stack.append((node.left, depth + 1, "L"))


def main():
    tree = WordAVL()
    print("Commands: search WORD | insert WORD | delete WORD | walk | print | quit")
    while True:
        try:
            line = input("> ")
        except EOFError:
            break
        parts = line.split()
        if not parts:
            continue
        command = parts[0].lower()
        if command == "quit" and len(parts) == 1:
            print("Goodbye.")
            break
        if command in ("walk", "print") and len(parts) == 1:
            getattr(tree, "tree_" + command)()
        elif command in ("search", "insert", "delete") and len(parts) == 2:
            try:
                print(getattr(tree, "tree_" + command)(parts[1]))
            except ValueError as error:
                print(error)
        else:
            print("Invalid command. Use search/insert/delete WORD, walk, print, or quit.")


if __name__ == "__main__":
    main()
