#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define ERR_EXIT(m)         \
	do                      \
	{                       \
		perror(m);          \
		exit(EXIT_FAILURE); \
	} while (0);

void handler(int sign)
{
	printf("received a sign = %d\n", sign);
	exit(EXIT_SUCCESS);
}

int main(void)
{
	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		ERR_EXIT("socket");
	}
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(6666);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("Connect");

	pid_t pid;
	pid = fork();
	if (pid == -1)
		ERR_EXIT("fork");

	if (pid == 0)
	{

		char recvbud[1024];
		while (1)
		{
			memset(recvbud, 0, sizeof(recvbud));
			int ret = read(sock, recvbud, sizeof(recvbud));

			if (ret == -1)
			{
				ERR_EXIT("read");
			}
			else if (ret == 0)
			{
				printf("peer closed\n");
				break;
			}
			fputs(recvbud, stdout);
		}
		kill(pid, SIGUSR1);
		exit(EXIT_SUCCESS);
	}
	else
	{
		signal(SIGUSR1, handler);
		char sendbuf[1024] = {0};
		while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
		{
			write(sock, sendbuf, strlen(sendbuf));

			memset(sendbuf, 0, sizeof(sendbuf));
		}
		exit(EXIT_SUCCESS);
	}

	close(sock);
	return 0;
}
