/*
 * Handler.c
 *
 *  Created on: 25-Sep-2014
 *      Author: matrix
 */

#include "qshell.h"

/**
 *Alarm handler
 */
void alarmhandler(int sig) {

	int l_procIter = 0;
	int errno;
	int returnStatus;
	unsigned int l_intNextAlarmStmp;
	bool l_boolquitval = 0;
	char g_chrAlarmIn;

	if (strstr(ALARM, "ENABLED") == ALARM) {

		//Check if process is running and if so then ask user for his desire to stop the process

		if (kill(g_intRunProc[0], 0) == 0) {
			printf("Do you want to stop the program? (y/n)\n");

			while (l_boolquitval == false) {
				//g_chrAlarmIn = getchar();

				while (!isalnum(g_chrAlarmIn)) {
					scanf(" %c", &g_chrAlarmIn);	//User input
				}

				//If user wants to stop process
				if ((g_chrAlarmIn == 'y') || (g_chrAlarmIn == 'Y')) {
					puts("Killing process");
					kill(g_intRunProc[0], SIGKILL);				//Kill process
					l_boolquitval = true;
				}

				//If user does not want to stop process
				else if ((g_chrAlarmIn == 'n') || (g_chrAlarmIn == 'N')) {
					puts("Not killing process. Waiting for it to complete");
					waitpid(g_intRunProc[0], &returnStatus, 0);	//Wait for process to be stopped

					if (returnStatus == -1) {
						if (errno == ECHILD) {
							puts("Error: Process does not exist");//Inexistent process
						} else if (errno == ENOSYS) {
							puts("Error: Process group not supported");
						}
					}
					l_boolquitval = true;
				}

				else {
					puts("Invalid input. Enter again");
					g_chrAlarmIn = '\0';
				}
			}
		}

	} else {

		waitpid(g_intRunProc[0], &returnStatus, 0);	//Wait for process to be stopped

		if (returnStatus == -1) {
			if (errno == ECHILD) {
				puts("Error: Process does not exist");		//Inexistent process
			} else if (errno == ENOSYS) {
				puts("Error: Process group not supported");
			}
		}
	}
	for (l_procIter = 0; l_procIter < g_intNoRunProc; l_procIter++) {
		g_intRunProc[l_procIter] = g_intRunProc[l_procIter + 1];
		alarmprocess[l_procIter] = alarmprocess[l_procIter + 1];
	}

	g_intNoRunProc--;
	if (g_intNoRunProc > 0) {
		l_intNextAlarmStmp = alarmprocess[0] + 5000 - (int) time(NULL);	//schedule next alarm
		alarm(l_intNextAlarmStmp / 1000);
	}
	g_chrAlarmIn = '\0';
}

/*
 * Terminate the shell
 */
void sayBye() {
	printf("Do you want to exit from qShell? (y/n)\n");
	g_chrYN = getchar();

	//Terminate the process
	if ((g_chrYN == 'y') || (g_chrYN == 'Y')) {
		exit(0);
	} else if (g_chrYN != 'N' && g_chrYN != 'n') {
		printf("\nUnrecognised option %c", g_chrYN);
	}
	printf("\n%s ", PROMPT);
	g_chrYN = '\0';
	return;
}

/**
 * Handle Ctrl + C (SIGINT) signal
 */
void inthandler(int sig) {
	if (runningProc > 0)
		kill(runningProc, SIGINT);
	else
		sayBye();
}

