#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/shm.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <arpa/inet.h>
//#include <sys/msg.h>

#define DELIM " \t\n\r"

typedef struct
{
	int msqid;
	char name[128];
} user;

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

int main(int argc , char *argv[])
{
	/*for socket*/
	int socket_desc , new_socket , c;
	struct sockaddr_in server , client;
	int port = atoi(argv[1]);
	/*for shared memory*/
	int shm_id;
	int shm_id2;

	/*Create shared memory. shm_id is used to save struct. shm_id2 is used to save size*/
	if ((shm_id = shmget(IPC_PRIVATE, sizeof(user) * 100, IPC_CREAT | 0666)) < 0) //| IPC_EXCL 
	{
		perror("shmget error");
		return 1;
	}
	if ((shm_id2 = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) < 0) //| IPC_EXCL 
	{
		perror("shmget2 error");
		return 1;
	}
	int *i;
	if((i = shmat(shm_id2,NULL,0)) < 0)
	{
		perror("shmat error");
	}
	*i = 0;
	shmdt(i);
     
	/*Create socket*/
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
     
	/*Prepare the sockaddr_in structure*/
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( port );

	/*Make socket port reuseable*/
	int on = 1;
	int status = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on));
	if (status == -1)
	{
		perror("setsockopt error");
		exit(1);
	}
     
	/*Bind*/
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("bind failed");
		exit(1);
	}
	puts("binding...");
     
	/*Listen*/
	listen(socket_desc , 3);

	/*Accept and incoming connection*/
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while ( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Connection accepted");
		
		/*fork ,child for client*/
		pid_t child = fork();
		if (child < 0)
		{
			perror("fork error");
			return 1;
		}
		else if (child == 0)
		{
			key_t key;
			key = ftok("chat.c", 'B');
			int msqid;
			if ((msqid = msgget(IPC_PRIVATE, 0644|IPC_CREAT)) < 0)
			{
				perror("msgget");
				exit(1);
			}
			/*put user into shm and size++*/
			user *a;
			if((a = shmat(shm_id,NULL,0)) < 0)
			{
				perror("shmat error");
			}
			int *i;
			if((i = shmat(shm_id2,NULL,0)) < 0)
			{
				perror("shmat error");
			}
			/*decide index of this user*/
			strcpy(a[*i].name, "user_");
			char str[128];
			sprintf(str, "%d", msqid);
			strcat(a[*i].name, str);
			/*Reply to the client*/
			//strcat(hello, a[0].name);
			char *hello = "Welcome to the chat server, ";
			write(new_socket, hello, strlen(hello));
			write(new_socket, a[*i].name, strlen(a[*i].name));
			write(new_socket, "\n", 1);
			a[*i].msqid = msqid;
			(*i)++;
			/*close shm*/
			shmdt(i);
			shmdt(a);

			pid_t child2 = fork();
			if (child2 < 0)
			{
				perror("fork error");
				return 1;
			}
			else if (child2 == 0)	{
			/*Read command*/
			char buf[128];
			write(new_socket, "$ ", strlen("$ "));

			while (read(new_socket, buf, 128) >= 0)
			{
				//write(new_socket, buf, strlen(buf));
				char *token;
				token = strtok(buf, DELIM);
				if(strcmp(token, "broadcast") == 0)
				{

					struct my_msgbuf msgbuf;
					user *a;
					a = shmat(shm_id,NULL,0);
					int *i;
					i = shmat(shm_id2,NULL,0);
					int j;
					for (j = 0; j < *i; j++)
					{
						if (msqid == a[j].msqid)
						{
							break;
						}
					}
					//printf("%d\n", msqid);
					strcpy(msgbuf.mtext, "<");
					strcat(msgbuf.mtext, a[j].name);
					strcat(msgbuf.mtext, "> said: ");
					token = strtok(NULL, "\n");
					strcat(msgbuf.mtext, token);
					write(new_socket, msgbuf.mtext, strlen(msgbuf.mtext));
					msgbuf.mtype = 1;
					/*to every one in shm*/
					for (j = 0; j < *i; j++)
					{
						if (msgsnd(a[j].msqid, (void *)&msgbuf, sizeof(struct my_msgbuf) - sizeof(long), 0) == -1)
						{
					            perror("msgsnd");
						}
					}
				}
				else if(strcmp(token, "send") == 0)
				{
					token = strtok(NULL, DELIM);
					user *a;
					a = shmat(shm_id,NULL,0);
					int *i;
					i = shmat(shm_id2,NULL,0);
					int talkto;
					for (talkto = 0; talkto < *i; talkto++)
					{
						if (strcmp(a[talkto].name,token) == 0)
						{
							write(new_socket, token, strlen(token));
							break;
						}
					}
					if(talkto == *i)
					{
						write(new_socket, "no this person here, try 'list' to check out\n$ ", strlen("\nno this people, try 'list' to check out\n$ "));
						continue;
					}

					struct my_msgbuf msgbuf;
					int j;
					for (j = 0; j < *i; j++)
					{
						if (msqid == a[j].msqid)
						{
							break;
						}
					}
					//printf("%d\n", msqid);
					strcpy(msgbuf.mtext, "<");
					strcat(msgbuf.mtext, a[j].name);
					strcat(msgbuf.mtext, "> said: ");
					token = strtok(NULL, "\n");
					strcat(msgbuf.mtext, token);
					msgbuf.mtype = 1;
					/*to every one in shm*/
					if (msgsnd(a[talkto].msqid, (void *)&msgbuf, sizeof(struct my_msgbuf) - sizeof(long), 0) == -1)
					{
						perror("msgsnd");
					}
				}
				else if(strcmp(token, "quit") == 0)
				{
					user *a;
					if((a = shmat(shm_id,NULL,0)) < 0)
					{
						perror("shmat error");
					}
					int *i;
					if((i = shmat(shm_id2,NULL,0)) < 0)
					{
						perror("shmat error");
					}
					int j;
					int find = -1;
					for (j = 0; j < *i; j++)
					{
						/*char is[64];
						sprintf(is, "%d", msqid);
						write(new_socket, is, strlen(is));*/
						if (msqid == a[j].msqid)
						{
							find = j;
						}
						if (find > -1 && find != j)
						{
							strcpy(a[find].name, a[j].name);
							a[find].msqid = a[j].msqid;
							find++;
						}
					}
					(*i)--;
					shmdt(i);
					shmdt(a);
					/*msgctl*/
					if (msgctl(msqid, IPC_RMID, NULL) == -1) {
						perror("msgctl");
						exit(1);
					}
					write(new_socket, "bye!\n", strlen("bye!\n"));
					close(new_socket);
				}
				else if(strcmp(token, "list") == 0)
				{
					user *a;
					if((a = shmat(shm_id,NULL,0)) < 0)
					{
						perror("shmat error");
					}
					int *i;
					if((i = shmat(shm_id2,NULL,0)) < 0)
					{
						perror("shmat error");
					}
					/*List all the users*/
					int j;
					write(new_socket, "----------\n", strlen("----------\n"));
					for (j = 0; j < *i; j++)
					{
						write(new_socket, a[j].name, strlen(a[j].name));
						write(new_socket, "\n", strlen("\n"));
					}
					write(new_socket, "----------\n", strlen("----------\n"));
					char tostr[128];
					sprintf(tostr, "%d", *i);
					write(new_socket, tostr, strlen(tostr));
					write(new_socket, " user(s) listed\n", strlen(" user(s) listed\n"));
					shmdt(i);
					shmdt(a);
				}
				else if(strcmp(token, "name") == 0)
				{
					token = strtok(NULL, DELIM);
					user *a;
					if((a = shmat(shm_id,NULL,0)) < 0)
					{
						perror("shmat error");
					}
					int *i;
					if((i = shmat(shm_id2,NULL,0)) < 0)
					{
						perror("shmat error");
					}
					int flag = 0;
					int j;
					for (j = 0; j < *i; j++)
					{
						if (strcmp(token, a[j].name) == 0)
						{
							flag = 1;
						}
					}
					if(flag == 1)
					{
						write(new_socket, "this name is used, try another one\n$ ", strlen("this name is used, try another one\n$ "));
						continue;
					}
					for (j = 0; j < *i; j++)
					{
						if (msqid == a[j].msqid)
						{
							strcpy(a[j].name,token);
							break;
						}
					}
					shmdt(i);
					shmdt(a);
				}
				else
				{
					write(new_socket, "supported command: list, name, send, broadcast, quit\n", strlen("supported command: list, name, send, broadcast, quit\n"));
				}
				write(new_socket, "$ ", strlen("$ "));
			}
			} else
				{
					struct my_msgbuf msgbuf;
					for(;;) { /* rcver never quits! */
						sleep(1);
						if (msgrcv(msqid, &msgbuf, sizeof(msgbuf.mtext), 0, 0) == -1) {
						    exit(1);
						}
						write(new_socket, msgbuf.mtext, strlen(msgbuf.mtext));
						write(new_socket, "\n$ ", strlen("\n$ "));
					}
				}
		}
		else /*parent*/
		{
			close(new_socket);
			sleep(1);
			user* b;
			if((b = shmat(shm_id,NULL,0)) < 0)
			{
				perror("shmat error");
			}
			//printf("%s\n", b[0].name);
			shmdt(b);
			int *i;
			if((i = shmat(shm_id2,NULL,0)) < 0)
			{
				perror("shmat error");
			}
			//printf("%d\n", *i);
			//printf("parent\n");
			
			continue;
		}
	}
	
	if (new_socket < 0)
	{
		perror("accept failed");
		return 1;
	}
 
	return 0;
}

