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

ssize_t readn(int fd, void *buf, size_t count)
{
	size_t nleft = count;
	ssize_t nread;
	char *bufp = (char *)buf;

	while (nleft > 0)
	{
		nread = read(fd, bufp, nleft);
		if (nread < 0)
		{
			if (errno == EINTR)
				continue;
			return -1;
		}
		else if (nread == 0)
		{
			return count - nleft;
		}
		else
		{
			bufp += nread;
			nleft -= nread;
		}
	}
	return count;
}

ssize_t written(int fd, const void *buf, size_t count)
{
	size_t nleft = count;
	ssize_t nwritten;
	char *bufp = (char *)buf;

	while (nleft > 0)
	{
		nwritten = write(fd, bufp, nleft);
		if (nwritten < 0)
		{
			if (errno == EINTR)
				continue;
			return -1;
		}
		else if (nwritten == 0)
		{
			continue;
		}
		else
		{
			bufp += nwritten;
			nleft -= nwritten;
		}
	}
	return count;
}

ssize_t recv_peek(int sockfd, void *buf, size_t len)
{
	while (1)
	{
		int ret = recv(sockfd, buf, len, MSG_PEEK);
		if (ret == -1 & errno == EINTR)
			continue;
		return ret;
	}
}

ssize_t readline(int sockfd, void *buf, size_t maxline)
{
	int ret;
	int nread;
	char *bufp = buf;
	int nleft = maxline;
	while (1)
	{
		ret = recv_peek(sockfd, bufp, nleft);
		if (ret <= 0)
		{
			return ret;
		}

		nread = ret;
		//
		int i;
		for (i = 0; i < nread; i++)
		{
			if (bufp[i] == '\n')
			{
				ret = readn(sockfd, bufp, i + 1);
				if (ret != i + 1)
				{
					exit(EXIT_FAILURE);
				}
				return ret;
			}
		}

		if (nread > nleft)
		{
			exit(EXIT_FAILURE);
		}

		//
		nleft -= nread;
		ret = readn(sockfd, bufp, nread);
		if (ret != nread)
		{
			exit(EXIT_FAILURE);
		}
		bufp += nread;
		nleft -= nread;
	}
	return -1;
}

void echo_cli(int sock)
{
	char sendbuf[1204] = {0};
	char recvbuf[1024] = {0};
	while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
	{
		written(sock, sendbuf, strlen(sendbuf));

		int ret = readline(sock, recvbuf, sizeof(recvbuf));
		if (ret == -1)
		{
			ERR_EXIT("read");
		}
		else if (ret == 0)
		{
			printf("Client closed.\n");
			break;
		}

		fputs(recvbuf, stdout);

		memset(recvbuf, 0, sizeof(recvbuf));
		memset(sendbuf, 0, sizeof(sendbuf));
	}
	close(sock);
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
