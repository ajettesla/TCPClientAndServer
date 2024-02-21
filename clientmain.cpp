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

//github  https://github.com/ajettesla/Assignment1a.git

std::vector<std::string> split(std::string sString,std::string delimiter);
void gsready(std::string &ip, int port, struct sockaddr_in *ipv4,struct sockaddr_in6 *ipv6, int* ipstatus);


std::vector<std::string> nString;


std::string temp;


std::string recvBuffer(int sockerfd);


void sendBuffer(int socketfd,const char* mess);


char* math(std::string string, double a, double b);



int main(int argc, char *argv[]){



std::string delimiter = ":";

std::vector<std::string> outputString = split(argv[1],":"); 

std::string ipString = "";
int port;

if(outputString.size() > 2){
  port = atoi(outputString[outputString.size()-1].c_str());
  for(int i=0; i < 8 ; i++){
  if(i > 0){
   ipString = ipString + ":" + outputString[i];}
   else{
   ipString = ipString + outputString[i];}
   }
  }

else{
port = atoi(outputString[1].c_str());
ipString = outputString[0];
}

#ifdef DEBUG 
  std::cout << "Host Name is " << ipString << " port " << port << std::endl;
#endif

struct sockaddr_in6 *ipv6 = new struct sockaddr_in6;

struct sockaddr_in *ipv4 = new struct sockaddr_in;

int *ipstatus = new int;

gsready(ipString ,port, ipv4, ipv6, ipstatus);

#ifdef DEBUG 
  if(*ipstatus == 1){
  std::cout << "IP type IPV4" << std::endl;}
  else if(*ipstatus == 2){
  std::cout << "IP type is IPV6" << std::endl;
  }
#endif


if(*ipstatus == 1){
   int socketfd;
   socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketfd == -1) {
        std::cerr << "Error in socket creation" << std::endl;
        return 1;
    }

   if(connect(socketfd, (struct sockaddr*)ipv4,sizeof(struct sockaddr_in)) == -1){
    std::cout << "Error in connect" << std::endl;
    close(socketfd);}

    std::string okChar = "OK\n";
     
    recvBuffer(socketfd);
    sendBuffer(socketfd,okChar.c_str());
    std::string message = recvBuffer(socketfd);
    std::vector<std::string> inputVector = split(message," ");
    double a = std::stod(inputVector[1]);
    double b = std::stod(inputVector[2]);
    char *result = math(inputVector[0], a, b);
    sendBuffer(socketfd, result);
    recvBuffer(socketfd);
    close(socketfd);
 
             }
else if(*ipstatus == 2){

     int socketfd;
     
     socketfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
     
     if(socketfd == -1) {
        std::cout << "Error in socket creation" << std::endl;
        return 1;
    }
    
    if(connect(socketfd, (struct sockaddr*)ipv6,sizeof(struct sockaddr_in6)) == -1){
    perror("connect error is " );
    close(socketfd);
    return 1;
    }
    
    std::string okChar = "OK\n";
    recvBuffer(socketfd);
    sendBuffer(socketfd,okChar.c_str());
    std::string message = recvBuffer(socketfd);
    std::vector<std::string> inputVector = split(message," ");
    double a = std::stod(inputVector[1]);
    double b = std::stod(inputVector[2]);
    char *result = math(inputVector[0], a, b);
    sendBuffer(socketfd, result);
    recvBuffer(socketfd);
    close(socketfd);
                }


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

}
freeaddrinfo(output);
}


void sendBuffer(int socketfd,const char* mess){

int sent_recv_bytes = send(socketfd, mess, strlen(mess),0);

if(sent_recv_bytes == -1){
std::cout << "There is error while sending " << std::endl;
perror("send");}

else{
std::cout << mess;}

}

std::string recvBuffer(int socketfd){
int bufferSize = 1024;
char recv_buffer[bufferSize];
int sent_recv_bytes = recv(socketfd, recv_buffer, bufferSize,0);
recv_buffer[sent_recv_bytes] = '\0';
std::string recvBuf(recv_buffer);
if(sent_recv_bytes == -1){
std::cout << "There is error while receving data" << std::endl;
}
else{
std::cout << recv_buffer;
}

return recvBuf;
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
        int c = (int)(a - b); 
        sprintf(str, "%d\n", c);
    } else if (string == "mul") {
        int c = (int)(a * b);
        sprintf(str, "%d\n", c);
    } else if (string == "add") {
        int c = (int)(a + b); 
        sprintf(str, "%d\n", c);
    } else if (string == "div") {
        if (b != 0) {
            int c = (int)(a / b); 
            sprintf(str, "%d\n", c);
        } else {
            strcpy(str, "Error: Division by zero\n");
        }
    } else {
        strcpy(str, "Error: Invalid operation\n");
    }

    return str;
}


