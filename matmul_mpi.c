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

#define MAX_PROCESSORS 1;

MPI_Status status;

static double a[SIZE][SIZE];
static double b[SIZE][SIZE];
static double c[SIZE][SIZE];


static double a1[SIZE / 2][SIZE];
static double a2[SIZE / 2][SIZE];

static double b1[SIZE][SIZE / 2];
static double b2[SIZE][SIZE / 2];

// Initialize a matrix of size (SIZE * SIZE).
// For simplicity, all values will be set to 1.0.
static void init_matrix(void)
{
    for (int x = 0; x < SIZE; x++)
	{
        for (int y = 0; y < SIZE; y++) 
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
				a2[x - SIZE / 2] = 1.0;
			}

			// b block-matrix
			if (y < SIZE / 2)
			{
				b1[x][y] = 1.0;
			}
			else
			{
				b2[x][y - SIZE / 2] = 1.0;
			}
		}
	}
}

// Function used to print the contents of matrix c.
static void print_matrix(void)
{
    for (int x = 0; x < SIZE; x++) 
	{
        for (int y = 0; y < SIZE; y++)
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

    MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &availableProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	if (availableProcs > MAX_PROCESSORS)
	{
		nproc = MAX_PROCESSORS;
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
		printf("%d node(s) will be used.", nproc);

		init_matrix();
		start_time = MPI_Wtime();
		
		if (nproc == 4)
		{
			// One block on master and one on each node
		}
		else if (nproc == 2)
		{
			// Two blocks on master, two blocks on node
		}
		else
		{
			// All blocks on master
			// Could be done as one block, but separating blocks to enable abstraction to nproc 2 and 4

			// Block 1
			for (i = 0; i < SIZE / 2; i++) // Row
			{
				for (j = 0; j < SIZE / 2; j++) // Element in row
				{
					c[i][j] = 0.0f;
					for (k = 0; k < SIZE / 2; k++)
					{
						c[i][j] = c[i][j] + a1[i][k] * b1[k][j];
					}
				}
			}
		}

		// Send part of matrix a and the whole matrix b to workers.
		rows = SIZE / nproc;
		mtype = FROM_MASTER;
		offset = rows;
		for (dest = 1; dest < nproc; dest++)
		{
			if (DEBUG)
			{
				printf("   Sending %d rows to task %d\n", rows, dest);
			}

			MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
			MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
			MPI_Send(&a[offset][0], rows*SIZE, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
			MPI_Send(&b, SIZE*SIZE, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);

			offset += rows;
		}

		// Masters part of the calculation.
		for (i = 0; i < rows; i++) 
		{
			for (j = 0; j < SIZE; j++) 
			{
				c[i][j] = 0.0;

				for (k = 0; k < SIZE; k++)
				{
					c[i][j] = c[i][j] + a[i][k] * b[k][j];
				}
			}
		}

		// Collect the result from the rest of the nodes.
		mtype = FROM_WORKER;
		for (src = 1; src < nproc; src++) 
		{
			MPI_Recv(&offset, 1, MPI_INT, src, mtype, MPI_COMM_WORLD, &status);
			MPI_Recv(&rows, 1, MPI_INT, src, mtype, MPI_COMM_WORLD, &status);
			MPI_Recv(&c[offset][0], rows*SIZE, MPI_DOUBLE, src, mtype, MPI_COMM_WORLD, &status);

			if (DEBUG)
			{
				printf("   Recived %d rows from task %d, offset = %d\n", rows, src, offset);
			}
		}

		end_time = MPI_Wtime();

		if (DEBUG)
		{
			print_matrix();
		}

		int time_taken = (end_time - start_time);
		printf("Execution time on %2d nodes: %f\n", nproc, time_taken);
    } 
	
	// Worker tasks.
	else if(myrank < PROCESSORS)
	{
		// Receive data from master node.
		mtype = FROM_MASTER;
		MPI_Recv(&offset, 1, MPI_INT, 0, mtype, MPI_COMM_WORLD, &status);
		MPI_Recv(&rows, 1, MPI_INT, 0, mtype, MPI_COMM_WORLD, &status);
		MPI_Recv(&a[offset][0], rows*SIZE, MPI_DOUBLE, 0, mtype, MPI_COMM_WORLD, &status);
		MPI_Recv(&b, SIZE*SIZE, MPI_DOUBLE, 0, mtype, MPI_COMM_WORLD, &status);

		if (DEBUG)
		{
			printf ("Rank = %d, offset = %d, row = %d, a[offset][0] = %e, b[0][0] = %e\n", myrank, offset, rows, a[offset][0], b[0][0]);
		}

		// Workers part of the calculation.
		for (i = offset; i < (offset + rows); i++)
		{
			for (j=0; j < SIZE; j++) 
			{
				c[i][j] = 0.0;

				for (k = 0; k < SIZE; k++)
				{
					c[i][j] = c[i][j] + a[i][k] * b[k][j];
				}
			}
		}

		if (DEBUG)
		{
			printf ("Rank = %d, offset = %d, row = %d, c[offset][0] = %e\n", myrank, offset, rows, a[offset][0]);
		}

		// Send result to the master node.
		mtype = FROM_WORKER;
		MPI_Send(&offset, 1, MPI_INT, 0, mtype, MPI_COMM_WORLD);
		MPI_Send(&rows, 1, MPI_INT, 0, mtype, MPI_COMM_WORLD);
		MPI_Send(&c[offset][0], rows*SIZE, MPI_DOUBLE, 0, mtype, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}