/*
 * qshell.h
 *
 *  Created on: 20-Sep-2014
 *      Author: matrix
 */

#ifndef QSHELL_H_
#define QSHELL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>

#define MAX_LENGTH 512
#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2

typedef int bool;
#define true 1
#define false 0

bool bg;
extern char *HOME_DIR;
extern char *PROMPT;
extern char *PATH;
extern char *PROFILE;
extern char *SYNTAX_ERR_MSG;
extern char *ALARM;
extern char *DEFAULT_PATH;
extern pid_t runningProc;

char *pathTokens[32];
char *l_cptrRealCmd_name;
int pathTokenCount;
volatile int l_intNewPrompt;
int g_intRunProc[32];
unsigned int alarmprocess[32];

extern volatile int g_intNoRunProc;
extern volatile char g_chrYN;

struct command {
	//Actual command
	char *cmd_name;
	//Command arguments
	char *cmd_args;
	//Special symbol &&, ||, |, ; followed by command
	char *separator;
	//Input stream from which command will get input
	char *input_stream;
	//Ouput stream to which this command's o/p is to be written
	char *output_stream;
	//Ouput stream to which this command's o/p is to be appended
	char *append_stream;
	//This command's return code
	int ret;
	//Next command following this command
	struct command* next;
};

//Utils
void loadProfile();
struct command* initNewCmd();
void freecommand(struct command* head);
char* getCmdPath(char *cmd, char *path[], int);
void tokenizeStr(char *, char *, int *, char *[]);
int alias_retrieve(char *l_cptrAliasName);
struct command* initCmdLL(char *cmdTokens[], int i);
struct command* prepareCmdTok(char *cmdTokens[], int i);
struct command* prepareCmd(char *cmd);
void setRunningProc(pid_t pid);
void sayBye();

//Exec
int executeCmds(struct command*);
void performCd(struct command* tmp);
int executeSingleCmd(struct command* tmp);
void conditionalExec(char *);
int f_addalias(char *l_cptrAliasName, char *l_cptrRealCmd, char *path[]);

//Handler
void alarmhandler(int sig);
void inthandler(int sig);

#endif /* QSHELL_H_ */
