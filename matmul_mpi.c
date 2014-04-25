// Compile with: mpicc -o mm matmul_mpi.c

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// SIZE is a multiple of the number of nodes, 
// Hint: use small sizes when testing, e.g., SIZE 8
#define SIZE 1024
#define FROM_MASTER 1
#define FROM_WORKER 2
#define DEBUG 0	

#define MAX_PROCESSORS 2;

MPI_Status status;

static double a[SIZE][SIZE];
static double b[SIZE][SIZE];
static double c[SIZE][SIZE];

static double a1[SIZE / 2][SIZE];
static double a2[SIZE / 2][SIZE];

static double b1[SIZE][SIZE / 2];
static double b2[SIZE][SIZE / 2];

static double cHalf[SIZE / 2][SIZE];
static double cQuarter[SIZE / 2][SIZE / 2];

// Initialize a matrix of size (SIZE * SIZE).
// For simplicity, all values will be set to 1.0.
static void init_matrix(void)
{
	int x, y;
    for (x = 0; x < SIZE; x++)
	{
        for (y = 0; y < SIZE; y++) 
		{
			a[x][y] = 1.0;
			b[x][y] = 1.0;

			// a block-matrix
			if (x < SIZE / 2)
			{
				a1[x][y] = 1.0;	
			}
			else
			{
				a2[x - (SIZE / 2)][y] = 1.0;
			}

			// b block-matrix
			if (y < SIZE / 2)
			{
				b1[x][y] = 1.0;
			}
			else
			{
				b2[x][y - (SIZE / 2)] = 1.0;
			}
		}
	}
}

// Function used to print the contents of matrix c.
static void print_matrix(void)
{
	int x, y;
    for (x = 0; x < SIZE; x++) 
	{
        for (y = 0; y < SIZE; y++)
		{
			printf(" %7.2f", c[x][y]);
		}

		printf("\n");
    }
}

int main(int argc, char **argv)
{
    int myrank, availableProcs, nproc;
    int rows; /* amount of work per node (rows per worker) */
    int mtype; /* message type: send/recv between master and workers */
    int dest, src, offset;
    double start_time, end_time;
    int i, j, k;
	int HALF_SIZE = SIZE / 2;

    MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &availableProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	int max_proc = MAX_PROCESSORS;
	if (availableProcs > max_proc)
	{
		nproc = max_proc;
	}
	else
	{
		nproc = availableProcs;
	}

	// Masters tasks.
    if (myrank == 0) 
	{
		// Initialization.
		printf("SIZE = %d, number of nodes = %d\n", SIZE, availableProcs);
		printf("%d node(s) will be used.\n", nproc);

		init_matrix();
		start_time = MPI_Wtime();
		
		if (nproc == 4)
		{
			// One block on master and one on each node


			// Send matrix blocks 2 to node 2.
			MPI_Send(&a1, HALF_SIZE * SIZE, MPI_DOUBLE, 1, FROM_MASTER, MPI_COMM_WORLD);
			MPI_Send(&b2, HALF_SIZE * SIZE, MPI_DOUBLE, 1, FROM_MASTER, MPI_COMM_WORLD);

			// Send matrix blocks 3 to node 3.
			MPI_Send(&a2, HALF_SIZE * SIZE, MPI_DOUBLE, 2, FROM_MASTER, MPI_COMM_WORLD);
			MPI_Send(&b1, HALF_SIZE * SIZE, MPI_DOUBLE, 2, FROM_MASTER, MPI_COMM_WORLD);

			// Send matrix blocks 4 to node 4.
			MPI_Send(&a2, HALF_SIZE * SIZE, MPI_DOUBLE, 2, FROM_MASTER, MPI_COMM_WORLD);
			MPI_Send(&b2, HALF_SIZE * SIZE, MPI_DOUBLE, 2, FROM_MASTER, MPI_COMM_WORLD);


			// Block 1 (1,1)
			for (i = 0; i < HALF_SIZE; i++) // Row
			{
				for (j = 0; j < HALF_SIZE / 2; j++) // Element in row
				{
					c[i][j] = 0.0f;
					for (k = 0; k < HALF_SIZE / 2; k++)
					{
						c[i][j] = c[i][j] + a1[i][k] * b1[k][j];
					}
				}
			}

			// Receive results from node 2.
			MPI_Recv(&cQuarter, HALF_SIZE * SIZE, MPI_DOUBLE, 1, FROM_WORKER, MPI_COMM_WORLD, &status);
			for (i = 0; i < HALF_SIZE; i++)
			{
				for (j = 0; j < HALF_SIZE; j++)
				{
					c[i][j + HALF_SIZE] = cQuarter[i][j];
				}
			}

			// Receive results from node 3.
			MPI_Recv(&cQuarter, HALF_SIZE * SIZE, MPI_DOUBLE, 2, FROM_WORKER, MPI_COMM_WORLD, &status);
			for (i = 0; i < HALF_SIZE; i++)
			{
				for (j = 0; j < HALF_SIZE; j++)
				{
					c[i + HALF_SIZE][j] = cQuarter[i][j];
				}
			}

			// Receive results from node 4.
			MPI_Recv(&cQuarter, HALF_SIZE * SIZE, MPI_DOUBLE, 3, FROM_WORKER, MPI_COMM_WORLD, &status);
			for (i = 0; i < HALF_SIZE; i++)
			{
				for (j = 0; j < HALF_SIZE; j++)
				{
					c[i + HALF_SIZE][j + HALF_SIZE] = cQuarter[i][j];
				}
			}
		}

		else if (nproc == 2)
		{

			// Send matrix blocks to the node
			MPI_Send(&a2, HALF_SIZE * SIZE, MPI_DOUBLE, 1, FROM_MASTER, MPI_COMM_WORLD);
			MPI_Send(&b1, HALF_SIZE * SIZE, MPI_DOUBLE, 1, FROM_MASTER, MPI_COMM_WORLD);
			MPI_Send(&b2, HALF_SIZE * SIZE, MPI_DOUBLE, 1, FROM_MASTER, MPI_COMM_WORLD);

			// Two blocks on master, two blocks on node

			// Block 1 (1,1)
			for (i = 0; i < HALF_SIZE; i++) // Row
			{
				for (j = 0; j < HALF_SIZE / 2; j++) // Element in row
				{
					c[i][j] = 0.0f;
					for (k = 0; k < HALF_SIZE / 2; k++)
					{
						c[i][j] = c[i][j] + a1[i][k] * b1[k][j];
					}
				}
			}

			// Block 2 (1,2)
			for (i = 0; i < HALF_SIZE / 2; i++)
			{
				for (j = 0; j < HALF_SIZE / 2; j++)
				{
					c[i][j + HALF_SIZE] = 0.0f;
					for (k = 0; k < HALF_SIZE / 2; k++)
					{
						c[i][j + HALF_SIZE] = c[i][j + HALF_SIZE] + a1[i][k] * b2[k][j];
					}
				}
			}

			// Receive result
			MPI_Recv(&cHalf, HALF_SIZE * SIZE, MPI_DOUBLE, 1, FROM_WORKER, MPI_COMM_WORLD, &status);
			
			for (i = 0; i < HALF_SIZE; i++)
			{
				for (j = 0; j < SIZE; j++)
				{
					c[i + HALF_SIZE][j] = cHalf[i][j];
				}
			}
		}

		else
		{
			// All blocks on master
			// Could be done as one block, but separating blocks to enable abstraction to nproc 2 and 4

			// Block 1 (1,1)
			for (i = 0; i < HALF_SIZE; i++) // Row
			{
				for (j = 0; j < HALF_SIZE / 2; j++) // Element in row
				{
					c[i][j] = 0.0f;
					for (k = 0; k < HALF_SIZE / 2; k++)
					{
						c[i][j] = c[i][j] + a1[i][k] * b1[k][j];
					}
				}
			}

			// Block 2 (1,2)
			for (i = 0; i < HALF_SIZE / 2; i++)
			{
				for (j = 0; j < HALF_SIZE / 2; j++)
				{
					c[i][j + HALF_SIZE] = 0.0f;
					for (k = 0; k < HALF_SIZE / 2; k++)
					{
						c[i][j + HALF_SIZE] = c[i][j + HALF_SIZE] + a1[i][k] * b2[k][j];
					}
				}
			}
			
			// Block 3 (2,1)
			for (i = 0; i < HALF_SIZE / 2; i++)
			{
				for (j = 0; j < HALF_SIZE / 2; j++)
				{
					c[i + HALF_SIZE][j] = 0.0f;
					for (k = 0; k < HALF_SIZE / 2; k++)
					{
						c[i + HALF_SIZE][j] = c[i + HALF_SIZE][j] + a2[i][k] * b1[k][j];
					}
				}
			}	
			
			// Block 4 (2,2)
			for (i = 0; i < HALF_SIZE / 2; i++)
			{
				for (j = 0; j < HALF_SIZE / 2; j++)
				{
					c[i + HALF_SIZE][j + HALF_SIZE] = 0.0f;
					for (k = 0; k < HALF_SIZE / 2; k++)
					{
						c[i + HALF_SIZE][j + HALF_SIZE] = c[i + HALF_SIZE][j + HALF_SIZE] + a2[i][k] * b2[k][j];
					}
				}
			}
		}

		end_time = MPI_Wtime();

		if (DEBUG)
		{
			print_matrix();
		}

		double time_taken = (end_time - start_time);
		printf("Execution time on %2d nodes: %f\n", nproc, time_taken);
    } 
	
	// Worker tasks.
	else if(myrank < nproc)
	{
		if (nproc == 2)
		{
			MPI_Recv(&a2, HALF_SIZE * SIZE, MPI_DOUBLE, 0, FROM_MASTER, MPI_COMM_WORLD, &status);
			MPI_Recv(&b1, HALF_SIZE * SIZE, MPI_DOUBLE, 0, FROM_MASTER, MPI_COMM_WORLD, &status);
			MPI_Recv(&b2, HALF_SIZE * SIZE, MPI_DOUBLE, 0, FROM_MASTER, MPI_COMM_WORLD, &status);

			// Block 3 (2,1)
			for (i = 0; i < HALF_SIZE / 2; i++)
			{
				for (j = 0; j < HALF_SIZE / 2; j++)
				{
					c[i][j] = 0.0f;
					for (k = 0; k < HALF_SIZE / 2; k++)
					{
						c[i][j] = c[i][j] + a2[i][k] * b1[k][j];
					}
				}
			}

			// Block 4 (2,2)
			for (i = 0; i < HALF_SIZE / 2; i++)
			{
				for (j = 0; j < HALF_SIZE / 2; j++)
				{
					c[i][j + HALF_SIZE] = 0.0f;
					for (k = 0; k < HALF_SIZE / 2; k++)
					{
						c[i][j + HALF_SIZE] = c[i][j + HALF_SIZE] + a2[i][k] * b2[k][j];
					}
				}
			}

			MPI_Send(&c, HALF_SIZE * SIZE, MPI_DOUBLE, 0, FROM_WORKER, MPI_COMM_WORLD); // Send result back

		}
		else if (nproc == 4)
		{
			// NODE 2.
			if (myrank == 1)
			{
				MPI_Recv(&a1, HALF_SIZE * SIZE, MPI_DOUBLE, 0, FROM_MASTER, MPI_COMM_WORLD, &status);
				MPI_Recv(&b2, HALF_SIZE * SIZE, MPI_DOUBLE, 0, FROM_MASTER, MPI_COMM_WORLD, &status);

				// Block 2 (1,2)
				for (i = 0; i < HALF_SIZE / 2; i++)
				{
					for (j = 0; j < HALF_SIZE / 2; j++)
					{
						cQuarter[i][j] = 0.0f;
						for (k = 0; k < HALF_SIZE / 2; k++)
						{
							cQuarter[i][j] = cQuarter[i][j] + a1[i][k] * b2[k][j];
						}
					}
				}

				// Send node 2 results back.
				MPI_Send(&cQuarter, HALF_SIZE * HALF_SIZE, MPI_DOUBLE, 0, FROM_WORKER, MPI_COMM_WORLD);
			}

			// NODE 3.
			else if (myrank == 2)
			{
				MPI_Recv(&a2, HALF_SIZE * SIZE, MPI_DOUBLE, 0, FROM_MASTER, MPI_COMM_WORLD, &status);
				MPI_Recv(&b1, HALF_SIZE * SIZE, MPI_DOUBLE, 0, FROM_MASTER, MPI_COMM_WORLD, &status);

				// Block 3 (2,1)
				for (i = 0; i < HALF_SIZE / 2; i++)
				{
					for (j = 0; j < HALF_SIZE / 2; j++)
					{
						cQuarter[i][j] = 0.0f;
						for (k = 0; k < HALF_SIZE / 2; k++)
						{
							cQuarter[i][j] = cQuarter[i][j] + a2[i][k] * b1[k][j];
						}
					}
				}

				// Send node 3 results back.
				MPI_Send(&cQuarter, HALF_SIZE * HALF_SIZE, MPI_DOUBLE, 0, FROM_WORKER, MPI_COMM_WORLD);
			}

			// NODE 4.
			else if (myrank == 3)
			{
				MPI_Recv(&a2, HALF_SIZE * SIZE, MPI_DOUBLE, 0, FROM_MASTER, MPI_COMM_WORLD, &status);
				MPI_Recv(&b2, HALF_SIZE * SIZE, MPI_DOUBLE, 0, FROM_MASTER, MPI_COMM_WORLD, &status);

				// Block 4 (2,2)
				for (i = 0; i < HALF_SIZE / 2; i++)
				{
					for (j = 0; j < HALF_SIZE / 2; j++)
					{
						cQuarter[i][j] = 0.0f;
						for (k = 0; k < HALF_SIZE / 2; k++)
						{
							cQuarter[i][j] = cQuarter[i][j] + a2[i][k] * b2[k][j];
						}
					}
				}

				// Send node 4 results back.
				MPI_Send(&cQuarter, HALF_SIZE * HALF_SIZE, MPI_DOUBLE, 0, FROM_WORKER, MPI_COMM_WORLD); 
			}
		}
    }

    MPI_Finalize();
    return 0;
}
