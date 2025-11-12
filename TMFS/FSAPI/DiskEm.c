#include "DiskEm.h"


internal int8 mounted;

internal void dshow(disk *dd){
    if (dd)
        printf(
                "drive 0x%.02hhx\n"
                "fd=%d\n"
                "NumOfBlocks%d\n",
                    (char)dd->driveNo, $i dd->fd, $i dd->blocks
            );
    return;
}

typedef int8 bootsector[500];

internal struct st_superblock {
    bootsector boot;
    int16 _;
    int16 blocks;
    int16 inodeblocks;
    int16 inodes;
    int16 magic1;
    int16 magic2;
} packed;
typedef struct st_superblock superblock;

internal struct st_filesystem {
    int8 driveNo;
    disk *dd;
    bool *bitmap;
    superblock metadata;
} packed;
typedef struct st_filesystem filesystem;

internal filesystem *mountfs(int8);
public disk *DiskDescriptor[MaxDrives];

public void dinit() {
    int8 n;

    mounted = 0;
    for (n=1; n<=MaxDrives; n++)
        *(DiskDescriptor+(n-1))= dmount(n);
    
    //not finished
    if (*(DiskDescriptor))
        mountfs(1);

    //for (n=1; n<=MaxDrives; n++)
      //  dunmount(DiskDescriptor[n-1]);
    
    return;
}

internal void dunmount(disk *dd) {
    int8 x;
    if (!dd)
        return;
    
    close($i dd->fd);
    x = ~(dd->driveNo) & mounted;
    mounted = x;
    release(dd);

    return;
}

internal disk *dmount(int8 driveNo){
    disk *dd;
    int16 size;
    int8 *file;
    signed int tmp;
    struct stat sbuf;

    if ((driveNo==1) || (driveNo==2));
    else return (disk *)0;

    if(mounted & driveNo)
        return (disk *)0;

    size = sizeof(struct st_disk);
    dd = (disk *)malloc($i size);
    if(!dd)
        return (disk *)0;

    zero($1 dd, size);
    file = strnum(BasePath, driveNo);
    tmp = open($c file, O_RDWR);
    if (tmp < 3){
        free(dd);
        return (disk *)0;
    }
    dd->fd = $4 tmp;
    tmp = fstat( dd->fd, &sbuf);
    if(tmp ||  !sbuf.st_blocks){
        close(dd->fd);
        free(dd);

        return (disk *)0;
    }
    dd->blocks = $2 (sbuf.st_blocks-1);
    dd->driveNo = driveNo;
    mounted |= driveNo;

    return dd;
}


// not finished
internal int16 openfile(disk *dd) {
    return 0;
}
internal void closeallfiles(disk *dd) {
    return;
}

