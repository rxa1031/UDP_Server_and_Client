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

struct addrinfo *server = (struct addrinfo *)NULL;
int iSocketFieldDescription = -1;

void Close(void)
{
  if (NULL != server)
  {
    fprintf(stdout, "Freeing Server Address Information...\n");
    freeaddrinfo(server);
  }
  if (-1 != iSocketFieldDescription)
  {
    fprintf(stdout, "Closing Socket...\n");
    close(iSocketFieldDescription);
  }
}

void CleanupAndExit(int signum)
{
  printf("\nCaught signal %d. Cleaning up...\n", signum);
  Close();
  exit(0);
}

int main()
{
  do
  {
    struct addrinfo hints = {
        .ai_flags = AI_PASSIVE,
        /* Allow IPv4 or IPv6 */ /* Ealier set to AF_INET to use IPV4 */
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_DGRAM,
        .ai_protocol = 0,
        .ai_addrlen = 0,
        .ai_addr = NULL,
        .ai_canonname = NULL,
        .ai_next = NULL
      };
    int iGetAddInfoErrCode = getaddrinfo((const char *)NULL, (const char *)"8080", &hints, &server);
    if (0 != iGetAddInfoErrCode)
    {
      perror(gai_strerror(iGetAddInfoErrCode));
      break;
    }
    struct addrinfo *ptrAddrInfo = server;
    /* getaddrinfo() returns a list of address structures.
      Try each address until we successfully bind(2).
      If socket(2)fails we try the next address.
      If bind(2) fails, we close the socket
      and try the next address. */
    for (; ptrAddrInfo != NULL; ptrAddrInfo = ptrAddrInfo->ai_next)
    {
      iSocketFieldDescription = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
      if (-1 == iSocketFieldDescription)
      {
        fprintf(stderr, "%s Socket open failed!\n",
                server->ai_family == AF_INET
                    ? "IPV4"
                    : (server->ai_family == AF_INET6 ? "IPV6" : "UNKNOWN"));
        perror(strerror(errno));
        continue;
      }
      if (bind(iSocketFieldDescription, ptrAddrInfo->ai_addr,
               ptrAddrInfo->ai_addrlen) == 0)
      {
        break; /* Success */
      }
      else
      {
        fprintf(stderr, "Bind failed with error %s\n", strerror(errno));
      }
      close(iSocketFieldDescription);
    }
    /* No longer needed, hence closing */
    freeaddrinfo(server);
    server = NULL;
    /* No address succeeded */
    if (NULL == ptrAddrInfo)
    {
      fprintf(stderr, "Could not bind\n");
      // exit(EXIT_FAILURE);
      break;
    }
    // Catch termination signals
    signal(SIGINT, CleanupAndExit);   // Ctrl+C
    signal(SIGTERM, CleanupAndExit);  // kill command
    signal(SIGQUIT, CleanupAndExit);  // Ctrl+\

    // Catch Ctrl+Z (SIGTSTP)
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = CleanupAndExit;
    sigaction(SIGTSTP, &sa, NULL);
  
    fprintf(stdout, "\nUDP server is listening...\n\n");

    const int iBufSz = 512;
    char cBuf[iBufSz];
    struct sockaddr PeerAddr;
    socklen_t PeerAddrLen = sizeof(PeerAddr);
    ssize_t PeerMsgLen = recvfrom(iSocketFieldDescription, cBuf, iBufSz, 0, &PeerAddr, &PeerAddrLen);
    if (-1 == PeerMsgLen)
    {
      perror("Message receive failed!\n");
      break;
    }
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];
    int status = getnameinfo(&PeerAddr, PeerAddrLen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICSERV);

    if (0 == status)
    {
      printf("Received %zd bytes from %s:%s\n", PeerMsgLen, host, serv);
    }
    else
    {
      fprintf(stderr, "getnameinfo: %s\n", gai_strerror(status));
    }

    if (PeerMsgLen != sendto(iSocketFieldDescription, cBuf, PeerMsgLen, 0, &PeerAddr, PeerAddrLen))
    {
      fprintf(stderr, "Error sending response\n");
    }

  } while (false);
  CleanupAndExit(0);
  return EXIT_SUCCESS;
}
