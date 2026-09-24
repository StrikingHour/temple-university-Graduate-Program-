#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

class WordPriorityQueue
{
private:
    vector<string> A;
    int heapSize;

    // Return the parent index.
    int parent(int i) const
    {
        return i / 2;
    }

    // Return the left child index.
    int left(int i) const
    {
        return 2 * i;
    }

    // Return the right child index.
    int right(int i) const
    {
        return 2 * i + 1;
    }

    // Restore the min-heap property starting at index i.
    void minHeapify(int i)
    {
        int l = left(i);
        int r = right(i);
        int smallest = i;

        if (l <= heapSize && A[l] < A[smallest])
        {
            smallest = l;
        }

        if (r <= heapSize && A[r] < A[smallest])
        {
            smallest = r;
        }

        if (smallest != i)
        {
            swap(A[i], A[smallest]);
            minHeapify(smallest);
        }
    }

    // Recursive helper function for search.
    int searchFrom(int i, const string &word) const
    {
        // Index is outside the heap.
        if (i > heapSize)
        {
            return -1;
        }

        // Word found.
        if (A[i] == word)
        {
            return i;
        }

        /*
         * Min-heap pruning:
         *
         * If word < A[i], then the word cannot occur
         * below A[i], because all descendants are
         * greater than or equal to A[i].
         */
        if (word < A[i])
        {
            return -1;
        }

        // Search the left subtree.
        int result = searchFrom(left(i), word);

        if (result != -1)
        {
            return result;
        }

        // Search the right subtree.
        return searchFrom(right(i), word);
    }

public:
    WordPriorityQueue()
    {
        /*
         * A[0] is unused so that the implementation
         * follows the 1-based indexing used in the lecture.
         */
        A.push_back("");
        heapSize = 0;
    }

    // Return the number of words in the queue.
    int size() const
    {
        return heapSize;
    }

    // Search for word and return its heap index or -1.
    int search(const string &word) const
    {
        return searchFrom(1, word);
    }

    // Add word if it is not already present.
    bool insert(const string &word)
    {
        // Do not add duplicates.
        if (search(word) != -1)
        {
            return false;
        }

        // Add the word at the end of the heap.
        A.push_back(word);
        heapSize++;

        int i = heapSize;

        // Move the new word upward if necessary.
        while (i > 1 && A[parent(i)] > A[i])
        {
            swap(A[i], A[parent(i)]);
            i = parent(i);
        }

        return true;
    }

    // Remove and return the first word in dictionary order.
    string extractMin()
    {
        if (heapSize == 0)
        {
            return "";
        }

        string first = A[1];

        // Move the last element to the root.
        A[1] = A[heapSize];

        A.pop_back();
        heapSize--;

        // Restore the min-heap property.
        if (heapSize > 0)
        {
            minHeapify(1);
        }

        return first;
    }

    // Display the current heap array.
    void printHeap() const
    {
        cout << "Heap:";

        for (int i = 1; i <= heapSize; i++)
        {
            cout << " [" << i << "]=" << A[i];
        }

        cout << endl;
    }
};


int main()
{
    WordPriorityQueue queue;

    int numberOfOperations;
    cin >> numberOfOperations;

    for (int i = 0; i < numberOfOperations; i++)
    {
        string operation;
        cin >> operation;

        if (operation == "ADD")
        {
            string word;
            cin >> word;

            if (queue.insert(word))
            {
                cout << "Added: " << word << endl;
            }
            else
            {
                cout << word << " already exists" << endl;
            }
        }

        else if (operation == "SIZE")
        {
            cout << "Size: "
                 << queue.size()
                 << endl;
        }

        else if (operation == "SEARCH")
        {
            string word;
            cin >> word;

            cout << "Search "
                 << word
                 << ": "
                 << queue.search(word)
                 << endl;
        }

        else if (operation == "REMOVE")
        {
            string word = queue.extractMin();

            if (word == "")
            {
                cout << "Queue is empty" << endl;
            }
            else
            {
                cout << "Removed: "
                     << word
                     << endl;
            }
        }

        else if (operation == "PRINT")
        {
            queue.printHeap();
        }
    }

    return 0;
}