/* myshell.c */
/* $begin shellmain */
#include "myshell.h"
#include <errno.h>
#define MAXARGS   128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 
void my_cd(char **argv);
void my_pipe(char **argv, int bg);
void eliminate_ch(char *str, char ch);

int main() 
{
	char cmdline[MAXLINE]; /* Command line */

	while (1) {
		/* Read */
		printf("CSE4100-SP-P4> ");                   
		fgets(cmdline, MAXLINE, stdin); 
		if (feof(stdin))
			exit(0);

		/* Evaluate */
		eval(cmdline);
	} 
}
/* $end shellmain */

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
	char *argv[MAXARGS]; /* Argument list execve() */
	char buf[MAXLINE];   /* Holds modified command line */
	int bg;              /* Should the job run in bg or fg? */
	pid_t pid;           /* Process id */



	strcpy(buf, cmdline);
	bg = parseline(buf, argv);

	if (argv[0] == NULL)  
		return;   /* Ignore empty lines */

	if(strcmp(argv[0],"exit") == 0) exit(0);
	else if(strcmp(argv[0], "cd") == 0){
		my_cd(argv);
	}
	else if(strchr(cmdline, '|') != NULL) my_pipe(argv, bg);
	else if (!builtin_command(argv)) { //quit -> exit(0), & -> ignore, other -> run
		if((pid = Fork()) == 0){  // Child process
			//char tmp_argv[20] = "/bin/";
			//strcat(tmp_argv,argv[0]);
			//*argv=tmp_argv;
			if (execvp(argv[0], argv) < 0) {	//ex) /bin/ls ls -al &
				printf("%s: Command not found.\n", argv[0]);
				exit(0);
			}
		}

		/* Parent waits for foreground job to terminate */
		if (!bg){ 
			int status;
			Waitpid(pid, &status, 0);
		}
		else//when there is backgrount process!
			printf("%d %s", pid, cmdline);
	}
	return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
	if (!strcmp(argv[0], "quit")) /* quit command */
		exit(0);  
	if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
		return 1;
	return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
	char *delim;         /* Points to first space delimiter */
	int argc;            /* Number of args */
	int bg;              /* Background job? */

	buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
	while (*buf && (*buf == ' ')) /* Ignore leading spaces */
		buf++;

	/* Build the argv list */
	argc = 0;
	while ((delim = strchr(buf, ' '))) {
		argv[argc++] = buf;
		*delim = '\0';
		buf = delim + 1;
		while (*buf && (*buf == ' ')) /* Ignore spaces */
			buf++;
	}
	argv[argc] = NULL;

	if (argc == 0)  /* Ignore blank line */
		return 1;
	
	for(int i=0 ; i< argc ; i++){
		eliminate_ch(argv[i], '"');
	}


	/* Should the job run in the background? */
	if ((bg = (*argv[argc-1] == '&')) != 0)
		argv[--argc] = NULL;

	return bg;
}
/* $end parseline */

/* $begin my_cd */
/* my_cd - execute cd instruction */
void my_cd(char **argv){
	int argc=0;
	while(argv[argc++] != NULL);
	argc--;
	if(argc == 1)
		chdir(getenv("HOME"));
	else if(argc == 2){
		if(chdir(argv[1])) printf("There's no directory.\n");
	}
}
/* $end my_cd */

/* $begin my_pipe */
/* my_pipe - Implement Pipe */
void my_pipe(char **argv, int bg){
	int argc = 0;
	int pipe_num = 0;
	int pipe_addr[10] = {-1, };
	pid_t pid;

	while(argv[argc++] != NULL);
	argc--; // 명령어 개수

	for(int i=0 ; i < argc ; i++){
		if(!strcmp(argv[i], "|"))
			pipe_addr[1 + pipe_num++] = i;
	}
	pipe_addr[pipe_num + 1] = argc;

	int fd[2];
	int fdd = 0;
	char *tmp_argv[10];
	int pipe_loc=0;

	for( int i=0 ; i<=pipe_num ; i++){
		pipe(fd);
		for(int j=0 ; j<10 ; j++) tmp_argv[j] = NULL;

		int idx = 0;

		for(int j=pipe_addr[i]+1 ; j<pipe_addr[i+1] ; j++){
			// printf("%d: %s\n", j, argv[j]);
			tmp_argv[idx++] = argv[j];
			
		}
		if((pid = Fork()) == 0){  // Child process
			//char tmp_argv[20] = "/bin/";
			//strcat(tmp_argv,argv[0]);
			//*argv=tmp_argv;
			dup2(fdd, 0);
			if(i != pipe_num) dup2(fd[1], 1);
			close(fd[0]);


			if (execvp(tmp_argv[0], tmp_argv) < 0) {	//ex) /bin/ls ls -al &
				printf("%s: Command not found.\n", tmp_argv[0]);
				exit(0);
			}
		}
		else{
			if (!bg){ 
				int status;
				Waitpid(pid, &status, 0);
				close(fd[1]);
				fdd = fd[0];
			}
			else//when there is backgrount process!
				printf("%d", pid);

		}
	}
	return;
}
/* $end my_pipe */

/* $begin eliminate_ch */
/* eliminate_ch - eliminate the character */
void eliminate_ch(char *str, char ch)
{    
    for (; *str != '\0'; str++){
        if (*str == ch){
            strcpy(str, str + 1);
            str--;            
        }
    }
}
/* $end eliminate_ch */

