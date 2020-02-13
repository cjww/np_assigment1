#include <stdio.h>
#include <stdlib.h>
/* You will to add includes here */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>

// Included to get the support library
#include <calcLib.h>

int main(int argc, char *argv[]){
  
  /* Do magic */
  if(argc < 3){
    fprintf(stderr, "usage: %s <hostname> <port>", argv[0]);
  }

  initCalcLib();

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd == -1){
    fprintf(stderr, "Failed to create socket\n");
    return 1;
  }

  short port = atoi(argv[2]);
  const char* paddr = argv[1];
  hostent* serverEnt = gethostbyname(paddr);
  if(serverEnt == NULL) {
    fprintf(stderr, "No host : %s\n", paddr);
    return 2;
  }
  sockaddr_in serverAddr;
  socklen_t serverAddrlen = sizeof(serverAddr);
  memset(&serverAddr, 0, serverAddrlen);
  inet_pton(AF_INET, paddr, &serverAddr.sin_addr);
  serverAddr.sin_port = htons(port);
  serverAddr.sin_family = AF_INET;

  int res = connect(sockfd, (sockaddr*)&serverAddr, serverAddrlen);
  if(res != 0){
    getsockname(sockfd, (sockaddr*)&serverAddr, &serverAddrlen);
    char buffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, (sockaddr*)&serverAddr, buffer, sizeof(buffer));
    fprintf(stderr, "Failed to connect to %s:%d\n", buffer, ntohs(serverAddr.sin_port));
    return 3;
  }

  shutdown(sockfd, SHUT_RDWR);
  return 0;
}
