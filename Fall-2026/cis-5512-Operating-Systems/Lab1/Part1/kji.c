#include <stdio.h>
#include <stdlib.h>
#include <time.h>


// #define N 5   // Size of the square matrices



int main()
{
	struct timespec start, end;


	
    static int A[N][N];
    static int B[N][N];
    static int C[N][N];
	
	double total_time = 0.0;

    srand(1);

    // Generate random values for matrix A and B
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            A[i][j] = rand() % 10;
            B[i][j] = rand() % 10;
            C[i][j] = 0;
        }
    }


 

    // Print result matrix C
    // printf("\nMatrix C = A x B: Order (k j i)\n");
	
	
	for (int run = 0; run < RUNS; run++)
    {
        // Reset result matrix before each run
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                C[i][j] = 0;
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &start);

        // Matrix multiplication
        for (int k = 0; k < N; k++)
        {
            for (int j = 0; j < N; j++)
            {
                for (int i = 0; i < N; i++)
                {
                    C[i][j] += A[i][k] * B[k][j];
                }
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &end);

        double elapsed =
            (end.tv_sec - start.tv_sec) +
            (end.tv_nsec - start.tv_nsec) / 1e9;


        total_time += elapsed;
    }
	
	
	#ifdef PRINT 

		FILE *fp = fopen("kji.txt", "w");
		
		if(fp == NULL){
			
			printf("Error opening file.\n");
			return 1;
		}
		
		for(int i = 0; i < N; i++){
			for(int j = 0; j < N; j++){
				fprintf(fp, "%d", C[i][j]);
			}
			fprintf(fp, "\n");
		}
		
		fclose(fp);
	#endif
    double avg_elapsed_time = total_time / RUNS;
	double performance = ((double)N * (double)N * (double)N) / avg_elapsed_time;
	
	printf("%.9f %.9f\n", avg_elapsed_time, performance);
	
	

	


    return 0;
}