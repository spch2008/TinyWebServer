#include <stdio.h>
#include "rio.h"
#include <unistd.h>
#include <string.h>

#define MAX_LEN 100



int parse_param(char *param, char *a, char *b)
{
    if (param == NULL || *param == '\0')
        return -1;
        
    char *left = param;
    char *right = strchr(param, '&');
    
    if (right == NULL)
        return -1;
        
    *right = '\0';
    right += 1;
    
    left = strchr(left, '=');
    right = strchr(right, '=');
    
    if (left == NULL || right == NULL)
        return -1;
        
    left  += 1;
    right += 1;
    
    strcpy(a, left);
    strcpy(b, right);
    
    return 0;
}

int main(int argc, char *argv[])
{
/*
    char tmp[MAX_LEN], data[] = "success";
    sprintf(tmp, "Content-length: %d\r\n", strlen(data));
    sprintf(tmp, "%sContent-type: %s\r\n\r\n", tmp, "text/plain");
    Rio_writen(STDOUT_FILENO, tmp, strlen(tmp));    
    
    Rio_writen(STDOUT_FILENO, data, strlen(data));
*/
    char *param, param_a[20], param_b[20], result[20];
    int res, a, b, sum;

    param = getenv("QUERY_STRING");
    res   = parse_param(param, param_a, param_b);
    
    if (res != 0) {
        exit(0);
    }
    
    sscanf(param_a, "%d", &a);
    sscanf(param_b, "%d", &b);
    
    sum = a + b;
    sprintf(result, "SUM: ");
    sprintf(result, "%s %d\n", result, sum);
    
    char tmp[MAX_LEN];
    sprintf(tmp, "Content-length: %d\r\n", strlen(result));
    sprintf(tmp, "%sContent-type: %s\r\n\r\n", tmp, "text/plain");
    Rio_writen(STDOUT_FILENO, tmp, strlen(tmp));    
    
    Rio_writen(STDOUT_FILENO, result, strlen(result));
    
    return 0;
}
