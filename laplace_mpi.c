
int main(int argc, char **argv)
{
	/*
	1. Generate matrix
	2. Divide into blocks
	3. Send blocks to other nodes
	4. Each node calculates their block, row by row
	5. Exchange row values with adjacent blocks (Note: Use Sendreceive to avoid deadlock!)
	6. Check acceptance value, if we have not passed it yet, goto 4
	7. If we pass the acceptance value, add everything together into one matrix
	*/
}

