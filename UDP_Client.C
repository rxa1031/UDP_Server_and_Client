#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h> /* Required for socket functions like socket(), connect(), etc. */
#include <sys/types.h>
#include <unistd.h> /* Required for close() */
#include <netdb.h>

int main()
{
	struct addrinfo *host = (struct addrinfo *)NULL;
	int iSocketFieldDescription = -1;
	int r,x;
	const int size = 1024;
	char input[size],buffer[size];

	do
	{
		struct addrinfo hints = {
			.ai_flags = AI_PASSIVE, /* earlier set to 0 */
			/* Allow IPv4 or IPv6 */
			.ai_family = AF_UNSPEC,
			.ai_socktype = SOCK_DGRAM,
			.ai_protocol = 0,
			.ai_addrlen = 0,
			.ai_addr = NULL,
			.ai_canonname = NULL,
			.ai_next = NULL
		};
		int iGetAddInfoErrCode = getaddrinfo((const char *)NULL, (const char *)"8080", &hints, &host);
		if (0 != iGetAddInfoErrCode)
		{
			perror(gai_strerror(iGetAddInfoErrCode));
			break;
		}

		/* create the socket */
    	struct addrinfo *ptrAddrInfo = host;
	    for (; ptrAddrInfo != NULL; ptrAddrInfo = ptrAddrInfo->ai_next)
	    {
			iSocketFieldDescription = socket(host->ai_family,host->ai_socktype,host->ai_protocol);
			if (-1 == iSocketFieldDescription)
			{ 	
		       fprintf(stderr, "%s Socket open failed!\n",
		                host->ai_family == AF_INET
		                    ? "IPV4"
		                    : (host->ai_family == AF_INET6 ? "IPV6" : "UNKNOWN"));
		        perror(strerror(errno));
		        continue;
			}
			break;
		}
		/* host information is needed when calling sendto() and recvfrom() */
		//freeaddrinfo(host);
		//host = NULL;
		/* No address succeeded */
		if (NULL == ptrAddrInfo)
		{
		  fprintf(stderr, "Socket creation failed...\n");
		  // exit(EXIT_FAILURE);
		  break;
		}

		/* prompt for input */
		printf("Type a string: ");
		fgets(input,size,stdin);
		/* remove the newline */
		for( x = 0; x < size; x++ )
		{
			if( input[x]=='\n' )		/* replace the newline */
			{
				input[x] = '\0';		/* with the null char */
				break;
			}
		}
		const int iRcvdMsgLen = strlen(input);
		fprintf(stdout, "Rcvd Msg Len %d\n", iRcvdMsgLen);
		fprintf(stdout, "Socket Field Description %d\n", iSocketFieldDescription);

		/* send the string to the server */
		r = sendto( iSocketFieldDescription, input, iRcvdMsgLen, 0, host->ai_addr, host->ai_addrlen );

		fprintf(stdout, "Message sent: %s\n", input);

		r = recvfrom( iSocketFieldDescription, buffer, size, 0, host->ai_addr, &host->ai_addrlen );
		buffer[r] = '\0';
		printf("%s\n",buffer);
	} while(false);

	/* all done, clean-up */
	if( NULL != host)
	{
		fprintf(stdout, "Freeing Client Address Information\n");
		freeaddrinfo(host);
	}
	if (-1 != iSocketFieldDescription)
	{
		close(iSocketFieldDescription);
	}

	return EXIT_SUCCESS;
}


