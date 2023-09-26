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


// Enable if you want debugging to be printed, see examble below.
// Alternative, pass CFLAGS=-DDEBUG to make, make CFLAGS=-DDEBUG
#define DEBUG

std::vector<std::string> split(std::string sString,std::string delimiter);

void gsready(std::string &ip, int port, struct sockaddr_in *ipv4,struct sockaddr_in6 *ipv6, int* ipstatus);


char* math(std::string string, double a, double b);

std::string getCalString();

char buffer[1000];
int master_socketfd = 0;
int comm_socketfd = 0;
int sent_recv_bytes;
fd_set readfds;

int main(int argc, char *argv[]){
  initCalcLib();
  
  std::string delimiter = ":";
  
  std::vector<std::string> outputString = split(argv[1],":"); 
 
  std::string ipString = "";
  
  int port;
  
  if(outputString.size() > 2){
  port = atoi(outputString[outputString.size()-1].c_str());
  for(int i=0; i < 8 ; i++){
   ipString = ipString + outputString[i];
           }
         }
     else{
   port = atoi(outputString[1].c_str());
   ipString = outputString[0];
      }

   struct sockaddr_in6 *ipv6 = new struct sockaddr_in6;

   struct sockaddr_in *ipv4 = new struct sockaddr_in;

   int *ipstatus = new int;

   gsready(ipString ,port, ipv4, ipv6, ipstatus);
   
   if(*ipstatus == 1){
      master_socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      
      if(master_socketfd < 0){
      std::cout << "Error while creating master socket file descriptor" << std::endl;
      exit(1);
      }
      
      if(bind(master_socketfd, (struct sockaddr*)ipv4, sizeof(struct sockaddr)) < 0){
      std::cout << "Error while bind with ip and port " << std::endl;
      exit(1);
      }
      
      socklen_t addrlen = sizeof(struct sockaddr);
      
      if(listen(master_socketfd, 5) < 0){
      std::cout <<" listen failed " << std::endl;      
      exit(1);}

      while(1){
      FD_ZERO(&readfds);
      FD_SET(master_socketfd, &readfds);
      
      select(master_socketfd + 1, &readfds, NULL, NULL, NULL);
      
      if(FD_ISSET(master_socketfd, &readfds)){
      std::cout << "New connection is arrived " << std::endl;
      }
      
      struct sockaddr_in client_addr;
      
      comm_socketfd = accept(master_socketfd, (struct sockaddr*)&client_addr, &addrlen);
      
      if(comm_socketfd < 0){
      std::cout << "Accept ERROR" << std::endl;
      
      }
      
      
      struct timeval timeout;
      timeout.tv_sec = 5;
      timeout.tv_usec = 0;
      
          
      setsockopt(comm_socketfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
      
      memset(buffer, 0, sizeof(buffer));
      
      char intialMessage[] = "TEXT TCP 1.0\n";
      
      sent_recv_bytes = send(comm_socketfd, intialMessage, sizeof(intialMessage),0);
      
      
      if(sent_recv_bytes < 0){
      std::cout << "error while sending data to client" << std::endl;
      close(comm_socketfd);
      break;
      }
     
      memset(buffer, 0, sizeof(buffer));
      recvAgain:
      
      sent_recv_bytes = recv(comm_socketfd, buffer, sizeof(buffer), 0);
      if(sent_recv_bytes < 0){
      
      std::cout << "timeout is occured" << std::endl;
      close(comm_socketfd);
      break;
      }
      
      std::string receivedData(buffer);
      
         
      receivedData = receivedData.substr(0,sent_recv_bytes-1);
        
      if(receivedData == "OK"){
       std::string calString = getCalString();
       std::vector<std::string> inputVector = split(calString," ");
        calString = calString + "\n";
        std::cout << calString << std::endl;
        double a = std::stod(inputVector[1]);
        double b = std::stod(inputVector[2]);
        char *result = math(inputVector[0], a, b);
        std::string calServerResult(result);        
        std::cout << "server result is " << calServerResult << std::endl;
        sent_recv_bytes = send(comm_socketfd, calString.c_str(), strlen(calString.c_str()),0);
        if(sent_recv_bytes < 0){
          std::cout << "error while sending data " << std::endl;
          close(comm_socketfd);
          break;
        }
        
        Again:
        
        memset(buffer, 0, sizeof(buffer));
        sent_recv_bytes = recv(comm_socketfd, buffer, sizeof(buffer), 0);
        
        if(sent_recv_bytes < 0){
                std::cout << "timeout is occured" << std::endl;
                goto Again;
                     }
                                
          std::cout << sent_recv_bytes << std::endl;
            
          std::string receivedData(buffer);
      
          receivedData = receivedData.substr(0,sent_recv_bytes-1);
          
          receivedData = receivedData + "\n";          
          
          
          if(receivedData == calServerResult){
           std::cout << "OK" << std::endl;
            char okchar[] = "OK\n";
            sent_recv_bytes = send(comm_socketfd, okchar, sizeof(okchar),0);
      
      
           if(sent_recv_bytes < 0){
                 std::cout << "error while sending data to client" << std::endl;
                 close(comm_socketfd);
                 break;
                                }
            close(comm_socketfd);
            break;}
              
             else {
             std::cout << "ERROR" << std::endl;
             std::cout << calServerResult.length() << std::endl;
             std::cout << receivedData.length() << std::endl;
             std::cout << calServerResult << std::endl;
             std::cout << "client result is " << receivedData << std::endl;
            char errorchar[] = "ERROR\n";
            sent_recv_bytes = send(comm_socketfd, errorchar, sizeof(errorchar),0);
           
           if(sent_recv_bytes < 0){
                 std::cout << "error while sending data to client" << std::endl;
                 close(comm_socketfd);
                 break;
                                }
              goto Again;             
             
             }
                                      
      
      }
      else{
        char wrongChar[] = "ERROR\n";
        sent_recv_bytes = send(comm_socketfd, wrongChar, sizeof(intialMessage),0);
         goto recvAgain;  
      }   

     
      }//while loop end;      
      
     
     
     } //if statement end;
  
   else if(*ipstatus == 2){
   
   master_socketfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
      
      if(master_socketfd < 0){
      std::cout << "Error while creating master socket file descriptor" << std::endl;
      exit(1);
      }
      
      if(bind(master_socketfd, (struct sockaddr*)ipv6, sizeof(struct sockaddr)) < 0){
      std::cout << "Error while bind with ip and port " << std::endl;
      exit(1);
      }
      
      socklen_t addrlen = sizeof(struct sockaddr);
      
      if(listen(master_socketfd, 5) < 0){
      std::cout <<" listen failed " << std::endl;      
      exit(1);}
      
      
      while(1){
      FD_ZERO(&readfds);
      FD_SET(master_socketfd, &readfds);
      
      select(master_socketfd + 1, &readfds, NULL, NULL, NULL);
      
      if(FD_ISSET(master_socketfd, &readfds)){
      std::cout << "New connection is arrived " << std::endl;
      }
      
      struct sockaddr_in client_addr;
      
      comm_socketfd = accept(master_socketfd, (struct sockaddr*)&client_addr, &addrlen);
      
      if(comm_socketfd < 0){
      std::cout << "Accept ERROR" << std::endl;
      
      }
      
      struct timeval timeout;
      timeout.tv_sec = 5;
      timeout.tv_usec = 0;
      
          
      setsockopt(comm_socketfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
      
      memset(buffer, 0, sizeof(buffer));
      
      char intialMessage[] = "TEXT TCP 1.0\n";
      
      sent_recv_bytes = send(comm_socketfd, intialMessage, sizeof(intialMessage),0);
      
       
      if(sent_recv_bytes < 0){
      std::cout << "error while sending data to client" << std::endl;
      close(comm_socketfd);
      break;
      }
     
      memset(buffer, 0, sizeof(buffer));
      
      recvAg:
      
      sent_recv_bytes = recv(comm_socketfd, buffer, sizeof(buffer), 0);
      if(sent_recv_bytes < 0){
      
      std::cout << "timeout is occured" << std::endl;
      close(comm_socketfd);
      break;
      }
      
      std::string receivedData(buffer);
      
         
      receivedData = receivedData.substr(0,sent_recv_bytes-1);
      
      if(receivedData == "OK"){
       std::string calString = getCalString();
       std::vector<std::string> inputVector = split(calString," ");
        calString = calString + "\n";
        std::cout << calString << std::endl;
        double a = std::stod(inputVector[1]);
        double b = std::stod(inputVector[2]);
        char *result = math(inputVector[0], a, b);
        std::string calServerResult(result);        
        std::cout << "server result is " << calServerResult << std::endl;
        sent_recv_bytes = send(comm_socketfd, calString.c_str(), strlen(calString.c_str()),0);
        if(sent_recv_bytes < 0){
          std::cout << "error while sending data " << std::endl;
          close(comm_socketfd);
          break;
        }
        
        Ag:
        
        memset(buffer, 0, sizeof(buffer));
        sent_recv_bytes = recv(comm_socketfd, buffer, sizeof(buffer), 0);
        
        if(sent_recv_bytes < 0){
                std::cout << "timeout is occured" << std::endl;
                goto Ag;
                     }
        std::cout << sent_recv_bytes << std::endl;
            
        std::string receivedData(buffer);
      
        receivedData = receivedData.substr(0,sent_recv_bytes-1);
          
        receivedData = receivedData + "\n"; 
        
        if(receivedData == calServerResult){
           std::cout << "OK" << std::endl;
            char okchar[] = "OK\n";
            sent_recv_bytes = send(comm_socketfd, okchar, sizeof(okchar),0);
      
      
        if(sent_recv_bytes < 0){
                 std::cout << "error while sending data to client" << std::endl;
                 close(comm_socketfd);
                 break;
                                }
            close(comm_socketfd);
            break;
            }
      
        else {
             std::cout << "ERROR" << std::endl;
             std::cout << calServerResult.length() << std::endl;
             std::cout << receivedData.length() << std::endl;
             std::cout << calServerResult << std::endl;
             std::cout << "client result is " << receivedData << std::endl;
            char errorchar[] = "ERROR\n";
            sent_recv_bytes = send(comm_socketfd, errorchar, sizeof(errorchar),0);
           
           if(sent_recv_bytes < 0){
                 std::cout << "error while sending data to client" << std::endl;
                 close(comm_socketfd);
                 break;
                                }
              goto Ag;             
             
             }}
             
             else{
        char wrongChar[] = "ERROR\n";
        sent_recv_bytes = send(comm_socketfd, wrongChar, sizeof(intialMessage),0);
         goto recvAg;  
      }
             
            
             
            
            
      
      
      
      
      
      
      }
      
   
   
   
   }   

return 0;
}

void gsready(std::string &ip, int port, sockaddr_in *ipv4, sockaddr_in6 *ipv6,int* ipstatus){

struct addrinfo hint, *output, *temp;
memset(&hint, 0, sizeof(hint));
hint.ai_family = AF_UNSPEC;
hint.ai_socktype = SOCK_STREAM;
int status = getaddrinfo(ip.c_str(), NULL, &hint, &output);
if(status != 0){
std::cout << "There is problem in getting getaddrinfo" << std::endl;

}

for(temp=output; temp != NULL;temp->ai_addr){

if(temp->ai_family == AF_INET){
ipv4->sin_family = AF_INET;
ipv4->sin_port = htons(port);
ipv4->sin_addr.s_addr = ((struct sockaddr_in*)temp->ai_addr)->sin_addr.s_addr;
*ipstatus = 1;
break;
                            }
                                              
else if(temp->ai_family == AF_INET6){
ipv6->sin6_family = AF_INET6;
ipv6->sin6_port = htons(port);
ipv6->sin6_addr = ((struct sockaddr_in6*)temp->ai_addr)->sin6_addr;
*ipstatus = 2;
break;

}
temp = temp->ai_next;
}
freeaddrinfo(output);
}


char* math(std::string string, double a, double b) {
    char* str = new char[30]; // Allocate memory for the result string
    if (str == NULL) {
        return NULL; // Memory allocation failed
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
