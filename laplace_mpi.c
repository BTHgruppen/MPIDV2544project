
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

#define SIZE 8
#define SIZEWITHBORDERS (SIZE + 2)
#define DIFFERANCELIMIT (0.00001 * SIZE)

#define FILLTYPE "Random"
#define MAXRANDOM 15

#define MAX_PROCESSORS 4
#define DEBUG 1
#define EVEN 0
#define ODD 1

static double A[SIZEWITHBORDERS][SIZEWITHBORDERS];
int processorRank;

MPI_Status status;

void InitializeMatrix();
void PrintMatrix();
int SequentialApproximation();

int main(int argc, char **argv)
{
	int iterations = 0;

	double startTime = 0; 
	double endTime	 = 0;
	double totalTime = 0;

	int processorsAvailable;

	// Generate the matrix.
	InitializeMatrix();

	// Initialize MPI API.
    MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &processorRank);
	MPI_Comm_size(MPI_COMM_WORLD, &processorsAvailable);

	// Ask for number of processors to be used. Loop if input is invalid.
	int processorsUsed = 1;
	while (processorsUsed != 1 && processorsUsed != 2 && processorsUsed != 4)
	{
		fflush(stdin);
		printf("How many processors should be used in the cluster? [1, 2, 4]: ");
		scanf("%d", &processorsUsed);
	}
	
	//==================================================//
	//          V V V MASTERS CODE BLOCK V V V			//
	//==================================================//
	if(processorRank == 0)
	{
		if(DEBUG)
		{
			printf("%d processors will be used.\n", processorsUsed);
			printf("\n>> Running LaPlace approximation...\n\n");
		}
		  
		// Start the timer.
		startTime = MPI_Wtime();

		// 1 processor used, the master does all the work (SEQUENTIAL).
		if(processorsUsed == 1)
		{
			// Start the timer.
			startTime = MPI_Wtime();

			iterations = SequentialApproximation();

			// If we reached to many iterations, quit.
			if(iterations > 100000)
			{
				return 0;
			}
		}

		// 2 processors used, the master and one worker.
		else if(processorsUsed == 2)
		{
			iterations = MasterBlockedApproximation(2);
		}

		// 4 processors used, the master and all three workers.
		else if(processorsUsed == 4)
		{
			iterations = MasterBlockedApproximation(4);
		}

		// Stop the timer.
		endTime = MPI_Wtime();

		printf("[SUCCESS] LaPlace approximation finished.\n");

		if(DEBUG)
		{
			printf("\n>> Printing matrix... \n\n");
			PrintMatrix();
		}

		// Output time taken.
		totalTime = (endTime - startTime);
		printf("Execution time on %2d nodes: %f\n", processorsUsed, totalTime);
	}
	//==================================================//
	//          ^ ^ ^ MASTERS CODE BLOCK ^ ^ ^ 			//
	//==================================================//
	
	//==================================================//
	//          V V V WORKER CODE BLOCK V V V			//
	//==================================================//
	else if(processorRank < processorsUsed)
	{
		// 2 processors used, the master and one worker.
		if(processorsUsed == 2)
		{
			// Worker 2 part.
			if(processorRank == 1)
			{
				WorkerBlockedApproximation();
			}
		}

		// 4 processors used, the master and all three workers.
		else if(processorsUsed == 4)
		{
			// Worker 2 part.
			if(processorRank == 1)
			{
				WorkerBlockedApproximation();
			}

			// Worker 3 part.
			else if(processorRank == 2)
			{
				WorkerBlockedApproximation();
			}

			// Worker 4 part.
			else if(processorRank == 3)
			{
				WorkerBlockedApproximation();
			}
		}
	}
	//==================================================//
	//          ^ ^ ^  WORKER CODE BLOCK ^ ^ ^ 			//
	//==================================================//

	// Finalize the MPI API, and then quit.
	MPI_Finalize();
    return 0;
}

void InitializeMatrix()
{
	// If we are in debug mode, print matrix definitions.
	if(DEBUG)
	{
		printf("Matrix size (without borders): %d x %d\n", SIZE, SIZE);
		printf("Matrix size (with borders): %d x %d\n", SIZEWITHBORDERS, SIZEWITHBORDERS);
		printf("Differance limit: %.7lf\n", DIFFERANCELIMIT);

		printf("Method for filling the matrix: %s\n", FILLTYPE);
		if(FILLTYPE == "Random")
		{
			printf("Maximum random value: %f\n", MAXRANDOM);
		}

		printf("\n>> Initializing matrix...\n\n");
	}
 
    // Initialize all matrix elements to 0.0.
	int i;
	int j;

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
	int count = 0;
    if (FILLTYPE == "Quickly") 
	{
		for (i = 1; i < SIZE + 1; i++)
		{
			count++;

			for (j = 1; j < SIZE + 1; j++) 
			{
				count++;

				// Even elements.
				if ((count % 2) == 0)
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
		printf("Initialization done!\n");
		printf("\n>> Printing matrix... \n\n");
		PrintMatrix();
	}
}

void PrintMatrix()
{
	int i;
	int j;

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

int SequentialApproximation()
{
	double previousMaximum_EVEN = 0.0;
	double previousMaximum_ODD = 0.0;
	double maximum = 0.0;
	double sum = 0.0;
	double w = 0.5;

    int	m, n, i;
    int turn = EVEN;
    int iteration = 0;
	int finished = 0;
    
	// Approximate until finished.
    while(!finished) 
	{
		iteration++;

		// Calculate even elements.
		if(turn == EVEN) 
		{
			for(m = 1; m < SIZE + 1; m++) 
			{
				for(n = 1; n < SIZE + 1; n++) 
				{
					if(((m + n) % 2) == 0)
					{
						// Perform average operation, using the elements 4 neighbours.
						A[m][n] = (1 - w) * A[m][n] + w * (A[m-1][n] + A[m+1][n] + A[m][n-1] + A[m][n+1]) / 4;
					}
				}
			}

			// Calculate the maximum sum of the elements.
			maximum = -999999.0;
			for(m = 1; m < SIZE + 1; m++) 
			{
				sum = 0.0;

				for(n = 1; n < SIZE + 1; n++)
				{
					sum += A[m][n];
				}

				if(sum > maximum)
				{
					maximum = sum;
				}
			}

			// Check wether the approximation is finished or not, by comparing the even sum with the previous sum.
			if(fabs(maximum - previousMaximum_EVEN) <= DIFFERANCELIMIT)
			{
				finished = 1;
			}

			// Print debug information if flaged.
			if(DEBUG && (iteration % 100) == 0)
			{
				printf("Iteration: %d, maximum: %f, previous (even) maximum: %f\n", iteration, maximum, previousMaximum_EVEN);
			}

			// Prepare for next iteration.
			previousMaximum_EVEN = maximum;
			turn = ODD;
		} 
	
		// Calculate odd elements.
		else if(turn == ODD) 
		{
			for(m = 1; m < SIZE + 1; m++) 
			{
				for(n = 1; n < SIZE + 1; n++) 
				{
					if(((m + n) % 2) == 1)
					{
						// Perform average operation, using the elements 4 neighbours.
						A[m][n] = (1 - w) * A[m][n] + w * (A[m-1][n] + A[m+1][n] + A[m][n-1] + A[m][n+1]) / 4;
					}
				}
			}

			// Calculate the maximum sum of the elements.
			maximum = -999999.0;
			for(m = 1; m < SIZE + 1; m++) 
			{
				sum = 0.0;

				for(n = 1; n < SIZE + 1; n++)
				{
					sum += A[m][n];	
				}

				if(sum > maximum)	
				{
					maximum = sum;
				}
			}

			// Check wether the approximation is finished or not, by comparing the odd sum with the previous sum.
			if(fabs(maximum - previousMaximum_ODD) <= DIFFERANCELIMIT)
			{
				finished = 1;
			}

			// Print debug information if flaged.
			if(DEBUG && (iteration%100) == 0)
			{
				printf("Iteration: %d, maximum: %f, previous (odd) maximum: %f\n", iteration, maximum, previousMaximum_ODD);
			}

			// Prepare for next iteration.
			previousMaximum_ODD = maximum;
			turn = EVEN;
		} 

		// Exit if the approximation does not converge fast enough.
		if(iteration > 100000) 
		{
			printf("[FAILURE] Maximum number of iterations reached before convergance.\n");
			printf("Change parameters and try again...\n");
			finished = 1;
		}
    }

    return iteration;
}

int MasterBlockedApproximation(int nodes)
{
	if (nodes == 2)
	{
		// Dont know if this needs to be done?
	}

	if (nodes == 4)
	{
		// Create 4 blocks and fill them
		// The blocks will be one quarter of the original block with an extra border in x and y for data exchange 
		int blocksize = (SIZEWITHBORDERS / 2) + 1;
		int blocksizeSquared = blocksize * blocksize;
		int* myBlock;
		int i, j;

		// Block (1,1) for node 1
		myBlock = malloc(sizeof(int)* blocksizeSquared);
		for (i = 0; i < blocksize; i++)
		{
			for (j = 0; j < blocksize; j++)
			{
				myBlock[i * blocksize + j] = A[i][j];
			}
		}
		MPI_Send(block12, blocksizeSquared, MPI_INT, 1, 0, MPI_COMM_WORLD);

		// Block (1,2) for node 2
		for (i = 0; i < blocksize; i++)
		{
			// No need to realloc memory since same size is used
			for (j = 0; j < blocksize; j++)
			{
				myBlock[i * blocksize + j] = A[i][j + blocksize - 1];
			}
		}
		MPI_Send(block12, blocksizeSquared, MPI_INT, 2, 0, MPI_COMM_WORLD);

		// Block (2,1) for node 3
		for (i = 0; i < blocksize; i++)
		{
			for (j = 0; j < blocksize; j++)
			{
				myBlock[i * blocksize + j] = A[i + blocksize - 1][j];
			}
		}
		MPI_Send(block12, blocksizeSquared, MPI_INT, 3, 0, MPI_COMM_WORLD);

		// Block (2,2) for self/master
		for (i = 0; i < blocksize; i++)
		{
			for (j = 0; j < blocksize; j++)
			{
				myBlock[i * blocksize + j] = A[i + blocksize - 1][j + blocksize - 1];
			}
		}

		// Start the timer.
		startTime = MPI_Wtime();

		// Start calculating.

	}
}

void WorkerBlockedApproximation()
{
	if (processorRank == 0)
	{
		printf("[ERROR] WorkerBlockedApproximation called on master node.");
		exit(1);
	}

	int** myBlock;

	// Work for the 1 remaining node.
	if(processorsUsed == 2)
	{
		int blockSizeX = (SIZEWITHBORDERS / 2) + 1;
		int blockSizeY = SIZEWITHBORDERS + 1;

		// Receive initial block
		MPI_Recv(&myBlock, blockSizeX * blockSizeY, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

		// Perform LaPlace approximation.
		//LaplaceOverBlock(myBlock, blockSizeX, blockSizeY); DOES NOT WORK, LaplaceOverBlock() written for rectangular blocks.
	}

	// Work for 3 nodes.
	else if(processorsUsed == 4)
	{
		int blockSize = (SIZEWITHBORDERS / 2) + 1;

		// Receive initial block
		MPI_Recv(&myBlock, blockSize * blockSize, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

		// Perform LaPlace approximation.
		LaplaceOverBlock(myBlock, blockSize);
	}
}

void LaplaceOverBlock(int* block, int blocksize)
{
	double previousMaximum_EVEN = 0.0;
	double previousMaximum_ODD = 0.0;
	double maximum = 0.0;
	double sum = 0.0;
	double w = 0.5;

	int	m, n, i;
	int turn = EVEN;
	int iteration = 0;
	int finished = 0;

	// Approximate until finished.
	while (!finished)
	{
		iteration++;

		// Calculate even elements.
		if (turn == EVEN)
		{
			for (m = 1; m < blocksize - 1; m++)
			{
				for (n = 1; n < blocksize - 1; n++)
				{
					if (((m + n) % 2) == 0)
					{
						// Perform average operation, using the elements 4 neighbours.
						block[m * blocksize + n] = (1 - w) * block[m * blocksize + n] + w * (block[(m - 1) * blocksize + n] + block[(m + 1)* blocksize + n] + block[m * blocksize + n - 1] + block[m * blocksize + n + 1]) / 4;
					}
				}
			}

			// Calculate the maximum sum of the elements.
			maximum = -999999.0;
			for (m = 1; m < blocksize - 1; m++)
			{
				sum = 0.0;

				for (n = 1; n < blocksize - 1; n++)
				{
					sum += block[m * blocksize + n];
				}

				if (sum > maximum)
				{
					maximum = sum;
				}
			}

			// Check wether the approximation is finished or not, by comparing the even sum with the previous sum.
			if (fabs(maximum - previousMaximum_EVEN) <= DIFFERANCELIMIT)
			{
				finished = 1;
			}

			// Print debug information if flaged.
			if (DEBUG && (iteration % 100) == 0)
			{
				printf("Iteration: %d, maximum: %f, previous (even) maximum: %f\n", iteration, maximum, previousMaximum_EVEN);
			}

			// Prepare for next iteration.
			previousMaximum_EVEN = maximum;
			turn = ODD;
		}

		// Calculate odd elements.
		else if (turn == ODD)
		{
			for (m = 1; m < blocksize - 1; m++)
			{
				for (n = 1; n < blocksize - 1; n++)
				{
					if (((m + n) % 2) == 1)
					{
						// Perform average operation, using the elements 4 neighbours.
						block[m * blocksize + n] = (1 - w) * block[m * blocksize + n] + w * (block[(m - 1) * blocksize + n] + block[(m + 1) * blocksize + n] + block[m * blocksize + n - 1] + block[m * blocksize + n + 1]) / 4;
					}
				}
			}

			// Calculate the maximum sum of the elements.
			maximum = -999999.0;
			for (m = 1; m < blocksize - 1; m++)
			{
				sum = 0.0;

				for (n = 1; n < blocksize - 1; n++)
				{
					sum += block[m * blocksize + n];
				}

				if (sum > maximum)
				{
					maximum = sum;
				}
			}

			// Check wether the approximation is finished or not, by comparing the odd sum with the previous sum.
			if (fabs(maximum - previousMaximum_ODD) <= DIFFERANCELIMIT)
			{
				finished = 1;
			}

			// Print debug information if flaged.
			if (DEBUG && (iteration % 100) == 0)
			{
				printf("Iteration: %d, maximum: %f, previous (odd) maximum: %f\n", iteration, maximum, previousMaximum_ODD);
			}

			// Prepare for next iteration.
			previousMaximum_ODD = maximum;
			turn = EVEN;
		}

		// Exit if the approximation does not converge fast enough.
		if (iteration > 100000)
		{
			printf("[FAILURE] Maximum number of iterations reached before convergance.\n");
			printf("Change parameters and try again...\n");
			finished = 1;
		}

		// Sendreceive 
		switch (processorrank)
		{
			int* sendbuff;
			int* receivebuff;

		case 1: // (1,1)
			// Exchange with node 2 (1,2)
			// Copy the send buffer from the matrix block
			sendbuff = malloc(sizeof(int)* blocksize);
			for (i = 0; i < blocksize; i++)
			{
				sendbuff[i] = block[(i * blocksize) + blocksize - 2];
			}
			// Send and receive
			receivebuff = malloc(sizeof(int)* blocksize);
			MPI_Sendrecv(sendbuff, blocksize, MPI_INT, 2, 0, receivebuff, blocksize, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
			
			// Fill the received buffer into the matrix block
			for (i = 0; i < blocksize; i++)
			{
				block[(i * blocksize) + blocksize - 1] = receivebuff[i];
			}
			free(sendbuff);
			free(receivebuff);

			// Echange with node 3 (2,1)
			// Point the send buffer to the right row in the matrix block
			sendbuff = &(block[(blocksize - 2) * blocksize]);
			// Point the receive buffer to the right row in the matrix block
			receivebuff = &(block[(blocksize - 1) * blocksize]);
			// Send and receive
			MPI_Sendrecv(sendbuff, blocksize, MPI_INT, 3, 0, receivebuff, blocksize, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);

			// Set pointers to 0, just in case.
			sendbuff = 0;
			receivebuff = 0;
			break;
		case 2: // (1,2)
			// Exchange with node 1 (1,1)
			// Copy the send buffer from the matrix block
			sendbuff = malloc(sizeof(int)* blocksize);
			for (i = 0; i < blocksize; i++)
			{
				sendbuff[i] = block[(i * blocksize) + 1];
			}
			// Send and receive
			receivebuff = malloc(sizeof(int)* blocksize);
			MPI_Sendrecv(sendbuff, blocksize, MPI_INT, 1, 0, receivebuff, blocksize, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

			// Fill the received buffer into the matrix block
			for (i = 0; i < blocksize; i++)
			{
				block[(i * blocksize)] = receivebuff[i];
			}
			free(sendbuff);
			free(receivebuff);

			// Echange with node 0 (2,2)
			// Point the send buffer to the right row in the matrix block
			sendbuff = &(block[(blocksize - 2) * blocksize]);
			// Point the receive buffer to the right row in the matrix block
			receivebuff = &(block[(blocksize - 1) * blocksize]);
			// Send and receive
			MPI_Sendrecv(sendbuff, blocksize, MPI_INT, 0, 0, receivebuff, blocksize, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

			// Set pointers to 0, just in case.
			sendbuff = 0;
			receivebuff = 0;
			break;
		case 3: // (2,1)
			// Exchange with node 0 (2,2)
			// Copy the send buffer from the matrix block
			sendbuff = malloc(sizeof(int)* blocksize);
			for (i = 0; i < blocksize; i++)
			{
				sendbuff[i] = block[(i * blocksize) + blocksize - 2];
			}
			// Send and receive
			receivebuff = malloc(sizeof(int)* blocksize);
			MPI_Sendrecv(sendbuff, blocksize, MPI_INT, 0, 0, receivebuff, blocksize, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

			// Fill the received buffer into the matrix block
			for (i = 0; i < blocksize; i++)
			{
				block[(i * blocksize) + blocksize - 1] = receivebuff[i];
			}
			free(sendbuff);
			free(receivebuff);

			// Echange with node 1 (1,1)
			// Point the send buffer to the right row in the matrix block
			sendbuff = &(block[blocksize]);
			// Point the receive buffer to the right row in the matrix block
			receivebuff = &(block[0]);
			// Send and receive
			MPI_Sendrecv(sendbuff, blocksize, MPI_INT, 1, 0, receivebuff, blocksize, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

			// Set pointers to 0, just in case.
			sendbuff = 0;
			receivebuff = 0;
			break;
		case 0: // (2,2)
			// Exchange with node 3 (2,1)
			// Copy the send buffer from the matrix block
			sendbuff = malloc(sizeof(int)* blocksize);
			for (i = 0; i < blocksize; i++)
			{
				sendbuff[i] = block[(i * blocksize) + 1];
			}
			// Send and receive
			receivebuff = malloc(sizeof(int)* blocksize);
			MPI_Sendrecv(sendbuff, blocksize, MPI_INT, 3, 0, receivebuff, blocksize, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);

			// Fill the received buffer into the matrix block
			for (i = 0; i < blocksize; i++)
			{
				block[(i * blocksize)] = receivebuff[i];
			}
			free(sendbuff);
			free(receivebuff);

			// Echange with node 2 (1,2)
			// Point the send buffer to the right row in the matrix block
			sendbuff = &(block[blocksize]);
			// Point the receive buffer to the right row in the matrix block
			receivebuff = &(block[0]);
			// Send and receive
			MPI_Sendrecv(sendbuff, blocksize, MPI_INT, 2, 0, receivebuff, blocksize, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

			// Set pointers to 0, just in case.
			sendbuff = 0;
			receivebuff = 0;
			break;
		case default:
			break;
		}
	}

	// TODO:
	// When each block is done, it should be sent back to the master.

	return iteration;
}
