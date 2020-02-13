#include <stdio.h>
#include <stdlib.h>
/* You will to add includes here */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <math.h>

// Included to get the support library
#include <calcLib.h>

#include <iostream>

using namespace std;

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
    memset(recvBuffer, '\0', sizeof(recvBuffer));
    nbytes = read(clientsock, recvBuffer, sizeof(recvBuffer));
    if(nbytes == 0) {
      fprintf(stderr, "Failed to read client answer\n");
      continue;
    }

    //Check if correct
    bool isCorrect = false;
    if(isFloat) {
      double ans = strtod(recvBuffer, NULL);
      printf("%8.8g\n", ans);
      double myans = calc<double>(op + 1, f1, f2);
      printf("%8.8g\n", myans);
      /*
      if(ans == myans) {
        isCorrect = true;
      }
      */
     if(fabs(ans - myans) < 0.0000001) {
       isCorrect = true;
     }
    }
    else {
      int ans = atoi(recvBuffer);
      printf("%d\n", ans);
      int myans = calc<int>(op, i1, i2);
      printf("%d\n", myans);
      if(ans == myans) {
        isCorrect = true;
      }
    }

    printf("%d\n", isCorrect);

    //Send OK if correct, ERROR otherwise
    
  }

  shutdown(sockfd, SHUT_RDWR);
  return 0;  

}
