#include "head.h"

int main(int argc, const char *argv[])
{
	int sockfd;
	struct sockaddr_in seraddr,
					   cltaddr;
	char *buff = NULL;
	int ret;

	buff = (char *)malloc(BUFF_SIZE);

	if (-1 == (sockfd = socket(AF_INET, SOCK_DGRAM, 0)))
		error_exit("socket");
#if 0
	/* not necessary*/
	cltaddr.sin_family = AF_INET;
	cltaddr.sin_port = htons(CLT_PORT);
	cltaddr.sin_addr.s_addr = inet_addr(CLT_ADDR);
	if (-1 == bind(sockfd, (struct sockaddr *)&cltaddr, sizeof(cltaddr)))
		error_exit("bind");
#endif
	
	strcpy(buff, "hellowrold");
	
    /* your can use connect on UDP */
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(SER_PORT);
	seraddr.sin_addr.s_addr = inet_addr(SER_ADDR);
	
	/* ordinary method */
	if (-1 == sendto(sockfd, buff, BUFF_SIZE, 0,
			(struct sockaddr *)&seraddr, sizeof(seraddr)))
		error_exit("sendto");
#if 0
	if (-1 == (ret = recvfrom(sockfd, buff, BUFF_SIZE, 0,
					NULL, NULL)))
		error_exit("recvfrom");
	printf("[%d]:%s\n", ret, buff);
#endif	

	close(sockfd);
	free(buff);

	return 0;
}
