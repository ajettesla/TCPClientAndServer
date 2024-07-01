#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <calcLib.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <chrono>



std::vector<std::string> split(std::string sString,std::string delimiter);

int gsready(std::string &ip, int port,int* ipstatus);

char* math(std::string string, double a, double b);

std::string getCalString();

void printtime();

char buffer[1024];
int master_socketfd = 0;
int comm_socketfd = 0;
int sent_recv_bytes;
fd_set readfds;

char errorchar[] = "ERROR\n";



int main(int argc, char *argv[]){

  initCalcLib();
  std::string delimiter = ":";
  std::vector<std::string> outputString = split(argv[1],":"); 
  std::string ipString = "";
  int port;
  char timeo[] = "ERROR TO\n";

  if(outputString.size() > 2){
  port = atoi(outputString[outputString.size()-1].c_str());
  for(int i=0; i < 8 ; i++){
   ipString = ipString + outputString[i];}}
  else{
   port = atoi(outputString[1].c_str());
   ipString = outputString[0];}

  int *ipstatus = new int;

  int master_socketfd = gsready(ipString ,port, ipstatus);   
  FD_ZERO(&readfds);
  FD_SET(master_socketfd, &readfds);

  while(1){
    int rc = select(master_socketfd + 1, &readfds, NULL, NULL, NULL);
    if(rc < 0){perror("error with select system call"); exit(1);}

    if(FD_ISSET(master_socketfd, &readfds)){
      struct sockaddr_in client_addr;
      struct sockaddr_in6 client_addr6;
      if(*ipstatus == 1){
      socklen_t addrlen = sizeof(client_addr);
      comm_socketfd = accept(master_socketfd, (struct sockaddr*)&client_addr, &addrlen);}
      else if(*ipstatus == 2){
      socklen_t addrlen = sizeof(client_addr6);
      comm_socketfd = accept(master_socketfd, (struct sockaddr*)&client_addr, &addrlen);}

      if(comm_socketfd < 0){
      perror("error with accept");exit(1);}

      struct timeval timeout;
      timeout.tv_sec = 5;
      timeout.tv_usec = 0;

      if(setsockopt(comm_socketfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1){{perror("setsockopt");exit(1);}}
      std::cout << std::endl;
      
      while(1){
        char intialMessage[] = "TEXT TCP 1.0\n";
        sent_recv_bytes = send(comm_socketfd, intialMessage, sizeof(intialMessage),0);
        if(sent_recv_bytes < 0){perror("error while sending data to client");close(comm_socketfd);exit(1);}
        char newline[] = "\n";
        sent_recv_bytes = send(comm_socketfd, newline, sizeof(newline),0);
        if(sent_recv_bytes < 0){perror("error while sending data to client");close(comm_socketfd);exit(1);}
         memset(buffer, 0, sizeof(buffer));
         sent_recv_bytes = recv(comm_socketfd, buffer, sizeof(buffer), 0);
         if(sent_recv_bytes < 0){
             if(errno == EAGAIN){
             sent_recv_bytes = send(comm_socketfd, timeo, sizeof(timeo),0);
                if(sent_recv_bytes < 0){perror("fail to send data");exit(1);}
                close(comm_socketfd);break;}
                else{perror("error while receving datat");close(comm_socketfd);exit(1);} }

        std::string receivedData(buffer);
        receivedData = receivedData.substr(0,sent_recv_bytes-1);

        if(receivedData == "OK"){
           std::string calString = getCalString();
           std::vector<std::string> inputVector = split(calString," ");
           calString = calString + "\n";
           std::cout << calString;
           double a = std::stod(inputVector[1]);
           double b = std::stod(inputVector[2]);
           char *result = math(inputVector[0], a, b);
           std::string calServerResult(result); 
           std::cout << result;
           sent_recv_bytes = send(comm_socketfd, calString.c_str(), strlen(calString.c_str()),0);
           if(sent_recv_bytes < 0){perror("error while sending data ");close(comm_socketfd);exit(1);}

           memset(buffer, 0, sizeof(buffer));
           sent_recv_bytes = recv(comm_socketfd, buffer, sizeof(buffer), 0);

           if(sent_recv_bytes < 0){
               if(errno == EAGAIN){
                sent_recv_bytes = send(comm_socketfd, timeo, sizeof(timeo),0);
                   if(sent_recv_bytes < 0){perror("fail to send data");exit(1);}
                   close(comm_socketfd);break;}
                   else{perror("error while receving data"); close(comm_socketfd);exit(1);}
                   close(comm_socketfd);break;}
           
           std::string receivedData(buffer);
           receivedData = receivedData.substr(0,sent_recv_bytes-1);
           receivedData = receivedData + "\n";
           if(receivedData == calServerResult){
            std::cout << "OK" << std::endl;
            char okchar[] = "OK\n";
            sent_recv_bytes = send(comm_socketfd, okchar, sizeof(okchar),0);
            if(sent_recv_bytes < 0){perror("error while sending data to client");close(comm_socketfd);exit(1);}
            close(comm_socketfd);
            break;}
            else {
             std::cout << errorchar;
             sent_recv_bytes = send(comm_socketfd, errorchar, sizeof(errorchar),0);
             if(sent_recv_bytes < 0){perror("error while sending data to client"); close(comm_socketfd);exit(1);}
             close(comm_socketfd);
             break;}
              }
            else{
              std::cout << errorchar;
              sent_recv_bytes = send(comm_socketfd, errorchar, sizeof(errorchar),0);
              close(comm_socketfd);
              break;}
            

      }
   }
  }







  return 0;
}


int gsready(std::string &ip, int port,int* ipstatus){
int socketfd; 
struct sockaddr_in ipv4;
struct sockaddr_in6 ipv6;
struct addrinfo hint, *output, *temp;
memset(&hint, 0, sizeof(hint));
hint.ai_family = AF_UNSPEC;
hint.ai_socktype = SOCK_STREAM;
int status = getaddrinfo(ip.c_str(), NULL, &hint, &output);
if(status != 0){
perror("error with getaddress info");exit(1);}

for(temp=output; temp != NULL;temp->ai_addr){
if(temp->ai_family == AF_INET){
ipv4.sin_family = AF_INET;
ipv4.sin_port = htons(port);
ipv4.sin_addr.s_addr = ((struct sockaddr_in*)temp->ai_addr)->sin_addr.s_addr;
socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if(socketfd > 0){
  if(bind(socketfd,(struct sockaddr*)&ipv4,sizeof(struct sockaddr)) < 0){perror("error with binding the ip address");exit(1);}
  std::cout << "Listening on " << ip << " port " << port << std::endl;
  *ipstatus = 1;
   break;
}}
                                              
else if(temp->ai_family == AF_INET6){
ipv6.sin6_family = AF_INET6;
ipv6.sin6_port = htons(port);
ipv6.sin6_addr = ((struct sockaddr_in6*)temp->ai_addr)->sin6_addr;
socketfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
if(socketfd > 0){
  if(bind(socketfd,(struct sockaddr*)&ipv6,sizeof(struct sockaddr_in6)) < 0){perror("error with binding the ip address");exit(1);}
   std::cout << "Listening on " << ip << " port " << port << std::endl;
  *ipstatus = 2;
   break;
}}}

if(*ipstatus != 1 && *ipstatus != 2){
  perror("error with socket");
  exit(1);}

if(listen(socketfd, 5) < 0){perror("error with listen function");exit(1);}
  
freeaddrinfo(output);
return socketfd;
}


char* math(std::string string, double a, double b) {
    char* str = new char[30]; 
    if (str == NULL) {
        return NULL; 
    }

    if (string == "fsub") {
        double c = a - b;
        sprintf(str, "%8.8g\n", c);
    } else if (string == "fmul") {
        double c = a * b;
        sprintf(str, "%8.8g\n", c);
    } else if (string == "fadd") {
        double c = a + b;
       sprintf(str, "%8.8g\n", c);
    } else if (string == "fdiv") {
        if (b != 0) {
            double c = a / b;
            sprintf(str, "%8.8g\n", c);
        } else {
            strcpy(str, "Error: Division by zero\n");
        }
    } else if (string == "sub") {
        int c = (int)(a - b); // Convert result to integer for "sub"
        sprintf(str, "%d\n", c);
    } else if (string == "mul") {
        int c = (int)(a * b); // Convert result to integer for "mul"
        sprintf(str, "%d\n", c);
    } else if (string == "add") {
        int c = (int)(a + b); // Convert result to integer for "add"
        sprintf(str, "%d\n", c);
    } else if (string == "div") {
        if (b != 0) {
            int c = (int)(a / b); // Convert result to integer for "div"
            sprintf(str, "%d\n", c);
        } else {
            strcpy(str, "Error: Division by zero\n");
        }
    } else {
        strcpy(str, "Error: Invalid operation\n");
    }

    return str;
}

std::vector<std::string> split(std::string sString,std::string delimiter){
std::vector<std::string> nString;
std::string temp;

for(int i=0; i < static_cast<int>(sString.length());i++){
  int  count = 0;
  if(sString[i] == delimiter[0]){
        count++;
        nString.push_back(temp);
        temp  = "";
    }
  else{
        temp = temp +  sString[i];
         }

  if(count==0 && (i == static_cast<int>(sString.length()-1))){
         nString.push_back(temp);}               }

  if(nString.size() < 2){
    std::cout << "ERROR" << std::endl;
    exit(1);
  }



return nString;
}


std::string getCalString(){

std::string calString;

double float1,float2;
int integer1,integer2;

char *oper;

oper = randomType();

integer1 = randomInt();
integer2 = randomInt();

float1 = randomFloat();
float2 = randomFloat();

    if(strcmp(oper,"fadd")==0){
      calString = "fadd " + std::to_string(float1) + " " + std::to_string(float2);
    } else if (strcmp(oper, "fsub")==0){
       calString = "fsub " + std::to_string(float1) + " " + std::to_string(float2);
    } else if (strcmp(oper, "fmul")==0){
       calString = "fmul " + std::to_string(float1) + " " + std::to_string(float2);
    } else if (strcmp(oper, "fdiv")==0){
       calString = "fdiv " + std::to_string(float1) + " " + std::to_string(float2);
    }
    else if(strcmp(oper,"add")==0){
       calString = "add " + std::to_string(integer1) + " " + std::to_string(integer2);
    } else if (strcmp(oper, "sub")==0){
       calString = "sub " + std::to_string(integer1) + " " + std::to_string(integer2);
    } else if (strcmp(oper, "mul")==0){
      calString = "mul " + std::to_string(integer1) + " " + std::to_string(integer2);
    } else if (strcmp(oper, "div")==0){
      calString = "div " + std::to_string(integer1) + " " + std::to_string(integer2);
    }

return calString;
}
