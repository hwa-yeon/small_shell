#include "smallsh.h"

char *prompt = "Command> ";

int main() {
	while(userin(prompt) != EOF)
		procline();
	return 0;
}

