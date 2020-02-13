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


template<typename T>
T calc(char* cmd, T v1, T v2){
  if(memcmp(cmd, "add", 3) == 0) {
    return v1 + v2;
  }
  else if(memcmp(cmd, "sub", 3) == 0) {
    return v1 - v2;
  }
  else if(memcmp(cmd, "mul", 3) == 0) {
    return v1 * v2;
  }
  else if(memcmp(cmd, "div", 3) == 0) {
    return v1 / v2;
  }
  return (T)0;
}

int main(int argc, char *argv[]) {
  
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

  //Receive protocol
  char recvBuffer[256];
  read(sockfd, recvBuffer, sizeof(recvBuffer));
  if(strcmp(recvBuffer, "TEXT TCP 1.0") != 0){
    fprintf(stderr, "Protocol not supported");
    shutdown(sockfd, SHUT_RDWR);
    return 0;
  }
  else {
    printf("Received protocol %s\n", recvBuffer);
  }
  
  //Send confirmation ("OK")
  write(sockfd, "OK", 3);

  //Recieve command
  memset(recvBuffer, '\0', sizeof(recvBuffer));
  int s = read(sockfd, recvBuffer, sizeof(recvBuffer));
  printf("recv %s : %d\n", recvBuffer, s);

  //Calculate and send answer
  char cmd[4];
  memset(cmd, '\0', 4);
  if(recvBuffer[0] == 'f') {
    memcpy(cmd, recvBuffer + 1, 3);
    char* tmp = recvBuffer;
    double v1 = strtod(recvBuffer + 5, &tmp);
    double v2 = strtod(tmp + 1, NULL);
    double ans = calc<double>(cmd, v1, v2);
    printf("%8.8g\n", ans); 
    char sans[32];
    sprintf(sans, "%8.8g", ans);
    write(sockfd, sans, sizeof(sans));
  }
  else {
    memcpy(cmd, recvBuffer, 3);
    char* tmp = recvBuffer;
    int v1 = strtol(recvBuffer + 4, &tmp, 10);
    int v2 = strtol(tmp + 1, NULL, 10);
    int ans = calc<int>(cmd, v1, v2);
    printf("%d\n", ans);  
    char sans[32];
    sprintf(sans, "%d", ans);
    write(sockfd, sans, sizeof(sans));
  }

  //Receive confirmation
  

  shutdown(sockfd, SHUT_RDWR);
  return 0;
}
