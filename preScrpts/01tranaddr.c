#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(void)
{
	const char* cp = "192.168.0.1";
	unsigned long num = inet_addr(cp);
	printf("%u\n", ntohl(num));
	return 0;
}
