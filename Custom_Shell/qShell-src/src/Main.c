#include "qshell.h"

char *HOME_DIR = NULL;
char *PROMPT = NULL;
char *PATH = NULL;
char *PROFILE = "/home/profile";
char *SYNTAX_ERR_MSG = "Syntax error!\n";
char *ALARM = "ENABLED";
char *DEFAULT_PATH =
		"/root/bin:/usr/local/bin:/bin:/sbin:/usr/bin:/usr/sbin:/usr/pkg/bin:/usr/pkg/sbin:/usr/pkg/X11R6/bin";

volatile int l_intNewPrompt = '1';
volatile int g_intNoRunProc = 0;
volatile char g_chrYN = '\0';

/**
 *Main Entry point for qShell
 */
int main(int argc, char *argv[]) {

	printf("\nWelcome to qShell! \n");

	char line[MAX_LENGTH];
	struct command* HEAD;
	int l_intAliasVal;
	loadProfile();
	char *del = ":";

	//Separate directories in PATH.
	tokenizeStr(PATH, del, &pathTokenCount, pathTokens);
	//Hook for Alarm signal
	signal(SIGALRM, alarmhandler);
	//Hook for Ctrl + C (SIGINT) signal
	signal(SIGINT, inthandler);

	// if user specified invalid home dir in profile file then set the default one and set the prompt accordingly
	if (chdir(HOME_DIR) == -1) {
		printf("\ncd: %s\nDefault home directory set.", strerror(errno));
		strcpy(HOME_DIR, "/home");
	}

	//Continuously print the prompt and read input command
	while (1) {
		setRunningProc(-1);

		bg = false;
		if (l_intNewPrompt != 0)
			printf("\n%s ", PROMPT);

		l_intNewPrompt = 1;
		while (!fgets(line, MAX_LENGTH, stdin)) {
			//break; ** do nothing.. not even break **
		}
		char* p = line;
		while (isspace(*p)) {
			p++;
		}

		if (p[0] == '\0') {
			printf("%s ", PROMPT);
		}

		strtok(p, "\n");

		//Handle if-then-else-fi command
		if ((strstr(p, "if") == p) && (p[2] == ' ')) {
			printf("\n");
			conditionalExec(p + 3);
		}

		//Handle alias command
		else if (strstr(p, "alias") == p) {
			char *cmd = strtok(p, " =\"");
			cmd = strtok(NULL, " =\"");
			char *l_cptrAliasName = cmd;
			cmd = strtok(NULL, " =\"");
			char *l_cptrRealCmd = cmd;
			strtok(PATH, ":");
			f_addalias(l_cptrAliasName, l_cptrRealCmd, pathTokens);

			if (cmd != NULL)
				cmd = NULL;

			if (l_cptrAliasName != NULL)
				l_cptrAliasName = NULL;

			if (l_cptrRealCmd != NULL)
				l_cptrAliasName = NULL;
		}

		//Handle alarm command
		else if (strstr(p, "alarm") == p) {
			if (strstr(p, "alarmoff") == p) {
				puts("Disabling alarm");
				ALARM = NULL;
				ALARM = malloc(9);
				strcpy(ALARM, "DISABLED");
			} else if (strstr(p, "alarmon") == p) {
				puts("Enabling alarm");
				ALARM = NULL;
				ALARM = malloc(8);
				strcpy(ALARM, "ENABLED");
			} else {
				puts("Invalid command");
				puts("Use alarmon to enable and alarmoff to disable alarm");
			}
		}

		//Let user exit with exiit command
		else if (strstr(p, "exit") == p) {
			sayBye();
		}

		else {
			if (strlen(p) != 0) {
				int i = 0;
				int l_intArgCount;
				char *l_carrCmdArgs[32];
				char *l_cptrTokFunc = malloc(strlen(p));
				strcpy(l_cptrTokFunc, p);
				tokenizeStr(l_cptrTokFunc, " ", &l_intArgCount, l_carrCmdArgs);
				l_intAliasVal = alias_retrieve(l_cptrTokFunc);

				//Use alias if it had been set previously
				if (l_intAliasVal == 1) {
					int l_intAliasFuncLen = 0;
					l_intAliasFuncLen = strlen(l_cptrRealCmd_name);
					while ((i + 1) < l_intArgCount) {
						l_intAliasFuncLen += strlen(l_carrCmdArgs[i + 1]);
						l_intAliasFuncLen++;
						i++;
					}
					char *l_cptrFullCmd = malloc(l_intAliasFuncLen);
					strcpy(l_cptrFullCmd, l_cptrRealCmd_name);
					i = 1;
					while ((i) < l_intArgCount) {
						strcat(l_cptrFullCmd, " ");
						strcat(l_cptrFullCmd, l_carrCmdArgs[i]);
						i++;
					}
					strcat(l_cptrFullCmd, "\0");
					HEAD = prepareCmd(l_cptrFullCmd);

					free(l_cptrFullCmd);
				} else {
					HEAD = prepareCmd(p);
				}
				executeCmds(HEAD);
				freecommand(HEAD);
				if (l_cptrTokFunc != NULL) {
					l_cptrTokFunc = NULL;
				}
			} else {
				l_intNewPrompt = 0;
			}
		}
	}

	return 0;
}
