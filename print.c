#include <stdio.h>
#include "rio.h"
#include <unistd.h>
#include <string.h>

#define MAX_LEN 100

int main(int argc, char *argv[])
{
    char tmp[MAX_LEN], data[] = "success";
    sprintf(tmp, "Content-length: %d\r\n", strlen(data));
    sprintf(tmp, "%sContent-type: %s\r\n\r\n", tmp, "text/plain");
    Rio_writen(STDOUT_FILENO, tmp, strlen(tmp));    
    
    Rio_writen(STDOUT_FILENO, data, strlen(data));

    return 0;
}
