#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

#define ERR_EXIT(m)         \
	do                      \
	{                       \
		perror(m);          \
		exit(EXIT_FAILURE); \
	} while (0);


int getlocalip(char *ip)
{
    char host[100] = {0};
    if(gethostname(host, sizeof(host)) < 0)
    {
        return -1;
    }

    struct hostent *ht;
    ht = gethostbyname(host);
    if(ht == NULL)
        return -1;
    
    strcpy(ip, inet_ntoa(*(struct in_addr*)ht->h_addr));
    return 0;
}

int main(void)
{

    char ip[16] = {0};
    getlocalip(ip);
    printf("%s\n", ip);
    
    return 0;
}