/*
 * Exec.c
 *
 *  Created on: 25-Sep-2014
 *      Author: matrix
 */

#include "qshell.h"

/**
 *Checks input command string for valid if-then-else-fi syntax,
 *builds command Linked List if syntax is valid and executes it
 *as per conditions.
 *
 *Sample input command: if date; then who; else ifconfig; fi;
 */
void conditionalExec(char *line) {
	char *del = ";";
	char *cmdTokens[MAX_LENGTH];
	int i;

	//Flag to check if else is provided
	bool execElse = false;

	int cmdTokenCnt;
	tokenizeStr(line, del, &cmdTokenCnt, cmdTokens);

	//There should be at at least 3 tokens, if, then and fi
	if (cmdTokenCnt < 3) {
		puts(SYNTAX_ERR_MSG);
		return;
	}

	//OK, at this point we have minimum tokens.
	//But are they valid tokens and commands?
	//Following sections will check that too.
	del = " ";
	char *thenTokens[MAX_LENGTH];
	int thenTokenCnt;

	char *tmpStr1 = malloc(MAX_LENGTH);

	//Store 'then' section tokens in tmp variable, we need to restore it later
	strcpy(tmpStr1, cmdTokens[1]);

	//Tokenize 'then' section (for example, then who;)
	tokenizeStr(tmpStr1, del, &thenTokenCnt, thenTokens);
	//First token must be 'then' followed by some command
	if (strcmp(thenTokens[0], "then") || (thenTokenCnt < 2)) {
		puts(SYNTAX_ERR_MSG);
		return;
	}

	char *elsefiTokens[MAX_LENGTH];
	int elsefiTokenCnt;

	char *tmpStr2 = malloc(MAX_LENGTH);

	//Store 'else' or 'fi' section tokens in tmp variable, we need to restore it later.
	//User may not provide else. Instead he/she will conclude the command with 'fi' immediately after 'if'
	strcpy(tmpStr2, cmdTokens[2]);

	//Tokenize 'else' or 'fi' section (for example, else ifconfig; , fi;)
	tokenizeStr(tmpStr2, del, &elsefiTokenCnt, elsefiTokens);

	//If first token is 'else' it must be followed by valid command followed by 'fi'
	if ((!strcmp(elsefiTokens[0], "else") && (elsefiTokenCnt >= 2)
			&& cmdTokens[3] != NULL && !strcmp(cmdTokens[3], "fi"))) {

		//Else token detected, so set flag to true
		execElse = true;

	}
	//If 'else' section is not provided, the token must be 'fi'
	else if (strcmp(elsefiTokens[0], "fi")) {
		puts(SYNTAX_ERR_MSG);
		return;
	}

	//OK, we have comlte valid command at this point. Let's execute it.

	//Prepare command linked list out of commands in 'if' section
	struct command* ifHEAD = prepareCmd(cmdTokens[0]);

	//Execute 'if' commands and get the return code
	int ret = executeCmds(ifHEAD);
	freecommand(ifHEAD);

	//if command succeeded, execute 'then' commands
	if (ret == 0) {
		//Execute then
		i = 0;
		while (i < thenTokenCnt - 1) {
			thenTokens[i] = thenTokens[i + 1];
			i++;
		}
		struct command* thenHEAD = prepareCmdTok(thenTokens, thenTokenCnt - 1);
		executeCmds(thenHEAD);
		freecommand(thenHEAD);
	}
	//if command failed, and else commands provided, then execute them
	else if (execElse) {
		//Execute else
		i = 0;
		while (i < elsefiTokenCnt - 1) {
			elsefiTokens[i] = elsefiTokens[i + 1];
			i++;
		}
		struct command* elseHEAD = prepareCmdTok(elsefiTokens,
				elsefiTokenCnt - 1);
		executeCmds(elseHEAD);
		freecommand(elseHEAD);
	}
}

/**
 *Traverse the command linked list and execute command at each node
 *
 */
int executeCmds(struct command* HEAD) {
	struct command* tmp = HEAD;
	int ret = 0;

	setRunningProc(-1);

	while (tmp != NULL) {
		if (strcmp(tmp->cmd_name, "cd") == 0) {
			performCd(tmp);
		} else if (strcmp(tmp->separator, "&&") == 0) {
			ret |= executeSingleCmd(tmp);
			if (ret)
				break;
		} else if (strcmp(tmp->separator, "||") == 0) {

			ret = ret | executeSingleCmd(tmp);
			if (!ret)
				break;
		} else {
			ret = ret | executeSingleCmd(tmp);
		}
		tmp = tmp->next;
	}

	return ret;
}

/**
 *Performs 'cd'.
 *There is no system command for this, so it has to be handled specially.
 */
void performCd(struct command* tmp) {
	char dir[MAX_LENGTH];

	if (tmp->cmd_args) {
		//handle relative paths here
		if (*tmp->cmd_args != '/') {
			if (strcmp(tmp->cmd_args, "~") == 0) {
				// ~ is home directory
				strcpy(dir, HOME_DIR);
			} else if (*tmp->cmd_args == '~' && *(tmp->cmd_args + 1) == '/') {
				// path relevant from home directory
				strcpy(dir, HOME_DIR);
				strcat(dir, tmp->cmd_args + 1);
			} else {
				// fall here for rest of the types of cd
				// just append the arguments to current directory
				if (getcwd(dir, sizeof(dir)) != NULL) {
					strcat(dir, "/");
					strcat(dir, tmp->cmd_args);
				} else
					printf("\ngetcwd: %s\n", strerror(errno));
			}
		} else
			strcpy(dir, tmp->cmd_args);
	} else
		strcpy(dir, HOME_DIR);

	// now change the directory and update the prompt
	if (chdir(dir) == -1)
		printf("\ncd : %s\n", strerror(errno));
	else {
		// get the entire prompt and display it
		if (getcwd(dir, sizeof(dir)) != NULL) {
			if (strcmp(dir, HOME_DIR) == 0)
				strcpy(PROMPT, "~");
			else {
				// just display the current directory name
				// not the complete path till current dir
				strcpy(PROMPT, basename(dir));
			}
			strcat(PROMPT, "$");
		} else
			printf("\ngetcwd: %s\n", strerror(errno));

	}
}

/**
 * Executes a single command node
 */
int executeSingleCmd(struct command* tmp) {

	//Get command's full path from PATH
	char *cmdPath = getCmdPath(tmp->cmd_name, pathTokens, pathTokenCount);

	//Check if command is available in any of the lcoations from PATH
	if (cmdPath == NULL) {
		printf("%s Command not found in PATH.", tmp->cmd_name);
		printf("\n");
		return -1;
	}

	pid_t pid;
	char *argv[] = { tmp->cmd_name, tmp->cmd_args, NULL };
	pid = fork();

	//If fork() failed, retry 3 times
	if (pid == -1) {
		fprintf(stderr, "\nfork failed: %s\n", strerror(errno));
		fprintf(stderr, "Retrying\n");
		// fork failed.. retry and give 3 attempts before exiting
		int attempt = 3;
		while (attempt != 0) {
			sleep(2);
			pid = fork();
			if (pid != -1) {
				// we are successful
				break;
			}
			attempt--;
		}
		if (attempt == 0) {
			// no success.. exit
			fprintf(stderr, "\nFATAL: could not fork a process\n");
			exit(1);
		}
	}

	if (pid == 0) {
		//In child process

		//Check if output stream manipulation / redirection is needed
		if ((tmp->output_stream != NULL) || (tmp->append_stream != NULL)) {
			int mode;
			int outFd;

			//Truncate the file and write output to it
			if (tmp->output_stream) {
				mode = O_WRONLY | O_CREAT | O_TRUNC;
				outFd = open(tmp->output_stream, mode);
			}
			//Append output to existing content in file
			else if (tmp->append_stream) {
				mode = O_WRONLY | O_CREAT | O_APPEND;
				outFd = open(tmp->append_stream, mode);
			}

			//Ouput redirection failed
			if (outFd == -1) {
				fprintf(stderr, "%s\n", strerror(errno));
			} else {
				//Close child's default output file descriptor
				close(STD_OUT);
				//Point default output descriptor to new output file descriptor
				if (dup(outFd) == -1) {
					fprintf(stderr, "\nFATAL: Could not set file descriptor\n");
					exit(1);
				}
			}
		}

		//Check if input stream manipulation / redirection is needed
		if (tmp->input_stream != NULL) {
			int inFd = open(tmp->input_stream, O_RDONLY);

			//Input redirection failed
			if (inFd == -1) {
				fprintf(stderr, "%s\n", strerror(errno));
			} else {
				//Close child's default input file descriptor
				close(STD_IN);
				//Point default input descriptor to new input file descriptor
				if (dup(inFd) == -1) {
					fprintf(stderr, "\nFATAL: Could not set file descriptor\n");
					exit(1);
				}
			}
		}

		//Execute child process in background
		if (bg) {
			signal(SIGCHLD, SIG_IGN); //set SIGCHLD to avoid zombie process if process is running in background.
			execv(cmdPath, argv);
		}
		//Execute child process in foreground
		else {
			signal(SIGCHLD, SIG_DFL); //set SIGCHLD to default for foreground process
			execv(cmdPath, argv);
		}
	}

	if (pid) {
		//You are in parent process check if its background job if background print child processid
		if (bg) {
			//Print child process' pid which is put in background
			printf("[%d]\n", pid);
			g_intRunProc[g_intNoRunProc] = pid;
			alarmprocess[g_intNoRunProc] = (int) time(NULL);
			if (g_intNoRunProc == 0) {
				alarm(5);
			}

			g_intNoRunProc++;
		}
		//if foreground job then wait for child to finish and then exit.
		else {
			// we are in parent
			int status;

			setRunningProc(pid);

			g_intRunProc[g_intNoRunProc] = pid;
			alarmprocess[g_intNoRunProc] = (int) time(NULL);
			if (g_intNoRunProc == 0) {
				alarm(5);
			}
			g_intNoRunProc++;
			waitpid(pid, &status, 0);
			return status;
		}
	}
}

/**** add an alias to a system command ****/

int f_addalias(char *l_cptrAliasName, char *l_cptrRealCmd, char *path[]) {

	char **p = path;
	int l_intCmdPresence = 0;
	int l_intAliasCreate = 1;
	while (*p != NULL) {

		char *l_cptrFullAddr = malloc(strlen(*p) + strlen(l_cptrRealCmd) + 2);
		if (l_cptrFullAddr != NULL) {
			strcpy(l_cptrFullAddr, *p);
			strcat(l_cptrFullAddr, "/");
			strcat(l_cptrFullAddr, l_cptrRealCmd);

			if (access(l_cptrFullAddr, F_OK) != -1) {
				l_intCmdPresence = 1;
			}

			free(l_cptrFullAddr);
			l_cptrFullAddr = NULL;
		}

		char *l_cptrAliasPath = malloc(
				strlen(*p) + strlen(l_cptrAliasName) + 2);
		if (l_cptrAliasPath != NULL) {
			strcpy(l_cptrAliasPath, *p);
			strcat(l_cptrAliasPath, "/");
			strcat(l_cptrAliasPath, l_cptrAliasName);

			if (access(l_cptrAliasPath, F_OK) != -1) {
				l_intCmdPresence = -1;
			}

			free(l_cptrAliasPath);
			l_cptrAliasPath = NULL;
		}

		*p++;
	}

	*p = NULL;

	if (l_intCmdPresence == 0) {
		puts("Error: Command not present");	//Specified system command not present
		puts("Cannot add to alias");
		return -1;
	}

	else if (l_intCmdPresence == -1) {
		puts("Error: Alias is a system command");//If alias specified is a system command
		puts("Cannot add to alias");
		return -2;
	}

	else {

		FILE * fp;
		char * line = NULL;
		size_t len = 0;
		ssize_t read;

		//open ALIAS_PROFILE file from home directory

		char *file_name = malloc(
				strlen(HOME_DIR) + strlen("ALIAS_PROFILE") + 2);
		strcpy(file_name, HOME_DIR);
		strcat(file_name, "/ALIAS_PROFILE");

		if (access(file_name, F_OK) != -1) {
			fp = fopen(file_name, "r");
			while ((read = getline(&line, &len, fp)) != -1) {
				char *cmd = strtok(line, " =");
				if (strcmp(cmd, l_cptrAliasName) == 0) {//check if the same alias is already assigned
					puts("Error: Alias already present");
					puts("Cannot add to alias");//Error shown if alias already present
					l_intAliasCreate = 0;
					break;
				}

				if (cmd != NULL) {
					cmd = NULL;
				}
			}
			fclose(fp);
		} else {
			l_intAliasCreate = 1;
		}

		fp = fopen(file_name, "a+");
		if (l_intAliasCreate == 1) {
			fprintf(fp, "%s=%s\n", l_cptrAliasName, l_cptrRealCmd);	//In case of no error, define alias in file
		}

		fclose(fp);
		fp = NULL;
		if (line)
			free(line);

		free(file_name);
		file_name = NULL;
	}

	if (l_intAliasCreate == 0) {
		return 0;
	}

	return 1;
}

