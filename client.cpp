#include <sys/socket.h>
#include <functional>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>

int	main(int argc, char **argv){
	int sock = 0, valread;
    struct sockaddr_in serv_addr;
    std::string hello = "NICK mdr\r\nUSER bla bla bla\r\n";
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    send(sock , hello.data() , hello.size() , 0 );
    printf("Hello message sent\n");
    valread = read( sock , buffer, 1024);
    printf("%s\n",buffer );
    return 0;
}