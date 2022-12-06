#include "includes.h"

int read_timeout(int fd, unsigned int wait_seconds)
{
    int ret = 0;
    if(wait_seconds > 0)
    {
        fd_set read_fdset;
        struct timeval timeout;
        
        FD_ZERO(&read_fdset);
        FD_SET(fd, &read_fdset);

        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        do
        {
            ret = select(fd + 1, &read_fdset, NULL, NULL, &timeout);
        }
        while(ret < 0 && errno == EINTR);

        if(ret == 0)
        {
            ret = -1;
            errno = ETIMEDOUT;
        }
        else if(ret == 1)
            ret = 0;
    }
    return ret;
}

int write_timeout(int fd, unsigned int wait_seconds)
{
    int ret = 0;
    if(wait_seconds > 0)
    {
        fd_set write_fdset;
        struct timeval timeout;
        
        FD_ZERO(&write_fdset);
        FD_SET(fd, &write_fdset);

        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        do
        {
            ret = select(fd + 1, NULL, &write_fdset, NULL, &timeout);
        }
        while(ret < 0 && errno == EINTR);

        if(ret == 0)
        {
            ret = -1;
            errno = ETIMEDOUT;
        }
        else if(ret == 1)
            ret = 0;
    }
    return ret;
}


int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
    int ret = 0;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    if(wait_seconds > 0)
    {
        fd_set accept_fdset;
        struct timeval timeout;

        FD_ZERO(&accept_fdset);
        FD_SET(fd, &accept_fdset);

        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;

        do
        {
            ret = select(fd + 1, &accept_fdset, NULL, NULL, &timeout);
        }
        while(ret < 0 && errno == EINTR);

        if(ret == 0)
        {
            errno = ETIMEDOUT;
            return -1;
        }
        else if(ret == -1)
            return -1;

        if(addr != NULL)
        {
            ret = accept(fd, (struct sockaddr*)addr, &addrlen);
        }
        else
        {
            ret = accept(fd, NULL, NULL);
        }
        if(ret == -1)
            ERR_EXIT("accept");
    }
    return ret;
}

//1 for active
//0 for enable
void set_nonblock(int fd, int set)
{
    int ret;
    int flags = fcntl(fd, F_GETFL);
    if(flags == -1)
    {
        ERR_EXIT("fcntl");
    }

    if(set)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    ret = fcntl(fd, F_SETFL, flags);
    if(ret == -1)
    {
        ERR_EXIT("fcntl");
    }
}

int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
    int ret;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    if(wait_seconds > 0)
    {
        set_nonblock(fd, 1);
    }

    ret = connect(fd, (struct  sockaddr*) addr, addrlen);
    if(ret < 0 && errno == EINPROGRESS)
    {
        fd_set connect_fdset;
        struct timeval timeout;

        FD_ZERO(&connect_fdset);
        FD_SET(fd, &connect_fdset);

        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;

        do
        {
            ret = select(fd + 1, NULL, &connect_fdset, NULL, &timeout);
        }
        while(ret < 0 && errno == EINTR);

        if(ret == 0)
        {
            ret = -1;
            errno = ETIMEDOUT;
        }
        else if(ret < 0)
        {
            return -1;
        }
        else if(ret == 1)
        {
            //
            int err;
            socklen_t socklen = sizeof(err);
            int sockoptret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &socklen);
            if(sockoptret == -1)
            {
                return -1;
            }
            if(err == 0)
            {
                ret = 0;
            }
            else 
            {
                errno = err;
                ret = -1;
            }
        }    
        
    }
    if(wait_seconds > 0)
    {
        set_nonblock(fd, 0);
    }
    return ret;
}