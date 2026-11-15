"""Word BST, without parent fields. Run with Python 3."""
from dataclasses import dataclass


def normalize(word):
    if not isinstance(word, str) or not word or not word.isascii() or not word.isalpha():
        raise ValueError("Enter one English word using letters A-Z only.")
    return word.lower()


@dataclass(slots=True)
class Node:
    key: str
    count: int = 1
    left: "Node | None" = None
    right: "Node | None" = None


class WordBST:
    def __init__(self):
        self.root = None

    def tree_search(self, word):
        key = normalize(word)
        node = self.root
        while node is not None:
            if key == node.key:
                return node.count
            node = node.left if key < node.key else node.right
        return 0

    def tree_insert(self, word):
        key = normalize(word)
        parent, node = None, self.root
        while node is not None:
            if key == node.key:
                previous = node.count
                node.count += 1
                return previous
            parent = node
            node = node.left if key < node.key else node.right
        new = Node(key)
        if parent is None:
            self.root = new
        elif key < parent.key:
            parent.left = new
        else:
            parent.right = new
        return 0

    def tree_delete(self, word):
        key = normalize(word)
        parent, node = None, self.root
        while node is not None and node.key != key:
            parent = node
            node = node.left if key < node.key else node.right
        if node is None:
            return 0
        previous = node.count
        if previous > 1:
            node.count -= 1
            return previous
        if node.left is not None and node.right is not None:
            successor_parent, successor = node, node.right
            while successor.left is not None:
                successor_parent, successor = successor, successor.left
            # Move the successor's entire word/count pair; remove its node.
            node.key, node.count = successor.key, successor.count
            if successor_parent is node:
                successor_parent.right = successor.right
            else:
                successor_parent.left = successor.right
            return previous
        child = node.left if node.left is not None else node.right
        if parent is None:
            self.root = child
        elif parent.left is node:
            parent.left = child
        else:
            parent.right = child
        return previous

    def tree_walk(self):
        stack, node = [], self.root
        while node is not None or stack:
            while node is not None:
                stack.append(node)
                node = node.left
            node = stack.pop()
            print(f"{node.key}({node.count})")
            node = node.right

    def tree_print(self):
        if self.root is None:
            print("(empty)")
            return
        stack = [(self.root, 0, "Root")]
        while stack:
            node, depth, label = stack.pop()
            print("  " * depth + f"{label}: " +
                  ("-" if node is None else f"{node.key}({node.count})"))
            if node is not None:
                stack.append((node.right, depth + 1, "R"))
                stack.append((node.left, depth + 1, "L"))


def main():
    tree = WordBST()
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
