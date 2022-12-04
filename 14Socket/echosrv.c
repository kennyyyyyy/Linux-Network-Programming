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
#include <sys/wait.h>
#include <sys/select.h>

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
	char *bufp = (char*) buf;

	while(nleft > 0)
	{
		nread = read(fd, bufp, nleft);
		if(nread < 0)
		{
			if(errno == EINTR)
				continue;
			return -1;
		}
		else if(nread == 0)
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
	char *bufp = (char*) buf;

	while(nleft > 0)
	{
		nwritten = write(fd, bufp, nleft);
		if(nwritten < 0)
		{
			if(errno == EINTR)
				continue;
			return -1;
		}
		else if(nwritten == 0)
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
    while(1)
    {
        int ret = recv(sockfd, buf, len, MSG_PEEK);
        if(ret == -1 & errno == EINTR)
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
    while(1)
    {
        ret = recv_peek(sockfd, bufp, nleft);
        if(ret <= 0)
        {
            return ret;
        }

        nread = ret;
        //
        int i;
        for(i = 0; i < nread; i++)
        {
            if(bufp[i] == '\n')
            {
                ret = readn(sockfd, bufp, i + 1);
                if(ret != i + 1)
                {
                    exit(EXIT_FAILURE);
                }
                return ret;
            }
        }

        if(nread > nleft)
        {
            exit(EXIT_FAILURE);
        }

        //
        nleft -= nread;
        ret = readn(sockfd, bufp, nread);
        if(ret != nread)
        {
            exit(EXIT_FAILURE);
        }    
        bufp += nread;
        nleft -= nread;
    }
    return -1;
}

void echo_srv(int conn)
{
    char recvbuf[1024];
    int len;
    while (1)
    {
        memset(recvbuf, 0, sizeof(recvbuf));
        int ret = readline(conn, recvbuf, sizeof(recvbuf));
        if(ret == -1)
        {
            ERR_EXIT("read");
        }
        else if(ret == 0)
        {
            printf("Client closed.\n");
            break;
        }

        fputs(recvbuf, stdout);
        written(conn, recvbuf, strlen(recvbuf));
    }
}

//
void handle_sigchld(int sig)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_sigpipe(int sig)
{
    printf("recv sign %d", sig);
}

int main(void)
{
    //signal(SIGCHLD, SIG_IGN);
	signal(SIGCHLD, handle_sigchld);
    signal(SIGPIPE, handle_sigpipe);

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
    socklen_t peerlen;
    int conn;

    int i;
    int maxidx = 0;
    int client[FD_SETSIZE];
    for(i = 0; i < FD_SETSIZE; i++)
    {
        client[i] = -1;
    }

    int nready;
    int maxfd = listenfd;
    fd_set rset;
    fd_set allset;
    FD_ZERO(&rset);
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    while(1)
    {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if(nready == -1)
        {
            if(errno == EINTR)
                continue;
            ERR_EXIT("select");
        }
        if(nready == 0)
            continue;
        if(FD_ISSET(listenfd, &rset))
        {
            peerlen = sizeof(peeraddr);
            if ((conn = accept(listenfd, (struct sockaddr *)&peeraddr, &peerlen)) < 0)
                ERR_EXIT("accept");
            

            for(i = 0; i < FD_SETSIZE; i++)
            {
                if(client[i] == -1)
                {
                    client[i] = conn;
                    maxidx = i > maxidx ? i : maxidx;
                    break;
                }
            }

            if(i == FD_SETSIZE)
            {
                fprintf(stderr, "too many clients\n");
                exit(EXIT_FAILURE);
            }

            FD_SET(conn, &allset);
            maxfd = conn > maxfd ? conn : maxfd;
            printf("ip = %s port = %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

            if(--nready <= 0)
                continue;
        }

        //
        for(i = 0; i <= maxidx; i++)
        {
            conn = client[i];
            if(conn == -1)
                continue;
            if(FD_ISSET(conn, &rset))
            {
                char recvbuf[1024] = {0};
                int ret = readline(conn, recvbuf, 1024);
                if(ret == -1)
                {
                    ERR_EXIT("client readline");
                }
                if(ret == 0)
                {
                    printf("client %d close\n", conn);
                    FD_CLR(conn, &allset);
                    client[i] = -1;
                }

                fputs(recvbuf, stdout);
                written(conn, recvbuf, strlen(recvbuf));

                if(--nready <= 0)
                    break;
            }
        }
    }

    return 0;
}