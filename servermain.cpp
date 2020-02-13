#include <stdio.h>
#include <stdlib.h>
/* You will to add includes here */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>


// Included to get the support library
#include <calcLib.h>

#include <iostream>

using namespace std;

int bind_socket(int socket, short port, sockaddr_in& addr) {
  socklen_t serverAddrlen = sizeof(addr);
  memset(&addr, 0, serverAddrlen);
  addr.sin_port = htons(port);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  return bind(socket, (sockaddr*)&addr, serverAddrlen);
}

void getAddrStr(sockaddr_in& addr, char* str, size_t strlen) {
  inet_ntop(AF_INET, &addr.sin_addr, str, strlen);
}

int main(int argc, char *argv[]){
  /* Do more magic */
  if(argc < 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    return 1;
  }
  initCalcLib();
  
  short port = atoi(argv[1]);

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd == -1){
    fprintf(stderr, "Failed to create socket\n");
    return 2;
  }
  sockaddr_in serverAddr;
  if(bind_socket(sockfd, port, serverAddr) == -1) {
    fprintf(stderr, "Failed to bind to port\n");
    return 3;
  }

  char name[INET_ADDRSTRLEN];
  getAddrStr(serverAddr, name, sizeof(name));

  int listerr = listen(sockfd, 5);
  if(listerr == -1) {
    fprintf(stderr, "Failed to listen\n");
    return 4;
  }

  printf("Listening on %s:%d\n", name, ntohs(serverAddr.sin_port));

  while(true) {
    sockaddr_in clientAddr;
    socklen_t clientlen;
    int clientsock = accept(sockfd, (sockaddr*)&clientAddr, &clientlen);
    if(clientsock == -1) {
      fprintf(stderr, "Failed to accept client\n");
    }
    else {
      char cbuf[INET_ADDRSTRLEN];
      getAddrStr(clientAddr, cbuf, sizeof(cbuf));
      printf("%s connected\n", cbuf);
    }

    //Send protocol
    char msg[] = "TEXT TCP 1.0"; 
    write(clientsock, msg, sizeof(msg));
    
    //Receive confirmation
    char recvBuffer[256];
    int nbytes = read(clientsock, recvBuffer, sizeof(recvBuffer));
    printf("Received %s\n", recvBuffer);
    if(strcmp(recvBuffer, "OK") != 0 || nbytes == -1) {
      fprintf(stderr, "Client not supported\n");
      shutdown(sockfd, SHUT_RDWR);
      continue;
    }
    else {
      printf("Client confirmed\n");
    }

    //Send operation
    char* op = randomType();
    char myop[32];
    memset(myop, '\0', 64);
    memcpy(myop, op, 5);
    
    double f1, f2;
    int i1, i2;
    bool isFloat = false;
    if(myop[0] == 'f') {
      isFloat = true;
      f1 = randomFloat();
      f2 = randomFloat();
      char sf1[32];
      char sf2[32];
      sprintf(sf1, " %8.8g", f1);
      sprintf(sf2, " %8.8g", f2);
      strcat(myop, strcat(sf1, sf2));
      write(clientsock, myop, sizeof(myop));
    }
    else{
      i1 = randomInt();
      i2 = randomInt();
      char si1[32];
      char si2[32];
      sprintf(si1, " %d", i1);
      sprintf(si2, " %d", i2);
      strcat(myop, strcat(si1, si2));
      write(clientsock, myop, sizeof(myop));
    }

    //Receive answer


    //Check if correct

    //Send OK if correct, ERROR otherwise

  }

  shutdown(sockfd, SHUT_RDWR);
  return 0;  

}
