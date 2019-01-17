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
#include <netdb.h>
#define BUFFER_SIZE 1024 
using namespace std;

enum HTTPRequest{
    GET, HEAD
};

char* generateSendline(HTTPRequest request,const char* ipAddress,const char* port,const char* fileName);
void getHTTPHEAD(int sockfd);

char* generateSendline(HTTPRequest request,const char* ipAddress,const char* port,const char* fileName){
    std::string requestLine, sendLine, sfileName(fileName), sipAddress(ipAddress), sport(port);
    if(request == GET){
        requestLine = "GET";
    }else if(request == HEAD){
        requestLine = "HEAD";
    }
    // Closed keep connect Connection:Close
    sendLine = requestLine + " /" + sfileName + " HTTP/1.1\r\nHost: " + sipAddress + ":" + sport + "\r\nConnection:Close\r\n\r\n";
    std::cout << "Generate sendline: " << sendLine << std::endl;
    const char* temp = sendLine.data();
    char *buf=new char[strlen(temp)+1];
    strcpy(buf, temp);
    return buf;
}

void getHTTPHEAD(int sockfd){
    if(sockfd == 0){
        printf("\n Socket invalid \n");
    }
    char buffer[1] = {0};
    std::string head;
    int cnt = 0;
    // Find the end of head with \r\n\r\n
    while(true){
        if((int)recv(sockfd, buffer, 1, 0)){
            head.push_back(buffer[0]);
            if(buffer[0] == '\r' || buffer[0] == '\n' ){
                ++cnt;
            }else{
                cnt = 0;
            }
            if(cnt == 4){
                // End of head
                std::cout << "----------------------Start of HEAD----------------- \n" << head << std::endl;
                std::cout << "----------------------End of HEAD------------------- \n" << std::endl;
                break;
            }
        }else{
            printf("\n HEAD invalid");
            break;
        }
    }

}

int main(int argc, char const *argv[]) 
{ 
    int sockfd = 0, valread; 
    struct sockaddr_in serv_addr; 
    HTTPRequest request;
    char buffer[BUFFER_SIZE] = {0}; 
    char* sendline;
    struct hostent *hostptr;
    char host_address[INET_ADDRSTRLEN];
    const char* address;
    if(atoi(argv[2]) < 0 || atoi(argv[2]) > 65535){
        printf("\n Port error\n"); 
    }

    // Host address
    if((hostptr = gethostbyname(argv[1]))==NULL){
        printf("gethostbyname error for host: %s: %s\n",argv[1],hstrerror(h_errno));
    }

    if((address = inet_ntop(hostptr->h_addrtype, hostptr->h_addr, host_address,sizeof(host_address)))<0){
        printf("\n Invalid address: %s\n",argv[1]);
        return -1; 
    }
    printf("official hostname: %s\n", hostptr->h_name);

    printf("Address: %s\n", address);

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
        printf("Socket creation error %s(errno: %d)\n", strerror(errno),errno);
        return -1; 
    }else{
        printf("Socket created \n"); 
    }
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
    int port_num = atoi(argv[2]);
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port_num); 


    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, address, &serv_addr.sin_addr)<=0)  
    { 
        printf("\n Invalid address: %s\n",argv[1]);
        return -1; 
    } 
   
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\n Connection Failed %s(errno: %d)\n",strerror(errno),errno);
        return -1; 
    } 

    // Send request message 
    printf("send message to server:\n%s\n", sendline);
    if( send(sockfd, sendline, strlen(sendline), 0) < 0){
        printf("send message error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    printf("Client message sent\n"); 
    // Recive data
    // Handle HTTP HEAD
    getHTTPHEAD(sockfd);
    // Create file
    std::string filename = argv[3];
    ofstream fout(filename);

    while(1){
        memset(buffer, 0, BUFFER_SIZE); // Clean buffer before each recv
        int recv_size = (int)recv(sockfd, buffer, BUFFER_SIZE, 0);
        if( recv_size >0 )
        {
            // Handle the buffer
            printf("Server return message: \n%s\n",buffer );
            // Write to file
            fout << buffer;
        }
        else{
            // Handle socket recv error
            if((recv_size<0) &&(recv_size == EAGAIN||recv_size == EWOULDBLOCK||recv_size == EINTR)) //error code, connection doesn't fail continue
            {
                printf("\n Socket error %s(errno: %d)\n", strerror(errno),errno);
                continue;
            }
            printf("Data recive success! \n");
            break;
        }

    }
    // End of connection
    fout.close();
    close(sockfd);
    return 0; 
} 