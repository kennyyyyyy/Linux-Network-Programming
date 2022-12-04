#include "tiout.h"
#include "datamgr.h"


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
    printf("recv sign %d\n", sig);
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

	printf("ip = %s port = %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

	//断开连接
	close(conn);
	close(listenfd);
	return 0;
}