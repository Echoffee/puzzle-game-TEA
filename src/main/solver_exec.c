#include <stdlib.h>
#include <stdio.h>
#include <solver.h>
#include <function_interface.h>

int main(int argc, char* argv[])
{
	if (argc != 3)
		printf("Usage : ./solver [a|r] <file>\n");

	game g = getGameFromConfigFile(argv[2]);
	solve(g, argv[1][0] == 'r');
	return 0;
}