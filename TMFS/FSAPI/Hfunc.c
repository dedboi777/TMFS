#define Access
#include "Hfunc.h"
#include "Mystd.h"
#include "DiskEm.h"

private int8 fds[256];

internal int8 *strnum(int8* str, int8 num) {
    static int8 buf[256];
    int16 n;
    int8 c;

    n = stringlen(str);
    if (!n)
        return str;
    else if (n > 250)
        return str;
    else if (num > 9)
        return str;
    
    zero($1 &buf, 256);
    copy($1 &buf, str, n);

    c = num + 0x30;
    buf[n++] = c;
    buf[n] = 0; // for the trailing zero \0 

    return buf;
}
/*
fd = 0 -> error
fd = 1 -> stdin
fd = 2 -> stdout
*/
 
private bool isopen(fd file) {
    signed int posixfd;
    struct stat _; //wont be used, was made for fstat to work
    
    if (file < 3)
        return false;

    posixfd = getposixfd(file);
    if (!posixfd)
        return false;
    if (fstat(posixfd, &_) == -1)
        return false;

    return true;
}
public bool load(fd file, int8 c) {
    int8 buf[2];
    signed int n;
    signed int posixfd;

    assert_initialized();

    if (file > 1)
        if (!isopen(file))
            reterr(InvFdErr);
    posixfd = getposixfd(file);
    if (!posixfd)
        reterr(InvFdErr);  
    
    posixfd = (posixfd == 1) ? 0
        : (posixfd == 2) ? 1
        : (posixfd);

    *buf = *(buf+1) = (int8)0;
    *buf = c;
    n = write(posixfd, $c buf, 1);
    if ( n != 1) 
        reterr(IOErr);
    return true;
}
public int8 store(fd file){
      int8 buf[2];
    signed int n;
    signed int posixfd;
    if (file > 2)
        if (!isopen(file))
            reterr(InvFdErr);

    posixfd = getposixfd(file);
    if (!posixfd)
        reterr(InvFdErr);  
    
    posixfd = (posixfd == 1) ? 0
        : (posixfd == 2) ? 1
        : (posixfd);

    *buf = *(buf+1) = (int8)0;
    n = read(posixfd, $c buf, 1);
    if ( n != 1) 
        reterr(IOErr);
    return (int8)*buf;
}

public void init() {
    errnumber = (int8)0;
    setupfds();
    initialized = true;

    return;
}

private void setupfds() {
    zero(fds, sizeof(fds));

    fds[0] = 1;
    fds[1] = 2;

    return;
}
