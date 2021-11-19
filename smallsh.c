#include "smallsh.h"

static char inpbuf[MAXBUF];
static char tokbuf[2*MAXBUF];
static char *ptr = inpbuf;
static char *tok = tokbuf;

static char special[] = {' ', '\t', '&', ';', '\n', '\0'};

int userin(char* p) {
	int c, count;
	ptr = inpbuf;
	tok = tokbuf;

	printf("%s", p);
	count = 0;

	while(1) {
		if ((c = getchar()) == EOF)
			return EOF;
		if (count < MAXBUF)
			inpbuf[count++] = c;
		if (c == '\n' && count < MAXBUF) {
			inpbuf[count] = '\0';
			return count;
		}
		if (c == '\n' || count >= MAXBUF) {
			printf("smallsh: input line too long\n");
			count = 0;
			printf("%s", p);
		}
	}
}

int gettok(char** outptr) {
	int type;
	*outptr = tok;
	while (*ptr == ' ' || *ptr == '\t')
			ptr++;

	*tok++ = *ptr;
	switch (*ptr++) {
		case '\n':
			type = EOL;
			break;
		case '&':
			type = AMPERSAND;
			break;
		case ';':
			type = SEMICOLON;
			break;
		default:
			type = ARG;
			while(inarg(*ptr))
				*tok++ = *ptr++;
	}
	*tok++ = '\0';
	return type;
}

int inarg (char c) {
	char *wrk;

	for (wrk = special; *wrk; wrk++) {
		if (c == *wrk)
			return 0;
	}
	return 1;
}

void procline() {
	char *arg[MAXARG + 1];
	int toktype, type;
	int narg = 0;
	for (;;) {
		switch (toktype = gettok(&arg[narg])) {
			case ARG:
				if (narg < MAXARG)
					narg++;
				break;
			case EOL:
			case SEMICOLON:
			case AMPERSAND:
				if (toktype == AMPERSAND) {
					/*
				       	type = BACKGROUND;
					if(gettok(&arg[narg+1]) == SEMICOLON) {
						arg[narg] = NULL;
						narg++;
						break;
					}
					*/
				}
				else type = FOREGROUND;
				if (narg != 0) {
					/*
					if(gettok(&arg[narg-1]) == AMPERSAND) {
						type = BACKGROUND;
					}
					*/
					arg[narg] = NULL;
					runcommand(arg, type);
				}
				if (toktype == EOL) return;
				narg = 0;
				break;
		}
	}
}

int runcommand(char **cline, int where) {
	pid_t pid;
	int status;
	
	int redirect_stdout = dup(1);
	char* filename;
	int fd;
	char** argv;

	if(strcmp(*cline, "cd") == 0) {
		if(cline[2] || !cline[1]) {
			printf("Usage: cd <directory>\n");
		}
		else {
			chdir(cline[1]);
		}
		return 0;
	}

	if(strcmp(*cline, "exit") == 0) {
		exit(0);
	}

	switch (pid = fork()) {
		case -1:
			perror("smallsh");
			return -1;
		case 0:
			for(int i = 0; cline[i]; i++) {
				if(strcmp(cline[i], ">") == 0) {
					if(!cline[i+1]) 
						printf("Usage: redirection <file>\n");
					else {
						filename = cline[i+1];
						fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
						dup2(fd, 1); // STDOUT_FILENO
						
						cline[i] = '\0';
						cline[i+1] = '\0';

						close(fd);
					}
					break;
				}
			}	
			execvp(*cline, cline);
			perror(*cline);
			dup2(redirect_stdout, 1);
			exit(1);
	}
	/* following is the code of parent */
	if (where == BACKGROUND) {
		printf("[Process id] %d\n", pid);
		return 0;
	}
	if (waitpid(pid, &status, 0) == -1) 
		return -1;
	else 
		return status;
}

