#include<bits/stdc++.h>

using namespace std;

long long insertionShifts = 0;     // calculate the number of shifts for insertion sort
long long insertComparisons = 0;


long long mergeCopies = 0;
long long mergeComparisons = 0;

void insertion_sort(vector<int> &A, int n)
{
    for(int i = 2; i <=n; i++){
        int key = A[i];
        int j = i - 1;
        while( j > 0 && A[j] > key){
            insertComparisons++;
            A[j + 1] = A[j];
            insertionShifts++;
            j -= 1;
        }
        A[j + 1] = key;
    }

}


void merge(vector<int>&A, int p, int q, int r)
{
    int nl = q - p + 1; // A[p:q]
    int nr = r - q; // A[q + 1:r]

    vector<int> L(nl); // L[0:nl - 1]
    vector<int> R(nr); // R[0: nr - 1]

    for(int i = 0; i < nl; i++) L[i] = A[p + i]; mergeCopies++;
    for(int j = 0; j < nr; j++) R[j] = A[q + j + 1]; mergeCopies++;

    int i = 0;
    int j  = 0;
    int k = p;

    while(i < nl &&  j < nr){
        mergeComparisons++;
        if(L[i] <= R[j]){
            A[k] = L[i];
            i += 1;

        }
        else{
            A[k] = R[j];
            j += 1;
        }
        mergeCopies++;
        k += 1;
    }

    while(i < nl){
        mergeCopies++;
        A[k] = L[i];
        i += 1;
        k += 1;
    }

    while(j < nr){
        mergeCopies++;
        A[k] = R[j];
        j += 1;
        k += 1;
    }


}

void merge_sort(vector<int>&A, int p, int r)
{
    if(p >= r) return;

    int q = (p + r) / 2;
    merge_sort(A, p, q);
    merge_sort(A, q + 1, r);

    merge(A, p, q,r);

}


int main() {

    srand(time(0));

    int n; // number of elements to be sorted
    cin >> n;
    vector <int> A(n + 1); // we will store and sort number from 1 to n ;
    vector <int> B(n + 1);

    for(int i = 1; i <= n; i++){
        A[i] = rand() % 1000;  // for taking numbers below 1000
    }

    B = A;

    cout << "Input numbers: " << endl;
    for(int i = 1; i <= n; i++) cout << A[i] <<" ";
    cout <<endl;

    insertion_sort(A, n);
    cout << "Insertion Sort: "<<endl;

    for(int i = 1; i <= n; i++) cout << A[i] <<" ";
    cout <<endl;

    
    cout << "Comparisons: " << insertComparisons << endl;
    cout << "Shifts: " << insertionShifts << endl;

    merge_sort(B, 1, n);
    cout << "Merge Sort:" <<endl;

    // for(int i = 1; i <= n; i++) cout << B[i] <<" ";
    // cout <<endl;
    
    
    cout << "Comparisons: " << mergeComparisons << endl;
    cout << "Shifts: " << mergeCopies << endl;



    return 0;
}