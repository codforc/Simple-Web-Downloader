#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <string> 
#include <arpa/inet.h>
#include <unistd.h>
#include<errno.h>
#include<iostream>
#include<fstream>
#define PORT 8080 

using namespace std;

enum HTTPRequest{
    GET, HEAD
};

char* generateSendline(HTTPRequest request,const char* ipAddress,const char* port,const char* fileName){
    std::string requestLine, sendLine, sfileName(fileName), sipAddress(ipAddress), sport(port);
    if(request == GET){
        requestLine = "GET";
    }else if(request == HEAD){
        requestLine = "HEAD";
    }
    //closed keep connect Connection:Close
    sendLine = requestLine + " /" + sfileName + "  HTTP/1.1\r\nHost: " + sipAddress + " " + sport + "\r\nConnection:Close\r\n\r\n";
    std::cout << "generate sendline: " << sendLine << std::endl;
    const char* temp = sendLine.data();
    char *buf=new char[strlen(temp)+1];
    strcpy(buf, temp);
    return buf;
}




int main(int argc, char const *argv[]) 
{ 
    struct sockaddr_in address; 
    int sockfd = 0, valread; 
    struct sockaddr_in serv_addr; 
    HTTPRequest request;
    char buffer[1024] = {0}; 
    char* sendline;
    if(atoi(argv[2]) < 0 || atoi(argv[2]) > 65535){
        printf("\n Port error\n"); 
    }
    printf("hhhh: %i", argc);
    if(argc == 4){
        //Get command
        sendline =  generateSendline(GET, argv[1], argv[2], argv[3]);
    }else if(argc == 5 && strcmp(argv[4], "-h") == 0){
        //HEAD command
        sendline =  generateSendline(HEAD, argv[1], argv[2], argv[3]);
    }else{
        printf("usage: ./wd <ipaddress> <port> <file> (optional -h)\n");
        return 0;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error %s(errno: %d)\n", strerror(errno),errno);
        return -1; 
    }else{
        printf("\n Socket created \n"); 
    }
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(80); 
    
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)  
    { 
        printf("\n Invalid address: %s\n",argv[1]);
        return -1; 
    } 
   
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\n Connection Failed %s(errno: %d)\n",strerror(errno),errno);
        return -1; 
    } 

    //send request message 
    printf("send message to server:\n %s\n", sendline);
    if( send(sockfd, sendline, strlen(sendline), 0) < 0){
        printf("send message error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    printf("Client message sent\n"); 
    //recive data

    //create file
    std::string filename = argv[3];
    ofstream fout(filename);

    while(1){
        int recv_error = (int)recv(sockfd, buffer, 1024, 0);
        if( recv_error >0 )
        {
            //handle the buffer
            printf("Server return message: \n%s\n",buffer );
            //write to file
            fout << buffer;
        }
        else{
            //handle socket recv error
            if((recv_error<0) &&(recv_error == EAGAIN||recv_error == EWOULDBLOCK||recv_error == EINTR)) //error code, connection doesn't fail continue
            {
                printf("\n Socket error %s(errno: %d)\n", strerror(errno),errno);
                continue;
            }
            printf("Data recive success! \n");
            break;
        }

    }
    //end of connection
    fout.close();
    close(sockfd);
    return 0; 
} 