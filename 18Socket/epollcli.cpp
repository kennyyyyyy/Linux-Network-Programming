#include "../headers/datamgr.h"



void echo_cli(int sock)
{

	fd_set rset;
	FD_ZERO(&rset);

	int nready;
	int maxfd;
	int fd_stdin = fileno(stdin);
	maxfd = fd_stdin > sock ? fd_stdin : sock;

	char sendbuf[1204] = {0};
	char recvbuf[1024] = {0};

	int stdineof = 0;

	while(1)
	{
		FD_SET(fd_stdin, &rset);
		FD_SET(sock, &rset);
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

		if(nready == -1)
		{
			ERR_EXIT("select");
		}
		else if(nready == 0)
		{
			continue;
		}
		if(FD_ISSET(sock, &rset))
		{
			int ret = readline(sock, recvbuf, sizeof(recvbuf));
			if (ret == -1)
			{
				ERR_EXIT("read");
			}
			else if (ret == 0)
			{
				printf("server closed.\n");
				break;
			}

			fputs(recvbuf, stdout);
			memset(recvbuf, 0, sizeof(recvbuf));
		}
		if(FD_ISSET(fd_stdin, &rset))
		{
			if(fgets(sendbuf, sizeof(sendbuf), stdin) == NULL)
			{
				stdineof = 1;
				shutdown(sock, SHUT_WR);
			}
			else if(stdineof == 0)
			{
				written(sock, sendbuf, strlen(sendbuf));
				memset(sendbuf, 0, sizeof(sendbuf));
			}

		}
	}
	//close(sock);
}

int main(void)
{
	//signal(SIGCHLD, SIG_IGN);
	
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

	struct sockaddr_in loacladdr;
	socklen_t addrlen = sizeof(loacladdr);
	if (getsockname(sock, (struct sockaddr *)&loacladdr, &addrlen) < 0)
		ERR_EXIT("getsockname");

	printf("ip = %s port = %d\n", inet_ntoa(loacladdr.sin_addr), ntohs(loacladdr.sin_port));

	echo_cli(sock);
	return 0;
}
