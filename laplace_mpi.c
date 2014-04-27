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

int main(int argc, char **argv)
{
	// Generate the matrix.
	InitializeMatrix();

	/* TODO
	2. Divide into blocks
	3. Send blocks to other nodes
	4. Each node calculates their block, row by row
	5. Exchange row values with adjacent blocks (Note: Use Sendreceive to avoid deadlock!)
	6. Check acceptance value, if we have not passed it yet, goto 4
	7. If we pass the acceptance value, add everything together into one matrix

	Add print matrix function.
	*/
}



