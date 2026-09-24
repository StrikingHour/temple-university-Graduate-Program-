#include<bits/stdc++.h>

using namespace std;


// Binary Search
// Cost = number of middle elements examined
int binary_search_cost(vector<int>& A, int x)
{
    int low = 0;
    int high = A.size() - 1;
    int cost = 0;

    while(low <= high)
    {
        int mid = (low + high) / 2;

        cost++;    // examine A[mid]

        if(A[mid] == x)
        {
            return cost;
        }
        else if(x > A[mid])
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }

    return cost;
}


// Generate a sorted array with n distinct random integers
vector<int> generate_array(int n, int maxValue)
{
    set<int> numbers;

    while(numbers.size() < n)
    {
        numbers.insert(rand() % maxValue + 1);
    }

    vector<int> A(numbers.begin(), numbers.end());

    return A;
}


// Generate a random value that is not in the array
int generate_absent_target(vector<int>& A, int maxValue)
{
    int x;

    while(true)
    {
        x = rand() % maxValue + 1;

        if(!binary_search(A.begin(), A.end(), x))
        {
            return x;
        }
    }
}


int main()
{
    srand(time(0));

    int sizes[] = {100, 500, 1000, 1500, 3000, 6000};

    // Multiple random arrays for every input size
    int arraysPerSize = 20;

    // Multiple absent targets for every random array
    int absentTrials = 100;

    cout << fixed << setprecision(4);

    cout << "n"
         << "\tavg_success"
         << "\tavg_not_found"
         << "\tSituation_1"
         << "\tSituation_2"
         << "\tlg(n)"
         << endl;


    for(int s = 0; s < 6; s++)
    {
        int n = sizes[s];

        int maxValue = 20 * n + 100;

        double totalAvgSuccess = 0;
        double totalAvgNotFound = 0;
        double totalSituation1 = 0;
        double totalSituation2 = 0;


        for(int trial = 0; trial < arraysPerSize; trial++)
        {
            vector<int> A = generate_array(n, maxValue);


            // --------------------------------------
            // Average cost of successful searches
            // --------------------------------------

            long long successCostSum = 0;

            for(int i = 0; i < n; i++)
            {
                successCostSum +=
                    binary_search_cost(A, A[i]);
            }

            double avgSuccess =
                (double) successCostSum / n;



            // --------------------------------------
            // Average cost of unsuccessful searches
            // --------------------------------------

            long long notFoundCostSum = 0;

            for(int i = 0; i < absentTrials; i++)
            {
                int x =
                    generate_absent_target(A, maxValue);

                notFoundCostSum +=
                    binary_search_cost(A, x);
            }

            double avgNotFound =
                (double) notFoundCostSum / absentTrials;



            // --------------------------------------
            // Situation 1
            //
            // n successful outcomes + 1 not-found
            // outcome, all equally probable.
            // --------------------------------------

            double situation1 =
                (
                    successCostSum + avgNotFound
                )
                /
                (n + 1.0);



            // --------------------------------------
            // Situation 2
            //
            // P(success) = 1/2
            // P(not found) = 1/2
            // --------------------------------------

            double situation2 =
                0.5 * avgSuccess
                +
                0.5 * avgNotFound;



            totalAvgSuccess += avgSuccess;
            totalAvgNotFound += avgNotFound;
            totalSituation1 += situation1;
            totalSituation2 += situation2;
        }


        // Average over all random arrays

        double avgSuccess =
            totalAvgSuccess / arraysPerSize;

        double avgNotFound =
            totalAvgNotFound / arraysPerSize;

        double avgSituation1 =
            totalSituation1 / arraysPerSize;

        double avgSituation2 =
            totalSituation2 / arraysPerSize;


        cout << n
             << "\t" << avgSuccess
             << "\t\t" << avgNotFound
             << "\t\t" << avgSituation1
             << "\t\t" << avgSituation2
             << "\t\t" << log2(n)
             << endl;
    }

    return 0;
}