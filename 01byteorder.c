#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(void)
{
	unsigned int val = 0x12345678;
	unsigned char *p = (unsigned char*)&val;
	printf("%0x %0x %0x %0x\n", p[0], p[1], p[2], p[3]);

	unsigned int val2 = htonl(val);
	p = (unsigned char*)&val2;
	printf("%0x %0x %0x %0x\n", p[0], p[1], p[2], p[3]);
	return 0;
}
