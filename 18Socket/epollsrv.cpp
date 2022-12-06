#include "../headers/includescpp.h"
#include "../headers/datamgr.h"
#include "../headers/tiout.h"

#define POLLSIZE 1024

typedef vector<struct epoll_event> EventList;


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

    //
    vector<int> clients;
    int epollfd;
    epollfd = epoll_create1(EPOLL_CLOEXEC);
    
    //
    struct epoll_event event;
    event.data.fd = listenfd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event);

    //
    EventList events(16);
    struct sockaddr_in peeraddr;
    socklen_t peerlen;

    int i;
    int conn;
    int nready;
    int count = 0;
    while(1)
    {
        nready = epoll_wait(epollfd, &*events.begin(), static_cast<int>(events.size()), -1);

        if(nready == -1)
        {
            if(errno == EINTR)
                continue;
            ERR_EXIT("epoll_wait");
        }
        if(nready == 0)
            continue;

        if((size_t)nready == events.size())
        {
            events.resize(events.size() * 2);
        }

        for(i = 0; i < nready; i++)
        {
            if(events[i].data.fd == listenfd)
            {
                peerlen = sizeof(peeraddr);
                if ((conn = accept(listenfd, (struct sockaddr *)&peeraddr, &peerlen)) < 0)
                    ERR_EXIT("accept");

                printf("ip = %s port = %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
                printf("count = %d\n", ++count);
                clients.push_back(conn);

                set_nonblock(conn, 1);

                event.data.fd = conn;
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, conn, &event);

            }
            else if(events[i].events & EPOLLIN)
            {
                conn = events[i].data.fd;
                if(conn == -1)
                    continue;
                if(events[i].events & POLLIN)
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
                        close(conn);

                        event = events[i];
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, conn, &event);
                        clients.erase(remove(clients.begin(), clients.end(), conn), clients.end());
                    }

                    fputs(recvbuf, stdout);
                    written(conn, recvbuf, strlen(recvbuf));
                }
            }
        }

    }

    return 0;
}