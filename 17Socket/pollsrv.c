#include "../headers/datamgr.h"

#define POLLSIZE 1024

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
    struct pollfd client[POLLSIZE];
    for(i = 0; i < POLLSIZE; i++)
    {
        client[i].fd = -1;
    }

    int nready;
    client[0].fd = listenfd;
    client[0].events = POLLIN;

    while(1)
    {
        nready = poll(client, maxidx + 1, -1);
        if(nready == -1)
        {
            if(errno == EINTR)
                continue;
            ERR_EXIT("poll");
        }
        if(nready == 0)
            continue;

        //
        if(client[0].revents & POLLIN)
        {
            peerlen = sizeof(peeraddr);
            if ((conn = accept(listenfd, (struct sockaddr *)&peeraddr, &peerlen)) < 0)
                ERR_EXIT("accept");
            

            for(i = 0; i < POLLSIZE; i++)
            {
                if(client[i].fd == -1)
                {
                    client[i].fd = conn;
                    client[i].events = POLLIN;
                    maxidx = i > maxidx ? i : maxidx;
                    break;
                }
            }

            if(i == POLLSIZE)
            {
                fprintf(stderr, "too many clients\n");
                exit(EXIT_FAILURE);
            }

            printf("ip = %s port = %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

            if(--nready <= 0)
                continue;
        }

        //
        for(i = 1; i < POLLSIZE; i++)
        {  
            conn = client[i].fd;
            if(conn == -1)
                continue;
            if(client[i].revents & POLLIN)
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
                    client[i].fd = -1;
                    close(conn);
                }

                fputs(recvbuf, stdout);
                // sleep(3);
                written(conn, recvbuf, strlen(recvbuf));

                if(--nready <= 0)
                    break;
            }
        }
    }

    return 0;
}