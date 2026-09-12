#include <stdio.h>
#include <stdlib.h>
#include <time.h>


// #define N 5   // Size of the square matrices



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
		
		for(int i = 1; i <N; i++){
			int key = A[i];
			int j = i - 1;
			while( j >= 0 && A[j] > key){
				A[j + 1] = A[j];
				j -= 1;
			}
			A[j + 1] = key;
		}

        clock_gettime(CLOCK_MONOTONIC, &end);

        double elapsed =
            (end.tv_sec - start.tv_sec) +
            (end.tv_nsec - start.tv_nsec) / 1e9;
			
		// printf("Array values after sorting: \n");
	
		


        total_time += elapsed;
    }
	
	for(int i = 0; i < N; i++){
		printf("%d ", A[i]);
	}
	printf("\n");
	
    double avg_elapsed_time = total_time / RUNS;
	
	
	#ifdef PRINT 

	FILE *fp = fopen("isort_elapsed_time.txt", "w");
			
	if(fp == NULL){
				
		printf("Error opening file.\n");
		return 1;
	}
			
	fprintf(fp, "%.9lf\n", avg_elapsed_time);
			
	fclose(fp);
	#endif
	


    return 0;
}