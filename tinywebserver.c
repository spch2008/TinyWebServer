#include <unistd.h>
#include <netdb.h>

#include "rio.h"

#define BACKLOG 10
#define MAX_LINE 20
#define MAX_BUFFER 100

void print_error(const char *err)
{
    perror("ERROR: ");
    perror(err);
    perror("\n");
}

int open_listenfd(char *port)
{
    struct addrinfo hint, *result;
    int res, sd;
    
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    
    res = getaddrinfo(NULL, port, &hint, &result);
    if (res == -1)
    {
        print_error("can not get address!");
        exit(1);
    }
    
    sd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sd == -1)
    {
        print_error("can not create socket");
        exit(1);
    }
    
    res = bind(sd, result->ai_addr, result->ai_addrlen);
    if (res == -1)
    {
        print_error("can not bind socket with address!");
        exit(1);
    }
    
    res = listen(sd, BACKLOG);
    if (res == -1)
    {
        print_error("can not listen!");
        exit(1);
    }
    
    freeaddrinfo(result);
    
    return sd;
}

void clienterror(int fd, char *cause, char *errnum, 
                 char *shortmsg, char *longmsg)
{
}



int main(int argc, char *argv[])
{
    return 0;
}
