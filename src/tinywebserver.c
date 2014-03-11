#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>


#include "rio.h"

#define BACKLOG 10
#define MAX_LINE 200
#define MAX_BUFFER 100

extern char *environ;

void print_error(const char *err)
{
    perror("ERROR: ");
    perror(err);
    perror("\n");
}

void print_client(struct sockaddr *clientaddr)
{
    char buf[16];
    
    struct sockaddr_in *addr = (struct sockaddr_in*)clientaddr;
    long ip = addr->sin_addr.s_addr;
    int a=(int)(ip/16777216);
    int b=(int)((ip%16777216)/65536);
    int c=(int)(((ip%16777216)%65536)/256);
    int d=(int)(((ip%16777216)%65536)%256);
    
    sprintf(buf, "%d.%d.%d.%d", d, c, b, a);
    printf("Vist IP : %s\n",  buf); 
}

int open_listenfd(char *port)
{
    struct addrinfo hint, *result;
    int res, sd;
    
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    
    res = getaddrinfo("0.0.0.0", port, &hint, &result);
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

void readheaders(rio_t *r)
{
    char buf[MAX_LINE];
    
    Rio_readlineb(r, buf, MAX_LINE);
    while (strcmp(buf, "\r\n") != 0) {
        Rio_readlineb(r, buf, MAX_LINE);
    }
	
	
}

int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;
	
    if (strstr(uri, "dynamic") == NULL) {
        strcpy(cgiargs, "");
        strcpy(filename, "./html/static");
        strcat(filename, uri);
        if (uri[strlen(uri) - 1] == '/')
            strcat(filename, "index.html");
        printf("filename : %s\n", filename);   
        return 1;
    
    } else {
        ptr = strchr(uri,'?');
        if (ptr != NULL)  {
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        }
        else
            strcpy(cgiargs, "");
            
        strcpy(filename, "./html");
        strcat(filename, uri);
        
        printf("%s\n", filename);
        return 0;
    }
}

void get_filetype(char *filename, char *filetype)
{
	if (strstr(filename, ".html"))
		strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
		strcpy(filetype, "image/gif");
	else if (strstr(filename, ".jpg"))
		strcpy(filetype, "image/jpeg");
    else if (strstr(filename, ".swf"))
        strcpy(filetype, "application/x-shockwave-flash");
	else
		strcpy(filetype, "text/plain");
        
    printf("filename: %s\n", filename);
    printf("filetype: %s\n", filetype);
}

void server_redirect(int fd, char *filename, int filesize)
{
    int srcfd;
	char *srcp, filetype[MAX_LINE], buf[MAX_BUFFER];
	
	get_filetype(filename, filetype);
	sprintf(buf, "HTTP/1.0 302 Moved Temporarily\r\n");
    sprintf(buf, "%sLocation: http://www.baidu.com\r\n", buf);
	sprintf(buf, "%sServer: Spch2008 Web Server\r\n", buf);
	//sprintf(buf, "%sContent-length:%d\r\n", buf, filesize);
    sprintf(buf, "%sContent-length:%d\r\n", buf, 0);
	sprintf(buf, "%sContent-type:%s\r\n\r\n", buf, filetype);
	Rio_writen(fd, buf, strlen(buf));

/*	
	srcfd = open(filename, O_RDONLY, 0);
	srcp  = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	close(srcfd);
	Rio_writen(fd, srcp, filesize);
	munmap(srcp, filesize);
    */
}

void serve_static(int fd, char *filename, int filesize)
{
	int srcfd;
	char *srcp, filetype[MAX_LINE], buf[MAX_BUFFER];
	
	get_filetype(filename, filetype);
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: Spch2008 Web Server\r\n", buf);
	sprintf(buf, "%sContent-length:%d\r\n", buf, filesize);
	sprintf(buf, "%sContent-type:%s\r\n\r\n", buf, filetype);
	Rio_writen(fd, buf, strlen(buf));
	
	srcfd = open(filename, O_RDONLY, 0);
	srcp  = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	close(srcfd);
	Rio_writen(fd, srcp, filesize);
	munmap(srcp, filesize);
	
}


void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    printf("end\n");
    char buf[MAX_LINE], *empty_list[] = { NULL };
    
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Spch2008 Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
    
    printf("being\n");
    if (fork() == 0) {
        setenv("QUERY_STRING", cgiargs, 1);
        dup2(fd, STDOUT_FILENO);
        execve(filename, empty_list, (char *const *)environ);
        printf("ok\n");
    }
    wait(NULL);  
}

void handle(int fd)
{
    
    char buf[MAX_LINE];
    char method[MAX_LINE], uri[MAX_LINE], version[MAX_LINE];
    char filename[MAX_LINE], cgiargs[MAX_LINE];
    int  is_static;
    struct stat fileinfo;
    rio_t rio;
    
	
    Rio_init(&rio, fd);
    Rio_readlineb(&rio, buf, MAX_LINE);
    sscanf(buf, "%s %s %s", method, uri, version);
	
    
    if (strcasecmp(method, "GET") != 0) {
        //error
    }
    
    readheaders(&rio);
    
	
    is_static = parse_uri(uri, filename, cgiargs);
    if (stat(filename, &fileinfo) < 0) {
        //error
    }
    
    if (is_static) {
        if (!(S_ISREG(fileinfo.st_mode)) || !(S_IRUSR & fileinfo.st_mode)) {
            //error
        }
        
        if (strstr(filename, "redirect"))
            server_redirect(fd, filename, fileinfo.st_size);
        else
            serve_static(fd, filename, fileinfo.st_size);
    } else {
        if (!(S_ISREG(fileinfo.st_mode)) || !(S_IXUSR & fileinfo.st_mode)) {
            //error
        }
        serve_dynamic(fd, filename, cgiargs);
    }
    
}


int main(int argc, char *argv[])
{
	int listenfd, connfd, addrlen;
	struct sockaddr clientaddr;
	
	if (argc < 2) {
		printf("usage: %s <port>\n", argv[0]);
		exit(1);
	}
		
	listenfd = open_listenfd(argv[1]);
	addrlen = sizeof(clientaddr);
	while (1) {
		connfd = accept(listenfd, &clientaddr, &addrlen);
        print_client(&clientaddr);
		handle(connfd);
		close(connfd);	
	}
	
    return 0;
}
