#ifndef _RIO_H_
#define _RIO_H_

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define RIO_BUFSIZE 8192
typedef struct
{
    int   rio_fd;
    int   rio_cnt;
    char *rio_bufp;
    char  rio_buf[RIO_BUFSIZE];
}rio_t;

static void unixt_error(const char *err);

static size_t rio_readn(int fd, void *usrbuf, size_t size);
static int   rio_writen(int fd, void *usrbuf, size_t size);
 
static void   rio_initb(rio_t *r, int fd);
static size_t rio_read(rio_t *r, void *usrbuf, size_t size);
static size_t rio_readnb(rio_t *r, void *usrbuf, size_t size);
static size_t rio_readlineb(rio_t *r, void *usrbuf, size_t maxlen);

size_t Rio_readn(int fd, void *usrbuf, size_t size);
int   Rio_writen(int fd, void *usrbuf, size_t size);
void Rio_init(rio_t *r, int fd);
size_t Rio_readnb(rio_t *r, void *usrbuf, size_t size);
size_t Rio_readlineb(rio_t *r, void *usrbuf, size_t maxlen);

#endif
