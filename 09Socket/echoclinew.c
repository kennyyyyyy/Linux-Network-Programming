#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<stdlib.h>
#include<errno.h>
#include <string.h>

#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);

struct packet
{
	int len;
	char buf[1024];
};


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


int main(void)
{
	int sock;
	if((sock = socket(AF_INET, SOCK_STREAM, 0) )< 0 )
	{
		ERR_EXIT("socket");
	}
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(6666);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	
	if(connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr) )< 0)
		ERR_EXIT("Connect");

	struct packet sendbuf;
	struct packet recvbuf;
	memset(&recvbuf, 0, sizeof(recvbuf));
	memset(&sendbuf, 0, sizeof(sendbuf));
	int len;
	while(fgets(sendbuf.buf, sizeof(sendbuf.buf), stdin) != NULL)
	{
		len = strlen(sendbuf.buf);
		sendbuf.len = htonl(len);
		written(sock, &sendbuf, 4 + len);

        int ret = readn(sock, &recvbuf.len, 4);
        if(ret == -1)
        {
            ERR_EXIT("read");
        }
        else if(ret < 4)
        {
            printf("Client closed.\n");
            break;
        }
        len = ntohl(recvbuf.len);
        ret = readn(sock, recvbuf.buf, len);
        if(ret == -1)
        {
            ERR_EXIT("read");
        }
        else if(ret < len)
        {
            printf("Client closed.\n");
            break;
        }

		fputs(recvbuf.buf, stdout);

		memset(&recvbuf, 0, sizeof(recvbuf));
		memset(&sendbuf, 0, sizeof(sendbuf));
	}
	close(sock);
	return 0;
}
