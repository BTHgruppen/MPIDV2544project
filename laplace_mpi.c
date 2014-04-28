/* TODO
2. Divide into blocks for 2 and 4 processors
3. Send blocks to other nodes
4. Each node calculates their block, row by row
5. Exchange row values with adjacent blocks (Note: Use Sendreceive to avoid deadlock!)
6. Check acceptance value, if we have not passed it yet, goto 4
7. If we pass the acceptance value, add everything together into one matrix
*/

#include <stdio.h>
#include <stdlib.h> 
#include <mpi.h>

#define SIZE 1024;
#define SIZEWITHBORDERS (SIZE + 2);
#define DIFFERANCELIMIT (0.00001 * SIZE);

#define FILLTYPE "Random";
#define MAXRANDOM 15;

#define MAX_PROCESSORS 4;
#define DEBUG 0;

static double A[SIZEWITHBORDERS][SIZEWITHBORDERS];

MPI_Status status;

void InitializeMatrix();
void PrintMatrix();

int main(int argc, char **argv)
{
	// Generate the matrix.
	InitializeMatrix();

	// Initialize MPI API.
    MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &processorRank);
	MPI_Comm_size(MPI_COMM_WORLD, &processorsAvailable);

	// Ask for number of processors to be used.
	int processorsUsed = 1;
	printf("How many processors should be used in the cluster? [1, 2, 4]: ");
	scanf("%d", &processorsUsed);

	// Check input, if wrong, terminate program.
	if(processorsUsed != 1 && processorsUsed != 2 && processorsUsed != 4)
	{
		cout >> "Wrong input used! Try again...";
		return 0;
	}
    
	// Master code.
	if(processorRank == 0)
	{
		// 1 processor used, i.e. the master only.
		if(processorsUsed == 1)
		{

		}

		// 2 processors used, the master and one worker.
		else if(processorsUsed == 2)
		{

		}

		// 4 processors used, the master and all three waorkers.
		else if(processorsUsed== 4)
		{

		}
	}

	// Worker code.
	else if(processorRank < processorsUsed)
	{
		// 2 processors used, the master and one worker.
		if(processorsUsed == 2)
		{
			// Worker 2 part.
			if(processorRank == 1)
			{

			}
		}

		// 4 processors used, the master and all three workers.
		else if(processorsUsed == 4)
		{
			// Worker 2 part.
			if(processorRank == 1)
			{

			}

			// Worker 3 part.
			else if(processorRank == 2)
			{

			}

			// Worker 4 part.
			else if(processorRank == 3)
			{

			}
		}
	}

	// Finalize the MPI API, and then quit.
	MPI_Finalize();
    return 0;
}

void InitializeMatrix()
{
	// If we are in debug mode, print matrix definitions.
	if(DEBUG)
	{
		printf("Number of processors to be used: %d\n", MAX_PROCESSORS);

		printf("Matrix size (without borders): %d x %d\n", SIZE, SIZE);
		printf("Matrix size (with borders): %d x %d\n", SIZEWITHBORDERS, SIZEWITHBORDERS);
		printf("Differance limit: %.7lf\n", DIFFERANCELIMIT);

		printf("Method for filling the matrix: %s\n", FILLTYPE);
		if(FILLTYPE == "Random")
		{
			printf("Maximum random value: %f\n", MAXRANDOM);
		}

		printf("Initializing matrix...");
	}
 
    // Initialize all matrix elements to 0.0.
	int i, j;
    for (i = 0; i < SIZEWITHBORDERS; i++) 
	{
		for (j = 0; j < SIZEWITHBORDERS; j++) 
		{
			A[i][j] = 0.0;
		}
    }

	// Fill the matrix with incrementing elements.
    if (FILLTYPE == "Counting") 
	{
		for (i = 1; i < SIZE + 1; i++)
		{
			for (j = 1; j < SIZE + 1; j++) 
			{
				A[i][j] = (double)(i / 2);
			}
		}
    }

	// Fill the matrix quickly.
	int temp = 0;
    if (FILLTYPE == "Quickly") 
	{
		for (i = 1; i < SIZE + 1; i++)
		{
			temp++;

			for (j = 1; j < SIZE + 1; j++) 
			{
				temp++;

				// Even elements.
				if ((temp % 2) == 0)
				{
					A[i][j] = 1.0;
				}

				// Odd elements.
				else
				{
					A[i][j] = 5.0;
				}
			}
		}
    }

	// Fill the matrix with random elements
    if (FILLTYPE == "Random") 
	{
		for (i = 1; i < SIZE + 1; i++)
		{
			for (j = 1; j < SIZE + 1; j++) 
			{
				A[i][j] = (rand() % MAXRANDOM) + 1.0;
			}
		}
    }

	// Edit the matrix corners.
    A[0][0]			  = A[1][1];
    A[0][SIZE+1]	  = A[1][SIZE];
    A[SIZE+1][0]	  = A[SIZE][1];
    A[SIZE+1][SIZE+1] = A[SIZE][SIZE];

    // Edit the top and bottom rows.
    for (i = 1; i < SIZE + 1; i++) 
	{
		A[0][i]		 = A[1][i];
		A[SIZE+1][i] = A[SIZE][i];
    }

    // Edit the left and right columns.
    for (i = 1; i < SIZE + 1; i++) 
	{
		A[i][0]		 = A[i][1];
		A[i][SIZE+1] = A[i][SIZE];
    }

	// If we are in debug mode, pring matrix.
	if(DEBUG)
	{
		 printf("Initialization done! \nPrinting matrix: \n\n");
		 PrintMatrix();
	}
}

void PrintMatrix()
{
	int i, j;

	// Iterate through the matrix and print it.
    for (i = 0; i < SIZEWITHBORDERS ;i++)
	{
		for (j = 0; j < SIZEWITHBORDERS ;j++)
		{
			printf(" %f", A[i][j]);
		}
		
		printf("\n");
	}

    printf("\n\n");
}