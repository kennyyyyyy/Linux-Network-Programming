#include "includes.h"

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
	char *bufp = (char *)buf;
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
