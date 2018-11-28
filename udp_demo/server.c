#include "head.h"

int main()
{
	int sockfd;
	struct sockaddr_in seraddr,
					   cltaddr;
	int ret;
	char *buff = NULL;
	socklen_t addrlen = sizeof(cltaddr);

	buff = (char *)malloc(BUFF_SIZE);

	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(SER_PORT);
	/* bind ip address is not necessary */
	seraddr.sin_addr.s_addr = 0;//inet_addr(SER_ADDR);

	if (-1 == (sockfd = socket(AF_INET, SOCK_DGRAM, 0)))
		error_exit("socket");
	if (-1 == bind(sockfd, (struct sockaddr *)&seraddr, sizeof(seraddr)))
		error_exit("bind");
	
    while (1)
    {
        if (-1 == (ret = recvfrom(sockfd, buff, BUFF_SIZE, 0,
                        (struct sockaddr *)&cltaddr, &addrlen)))
            error_exit("recvfrom");

        printf("recvfrom: %s:%d\n", 
                inet_ntoa(cltaddr.sin_addr), ntohs(cltaddr.sin_port));
        printf("[%d]:%s\n", ret, buff);

        strcat(buff, "---------echo");
        if (-1 == sendto(sockfd, buff, strlen(buff)+1, 0,
                    (struct sockaddr *)&cltaddr, sizeof(cltaddr)))
            error_exit("sendto");
    }
	close(sockfd);
	free(buff);
	
	return 0;
}
