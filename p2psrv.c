#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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
	//创建套接字
	int listenfd;
	if ((listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		ERR_EXIT("socket");
	}

	//创建套接字地址
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(6666);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	// servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	// inet_aton()"127.0.0.1", &servaddr.sin_addr);

	// set REUSEADDR opt
	int on = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		ERR_EXIT("setsocektopt");
	//绑定地址
	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("bind");

	//监听，等待连接请求
	if (listen(listenfd, SOMAXCONN) < 0)
		ERR_EXIT("listen");

	struct sockaddr_in peeraddr;
	socklen_t peerlen = sizeof(peeraddr);
	int conn;
	//从已完成连接中会的第一个连接，若无连接，则阻塞（被动套接字）
	if ((conn = accept(listenfd, (struct sockaddr *)&peeraddr, &peerlen)) < 0)
		ERR_EXIT("accept");

	pid_t pid;
	pid = fork();
	if(pid == -1)
		ERR_EXIT("fork");

	if(pid == 0)
	{
		signal(SIGUSR1, handler);
		char sendbuf[1024] = {0};
		while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
		{
			write(conn, sendbuf, strlen(sendbuf));

			memset(sendbuf, 0, sizeof(sendbuf));
		}
		exit(EXIT_SUCCESS);
	}
	else
	{
		char recvbud[1024];
		while (1)
		{
			memset(recvbud, 0, sizeof(recvbud));
			int ret = read(conn, recvbud, sizeof(recvbud));
			
			if(ret == -1)
			{
				ERR_EXIT("read");
			}
			else if(ret == 0)
			{
				printf("peer closed\n");
				break;
			}
			fputs(recvbud, stdout);
		}
		kill(pid, SIGUSR1);
		exit(EXIT_SUCCESS);
	}

	//断开连接
	close(conn);
	close(listenfd);
	return 0;
}