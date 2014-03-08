#include "rio.h"


static void unixt_error(const char *err)
{
    perror(err);
}

/* rio_readn instead of read, but safe */
size_t rio_readn(int fd, void *usrbuf, size_t size)
{
    char *bufp  = usrbuf;
    int   leftn = size;
    int   readn;
    
    while (leftn > 0) {
        if ((readn = read(fd, bufp, size)) < 0) {
            if (readn == EINTR)
                readn = 0;
            else
                return readn;
        
        } else if (readn == 0) {
            break;
            
        } else {
            leftn -= readn;
            bufp  += readn;
        }
    }
    
    return size - leftn;
}

/* rio_readn instead of read, but safe */
int rio_writen(int fd, void *usrbuf, size_t size)
{
    char *bufp  = usrbuf;
    int   leftn = size;
    int   writen;
    
    while (leftn > 0) {
        if ((writen = write(fd, bufp, size)) <= 0) {
            if (writen == EINTR) 
                writen = 0;
            else
                return writen;
                
        } else {
            leftn -= writen;
            bufp  += writen;
        }
    }
    return size;
}

void rio_init(rio_t *r, int fd)
{
    r->rio_fd  = fd;
    r->rio_cnt = 0;
    r->rio_bufp = r->rio_buf;
}

size_t rio_read(rio_t *r, void *usrbuf, size_t size)
{
    int cnt;
    
    if (r->rio_cnt <= 0) {
        r->rio_cnt = rio_readn(r->rio_fd, r->rio_buf, sizeof(r->rio_buf));
        if (r->rio_cnt < 0)
            return r->rio_cnt;
        
        r->rio_bufp = r->rio_buf;
    }
    
    cnt = min(size, r->rio_cnt);
    memcpy(usrbuf, r->rio_bufp, cnt);
    r->rio_cnt  -= cnt;
    r->rio_bufp += cnt;
    
    return cnt;
}

size_t rio_readnb(rio_t *r, void *usrbuf, size_t size)
{
    char  *bufp   = usrbuf;
    size_t leftn  = size;
    size_t readn;
    
    while (leftn > 0) {
        if ((readn = rio_read(r, usrbuf, size)) < 0)
            return readn;
            
        else if (readn == 0) 
            break;
            
        leftn -= readn;
        bufp  += readn;
    }
    
    return size - leftn;
}

size_t rio_readlineb(rio_t *r, void *usrbuf, size_t maxlen)
{
    int i, cnt;
    char c;
    char *bufp = usrbuf;
    
    // '\n' need one slot, so maxlen - 1
    for (i = 0; i < maxlen - 1; i++) {
        if ((cnt = rio_read(r, &c, 1)) == 1) {
            *bufp++ = c;
            if (c == '\n')
                break;
                
        } else if (cnt == 0) {
            if (i == 0)  // no data
                return 0;   
            else
                break;   // read some data
                
        } else {
            return cnt;
        }
    }
    
    return i + 1;
}

size_t Rio_readn(int fd, void *usrbuf, size_t size)
{
    int cnt;
    if ((cnt = rio_readn(fd, usrbuf, size)) < 0)
        unix_error("Rio_read error\n");
    return cnt;
}

int   Rio_writen(int fd, void *usrbuf, size_t size)
{
    int cnt;
    if ((cnt = rio_writen(fd, usrbuf, size) != size))
        unix_error("Rio_writen error\n");
    return cnt;
}

void Rio_init(rio_t *r, int fd)
{
    rio_init(r, fd);
}

size_t Rio_readnb(rio_t *r, void *usrbuf, size_t size)
{
    int cnt;
    if ((cnt = rio_readnb(r, usrbuf, size)) < 0)
        unix_error("Rio_readnb error\n");
    return cnt;
}

size_t Rio_readlineb(rio_t *r, void *usrbuf, size_t maxlen)
{
    int cnt;
    if ((cnt = rio_readlineb(r, usrbuf, maxlen)) < 0)
        unix_error("Rio_readlineb error\n");
    return cnt;
}