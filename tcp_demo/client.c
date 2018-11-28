#include "head.h"

int main()
{
	int sockfd;
	struct sockaddr_in seraddr,
					   cltaddr;
	char *buff = NULL;

	buff = (char *)malloc(BUFF_SIZE);

	
	if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0)))
		error_exit("socket");
	
/* bind client`s address/port is not necessary */
	cltaddr.sin_family = AF_INET;
	cltaddr.sin_port = htons(CLT_PORT);
	cltaddr.sin_addr.s_addr = inet_addr(CLT_ADDR);
	//if (-1 == bind(sockfd, (struct sockaddr *)&cltaddr, sizeof(cltaddr)))
	//	error_exit("bind");
	
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(SER_PORT);
	seraddr.sin_addr.s_addr = inet_addr(SER_ADDR);
	if (-1 == connect(sockfd, (struct sockaddr *)&seraddr, sizeof(seraddr)))
		error_exit("connect");

    //while (1)
    {
        strcpy(buff, "helloworld");
        if (-1 == send(sockfd, buff, strlen(buff)+1, 0))
            error_exit("send");
#if 0
        memset(buff, 0, BUFF_SIZE);
        if (-1 == recv(sockfd, buff, BUFF_SIZE, 0))
            error_exit("recv");

        printf("recv: %s\n", buff);
        usleep(1000);
#endif
    }
	close(sockfd);
	free(buff);
	
	return 0;
}
