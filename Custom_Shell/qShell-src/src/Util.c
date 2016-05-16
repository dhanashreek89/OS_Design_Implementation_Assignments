/*
 * Util.c
 *
 *  Created on: 25-Sep-2014
 *      Author: matrix
 */

#include "qshell.h"
pid_t runningProc = -1;
/**
 * Prepares a command linked list out of plain command string provided by user
 */
struct command* prepareCmd(char *cmd) {
	char *del = " ";
	int i = 0;
	char *cmdTokens[MAX_LENGTH];
	tokenizeStr(cmd, del, &i, cmdTokens);
	return initCmdLL(cmdTokens, i);
}

/**
 * Prepare a command linked list out of already tokenized command string
 */
struct command* prepareCmdTok(char *cmdTokens[], int i) {
	return initCmdLL(cmdTokens, i);
}

/**
 * Initialize command linked list out of command line tokens
 */
struct command* initCmdLL(char *cmdTokens[], int i) {
	int j;
	struct command* HEAD = NULL;
	struct command* tmp = NULL;

	j = 0;
	//Iterate through every token and build linked list
	for (j = 0; j < i; j++) {
		if (j == 0) {
			tmp = initNewCmd();
			HEAD = tmp;
			strcpy(tmp->cmd_name, cmdTokens[j]);
		} else if (!strcmp(cmdTokens[j], "&&") || !strcmp(cmdTokens[j], "||")
				|| !strcmp(cmdTokens[j], ";") || !strcmp(cmdTokens[j], "|")) {
			strcpy(tmp->separator, cmdTokens[j]);
			tmp->next = initNewCmd();
			tmp = tmp->next;
			j++;
			strcpy(tmp->cmd_name, cmdTokens[j]);
		} else if (!strcmp(cmdTokens[j], ">")) {
			//Output redirection detected
			tmp->output_stream = malloc(MAX_LENGTH);
			strcpy(tmp->output_stream, cmdTokens[++j]);
		} else if (!strcmp(cmdTokens[j], "<")) {
			//Input redirection detected
			tmp->input_stream = malloc(MAX_LENGTH);
			strcpy(tmp->input_stream, cmdTokens[++j]);
		} else if (!strcmp(cmdTokens[j], ">>")) {
			//Output redirection detected
			tmp->append_stream = malloc(MAX_LENGTH);
			strcpy(tmp->append_stream, cmdTokens[++j]);
		} else if (strcmp(cmdTokens[j], "&")) {
			if (!tmp->cmd_args) {
				tmp->cmd_args = malloc(MAX_LENGTH);
				strcpy(tmp->cmd_args, "\0");
			}
			strcat(tmp->cmd_args, cmdTokens[j]);
		}
	}

	//Current command has to be executed in background
	if (!strcmp(cmdTokens[--j], "&")) {
		bg = true;
	}

	return HEAD;
}

/**** retrieves original functions from alias functions ****/
int alias_retrieve(char *l_cptrAliasName) {

	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	// open ALIAS_PROFILE file

	char *file_name = malloc(strlen(HOME_DIR) + strlen("ALIAS_PROFILE") + 2);
	strcpy(file_name, HOME_DIR);
	strcat(file_name, "/ALIAS_PROFILE");

	fp = fopen(file_name, "r");

	if (fp == NULL) {
		fp = NULL;
		return 0;
	}

	char *end = l_cptrAliasName + strlen(l_cptrAliasName) - 1;
	while (end > l_cptrAliasName && isspace(*end))
		end--;

	*(end + 1) = 0;
	while ((read = getline(&line, &len, fp)) != -1) {
		l_cptrRealCmd_name = strtok(line, " =");

		// loop through it and find if there is any match for given function in alias list
		if (strcmp(l_cptrRealCmd_name, l_cptrAliasName) == 0) {
			l_cptrRealCmd_name = strtok(NULL, " \n");
			fclose(fp);
			fp = NULL;
			return 1;
		}
	}

	fclose(fp);
	fp = NULL;
	if (line)
		free(line);

	free(file_name);
	file_name = NULL;

	l_cptrRealCmd_name = NULL;
	return 0;
}

/**
 *Read environment variables from profile file
 */
void loadProfile() {
	FILE *fp;
	char string[MAX_LENGTH];
	char *var, *val, *del;

	var = val = NULL;
	del = "=";

	// read the profile file and set the appropriate variables
	if ((fp = fopen(PROFILE, "r"))) {
		while (!feof(fp)) {
			strcpy(string, "\0");
			char * ptr = fgets(string, MAX_LENGTH, fp);
			if (ptr != NULL) {
				if (string[strlen(string) - 1] == '\n')
					string[strlen(string) - 1] = '\0';
				var = strtok(string, del);
				if (var) {
					val = strtok(NULL, del);
					if (!val)
						printf(
								"\nprofile: value not present for %s; using default value",
								var);
					if (strcmp(var, "HOME") == 0) {
						HOME_DIR = malloc(MAX_LENGTH);
						if (!val)
							strcpy(HOME_DIR, "/home");
						else
							strcpy(HOME_DIR, val);
					} else if (strcmp(var, "PROMPT") == 0) {
						PROMPT = malloc(MAX_LENGTH);
						strcpy(PROMPT, "~");
						if (!val)
							strcat(PROMPT, "$");
						else
							strcat(PROMPT, val);
					} else if (strcmp(var, "PATH") == 0) {
						PATH = malloc(MAX_LENGTH);
						if (!val)
							strcpy(PATH, "/bin:/usr/bin");
						else
							strcpy(PATH, val);
					} else if (strcmp(var, "ALARM") == 0) {
						ALARM = malloc(MAX_LENGTH);
						if (!val)
							strcpy(ALARM, "ENABLED");
						else
							strcpy(ALARM, val);
					} else
						printf("\nprofile: unrecognised symbol %s\n", var);
				}
			}
		}
		fclose(fp);
	}

	// check if all the variables have been set by profile file. If not, set them to default values
	if (HOME_DIR == NULL) {
		HOME_DIR = malloc(MAX_LENGTH);
		strcpy(HOME_DIR, "/home");
	}

	if (PROMPT == NULL) {
		PROMPT = malloc(MAX_LENGTH);
		strcpy(PROMPT, "~");
		strcat(PROMPT, "$");
	}

	if (PATH == NULL) {
		PATH = malloc(MAX_LENGTH);
		strcpy(PATH, DEFAULT_PATH);
	}

	// set the environment variable path
	if (setenv("PATH", PATH, 1) == -1)
		fprintf(stderr, "\nsetenv: %s\n", strerror(errno));
}

/**
 *Get the location of command in directories set in PATH.
 *Get Returns NULL if command not found in path
 */
char* getCmdPath(char *cmd, char *path[], int pathLength) {
	char *fullCmdPath = malloc(512);
	int i = 0;
	for (; i < pathLength; i++) {
		strcpy(fullCmdPath, path[i]);
		strcat(fullCmdPath, "/");
		strcat(fullCmdPath, cmd);
		if (access(fullCmdPath, X_OK) == 0)
			return fullCmdPath;
	}
	return NULL;
}

/**
 *Utility function to tokenize a string
 */
void tokenizeStr(char * str, char *del, int *count, char* array[]) {
	char *token = NULL;
	*count = 0;

	token = strtok(str, del);
	while (token != NULL) {
		char* p = token;
		while (isspace(*p)) {
			p++;
		}
		array[*count] = p;
		(*count)++;
		token = strtok(NULL, del);
	}
}

void setRunningProc(pid_t pid) {
	runningProc = pid;
}
/**
 *Initialize a command node
 */
struct command* initNewCmd() {
	struct command* tmp = malloc(sizeof(struct command));
	tmp->append_stream = NULL;
	tmp->cmd_args = NULL;
	tmp->cmd_name = malloc(MAX_LENGTH);
	tmp->input_stream = NULL;
	tmp->next = NULL;
	tmp->output_stream = NULL;
	tmp->ret = 0;
	tmp->separator = malloc(MAX_LENGTH);

	return tmp;
}

void freecommand(struct command* head) {

	struct command *tmp;
	while (head != NULL) {
		tmp = head;
		if (tmp) {
			free(tmp);
		}
		head = head->next;

	}

}
