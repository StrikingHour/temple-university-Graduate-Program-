#include <stdio.h>
#include <stdlib.h>
#include <time.h>


// #define N 5   // Size of the square matrices


void mergeSortedHalves(int A[], int N)
{
    int mid = N / 2;

    int temp[N];

    int i = 0;      // start of first half
    int j = mid;    // start of second half
    int k = 0;      // index for temp array

    // Compare elements from both halves
    while (i < mid && j < N)
    {
        if (A[i] <= A[j])
        {
            temp[k] = A[i];
            i++;
        }
        else
        {
            temp[k] = A[j];
            j++;
        }

        k++;
    }

    // Copy remaining elements from first half
    while (i < mid)
    {
        temp[k] = A[i];
        i++;
        k++;
    }

    // Copy remaining elements from second half
    while (j < N)
    {
        temp[k] = A[j];
        j++;
        k++;
    }

    // Copy merged result back to A
    for (i = 0; i < N; i++)
    {
        A[i] = temp[i];
    }
}



int main()
{
	struct timespec start, end;
	
	int N = 5000;
	int RUNS = 5;
	
	int A[N];
	
	double total_time = 0.0;

    

   
	
	
	for (int run = 0; run < RUNS; run++)
    {
		
		srand(1);
		
		 // Generate random values for matrix A and B
		for (int i = 0; i < N; i++)
		{
			A[i] = rand() % 1000;    
		}
		
		// printing the unordered values of the array A
		
		// printf("Array values before sorting: \n");
		
		// for(int i = 0; i < N; i++){
			// printf("%d ", A[i]);
		// }
		// printf("\n");

        clock_gettime(CLOCK_MONOTONIC, &start);

        // insertion sort
		
		int mid = N / 2;
		
		for(int i = 1; i < mid; i++){
			int key = A[i];
			int j = i - 1;
			while( j >= 0 && A[j] > key){
				A[j + 1] = A[j];
				j -= 1;
			}
			A[j + 1] = key;
		}
		
		for(int i = mid + 1; i < N; i++){
			int key = A[i];
			int j = i - 1;
			while( j >= mid && A[j] > key){
				A[j + 1] = A[j];
				j -= 1;
			}
			A[j + 1] = key;
		}
		
		// merging two arrays 
		
		mergeSortedHalves(A,N);
		
		

        clock_gettime(CLOCK_MONOTONIC, &end);

        double elapsed =
            (end.tv_sec - start.tv_sec) +
            (end.tv_nsec - start.tv_nsec) / 1e9;
			
		// printf("Array values after sorting: \n");
	
		// for(int i = 0; i < N; i++){
			// printf("%d ", A[i]);
		// }
		// printf("\n");


        total_time += elapsed;
    }
	
	
	
    for(int i = 0; i < N; i++){
		printf("%d ", A[i]);
	}
	printf("\n");
	
    double avg_elapsed_time = total_time / RUNS;
	
	
	#ifdef PRINT 

	FILE *fp = fopen("isort2_elapsed_time.txt", "w");
			
	if(fp == NULL){
				
		printf("Error opening file.\n");
		return 1;
	}
			
	fprintf(fp, "%.9lf\n", avg_elapsed_time);
			
	fclose(fp);
	#endif
	


    return 0;
}