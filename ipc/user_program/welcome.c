#include "qall.h"

int get_process_id(char *name)
{
	FILE *fp;
	char string[MAX_LENGTH];
	char *var, *val, *del;

	var = val = NULL;
	del = "=";

	// read the CONFIG file and check for the specified receiver
	if ((fp = fopen("CONFIG", "r"))) {
		while (!feof(fp)) {
			strcpy(string, "\0");
			char * ptr = fgets(string, MAX_LENGTH, fp);
			if (ptr != NULL) {
				if (string[strlen(string) - 1] == '\n')
					string[strlen(string) - 1] = '\0';
				var = strtok(string, del);
				if (var) {
					val = strtok(NULL, del);
					if(strcmp(var, name) == 0) {
						if (!val)
														printf("\nCONFIG: id not present for %s", name);
						else
							return atoi(val);
					}
				}
			}
		}
		fclose(fp);
	}

	return -1;

}

void nonblock_send()
{
	char ch;
	char data[MAX_LENGTH], name[MAX_LENGTH], qname[MAX_LENGTH];
	char new_name[MAX_LENGTH], senname[MAX_LENGTH];
	int ts;

	int priority, i =0, id = 0, ids[20] = {0}, totrec = 0, senid = 0;
	
	printf("\nEnter qname : ");
	scanf("%s", qname);

	printf("\nEnter sender name: ");
	scanf("%s", senname);
 	senid = get_process_id(senname);
	if (senid == -1) {
		printf("\nERROR: Could not fetch id for sender %s", senid);
		return;
	}
	printf("\nEnter total number of receivers : ");
	scanf("%d", &totrec);

	for(i=0; i<totrec; i++) {
		printf("\nEnter receiver %d name: ", i+1);
	 	scanf("%s", name);
		id = get_process_id(name);
		if (id == -1) {
                        printf("\nERROR: Could not fetch id for receiver %s", name);
		}
		else
		{
			ids[i] = id;
		}
	}	

	

	printf("\nEnter data to be sent: ");
        scanf("%s", data);
        printf("\nEnter priority : ");
        scanf("%d", &priority);

	printf("\nEnter message expiry timestamp: ");
	scanf("%d", &ts);
	
	if (totrec >0) {
 
	
		// send the message now
		int ret = _async_send(qname, senid, ids, totrec, data, priority, ts);
		if (ret == 9)
			printf("\nERROR: Queue %s does not exist", qname);
		else if( ret == 4)
			printf("\nERROR: Queue %s is full", qname);
		else if(ret == 5)
			printf("\nERROR: message add failed");
		else if(ret == 3)
			printf("\nMessage added to %s successfully", qname);
	}else
		printf("\nERROR: Could not send message");	
}

void block_receive() {
	char recname[MAX_LENGTH];
	char qname[MAX_LENGTH];
	char senname[MAX_LENGTH];
	int ch, recid, senid;

	printf("\nEnter queue name: ");
	scanf("%s", qname);

	printf("\nEnter the receiver name: ");
	scanf("%s", recname);

	printf("\nDo you wish to receive from a particular sender? (1/0): ");
	scanf("%d",&ch);

	if(ch) {
		printf("\nEnter the sender name: ");
		scanf("%s", senname);
		senid = get_process_id(senname);
		if(senid == -1) {
			printf("\nERROR: Could not fetch id for sender %s", senname);
			return;
		} 	
	}
	else
		senid = -1;
	
	recid = get_process_id(recname); 
 	if(recid == -1) {
		printf("\nERROR: Could not fetch id for receiver %s", recname);
                return;
	}
	
	_blocking_receive(qname, recid, senid);
}

void blocking_send(){

	char ch;
	char data[MAX_LENGTH], name[MAX_LENGTH], qname[MAX_LENGTH];
	char new_name[MAX_LENGTH], senname[MAX_LENGTH];
	int ts;

	int priority, i =0, id = 0, ids[20] = {0}, totrec = 0, senid = 0;
	
	printf("\nEnter qname : ");
	scanf("%s", qname);

	printf("\nEnter sender name: ");
	scanf("%s", senname);
 	senid = get_process_id(senname);
	if (senid == -1) {
		printf("\nERROR: Could not fetch id for sender %d", senid);
		return;
	}
	printf("\nEnter total number of receivers : ");
	scanf("%d", &totrec);

	for(i=0; i<totrec; i++) {
		printf("\nEnter receiver %d name: ", i+1);
	 	scanf("%s", name);
		id = get_process_id(name);
		if (id == -1) {
                        printf("\nERROR: Could not fetch id for receiver %s", name);
		}
		else
		{
			ids[i] = id;
		}
	}	

	

	printf("\nEnter data to be sent: ");
        scanf("%s", data);
        printf("\nEnter priority : ");
        scanf("%d", &priority);

	printf("\nEnter message expiry timestamp: ");
	scanf("%d", &ts);
	
	if (totrec >0) {
 
	
		// send the message now
		int ret = mq_send(qname, senid, ids, totrec, data, priority, ts);
		if (ret == 0)
			printf("\nERROR: Message could not be added to %s", qname);
		else if( ret == 1)
			printf("\nBlocking send successful");

	}else
		printf("\nError sending message");	


}

void reqnotify() {
	char recname[MAX_LENGTH];
	int retVal;
	printf("\nEnter the receiver name: ");
	scanf("%s", recname);


	int recid = get_process_id(recname); 
 	if(recid == -1) {
		printf("\nERROR: Could not fetch id for receiver %s", recname);
                return;
	}	
	retVal = mq_reqnotify( recid);
	
	if(retVal == 1){
		printf("\nNotification request success");
	}
	else{
		printf("\nERROR: Notification request failure");
	}
	
	while(notification_alert == 0);
	printf("\nNew message has come %d", retVal);
}

void Qopen()
{
	char qname[MAX_LENGTH], newname[MAX_LENGTH];

	printf("\nEnter queue to be created: ");
	scanf("%s", qname);

	int i = _open_queue(qname, newname);
	if (i==2)
		printf("\nERROR: Queue %s open failed", qname);
	else if(i==1)
		printf("\nERROR: Queue %s already exists", qname);
	else
		printf("\nQueue %s created successfully", qname);	
}

void Qclose()
{
	char qname[MAX_LENGTH];

        printf("\nEnter queue to be closed: ");
        scanf("%s", qname);

	int i = _close_queue(qname);
	if(i==9)
                printf("\nERROR: Queue %s does not exist", qname);
        else
                printf("\nQueue %s closed successfully", qname);     


}

void Qsetattr()
{
	char qname[MAX_LENGTH];

        printf("\nEnter queue name: ");
        scanf("%s", qname);

	int capacity, type;
	printf("\nEnter capacity for queue %s: ", qname);
	scanf("%d", &capacity);

	printf("\nEnter type (blocking/nonblocking) for queue %s: ", qname);
	scanf("%d", &type);

	int i = _setattr_queue(qname, capacity, type);
	if (i==9)
		printf("\nERROR: Queue %s does not exist", qname);
	else
		printf("\nAttributes set successfully for %s", qname);
}

void Qgetattr()
{
	char qname[MAX_LENGTH];

        printf("\nEnter queue name: ");
        scanf("%s", qname);

	int capacity = 0, type = 0;
	int i = _getattr_queue(qname, &capacity, &type);
	if(i==9)
		printf("\nERROR: Queue %s does not exist", qname);
	else
	{
		printf("\n\t Capacity : %d", capacity);
		printf("\n\t Type     : %d", type);
	}
}

void nonblock_receive()
{
	char recname[MAX_LENGTH];
	char qname[MAX_LENGTH];
	char senname[MAX_LENGTH];
	int ch, recid, senid;

	printf("\nEnter queue name: ");
	scanf("%s", qname);

	printf("\nEnter the receiver name: ");
	scanf("%s", recname);

	printf("\nDo you wish to receive from a particular sender? (1/0): ");
	scanf("%d",&ch);

	if(ch) {
		printf("\nEnter the sender name: ");
		scanf("%s", senname);
		senid = get_process_id(senname);
		if(senid == -1) {
			printf("\nERROR: COuld not fetch id for sender %s", senname);
			return;
		} 	
	}
	else
		senid = -1;
	
	recid = get_process_id(recname); 
 	if(recid == -1) {
		printf("\nERROR: Could not fetch id for receiver %s", recname);
                return;
	}

	_async_receive(qname, recid, senid);	
}

int main(void)
{
	int input;

	system("clear");

	printf("*****************************************************\n");
	printf("*                 Welcome to QIPC                   *\n");
	printf("*****************************************************\n\n");
	do{
		printf("\n\n1. Open Queue\n");
		printf("2. Close Queue\n");
		printf("3. Set Attribute\n");
		printf("4. Get Attribute\n"); 
		printf("5. Non Blocking Send\n");
		printf("6. Non Blocking Receive\n");
		printf("7. Blocking Receive\n");
		printf("8. Notify\n");
		printf("9. Blocking Send\n");
		printf("10. Exit\n");
		printf("Choice: ");

		scanf("%d",&input);
		fflush(stdin);
		fflush(stdout);

		switch(input)
		{
			case 1:
				Qopen();
				break;
			case 2:
				Qclose();
				break;
			case 3:
				Qsetattr();
				break;
			case 4:
				Qgetattr();
				break;
			case 5:
				nonblock_send();
				break;
			case 6:
				nonblock_receive();
				break;
			case 7:
				block_receive();
				break;
			case 8:
				reqnotify();
				break;
			case 9:
				blocking_send();
				break;				
			case 10:
				printf("Exiting.\n");
				exit(0);
				break;
			default:
				printf("\nInvalid choice");
				break;
		}

	}while(1);
}
