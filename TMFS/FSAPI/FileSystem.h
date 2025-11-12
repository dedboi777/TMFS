#pragma once
#include <stdio.h>
#include "Hfunc.h"
#include "DiskEm.h"

#define Magic1          (0xDEAD)
#define Magic2          (0xCAFE)
#define InodesPerBlock  (16)
#define PtrPerInode     (8)
#define PtrPerBlock     (256)
#define ValidChars      ($1 "abcdefghijklmnopqrstuvwxyz0123456789_-")
#define ValidPathChars  ($1 "abcdefghijklmnopqrstuvwxyz0123456789_-/:.")
#define MaxLength        (16)

typedef int16 ptr;
typedef int8 bootsector[500];
typedef bool bitmap;

internal packed enum en_type{
    TypeNotValid = 0x00,
    TypeFile = 0x01,
    TypeDir = 0x03
};
typedef enum en_type type;

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

extern disk *DiskDescriptor[MaxDrives];

internal struct st_filename {
    int8 name[8];
    int8 ext[3];
} packed;
typedef struct st_filename filename;

internal struct st_inode {
    int8 validtype;
    int16 size;
    filename name;
    ptr indirect;
    ptr direct[PtrPerInode];
} packed; 
typedef struct st_inode inode;

internal union uni_fsblock {
    superblock super;
    int8 data[BlockSize];
    ptr pointers[PtrPerBlock];
    inode inodes[InodesPerBlock];
} packed;
typedef union uni_fsblock fsblock;

internal struct st_fileinfo {
    ptr idx;
    int16 size;
} packed;
typedef struct st_fileinfo fileinfo;

private struct st_path {
    filesystem *fs;
    int8 driveNo;
    filename target;
    int8 dirPath[MaxLength+1][9];

} packed;
typedef struct st_path path;

#define inrelease(f,p) (bool)releasein((f),(p))
#define filenametolow(x) tolowercase($1 (x))

public filesystem *format(disk*, bootsector*, bool);
internal bitmap *crtbitmap(filesystem*, bool);
internal int16 reservebitmap(filesystem *fs, bitmap*);
internal void releasebitmap(filesystem*, bitmap*, int16);
internal void show(filesystem*,bool); //bool is to show the bitmap or not
internal inode *searchin(filesystem *, ptr);
internal int8 *file2str(filename *);
internal filename *str2file(int8*);
internal filesystem *mountfs(int8);
internal void unmountfs(filesystem*);

internal ptr createin(filesystem*,filename*, type);
internal ptr reservein(filesystem*);
internal bool releasein(filesystem*, ptr);
internal ptr writein(filesystem*,inode*,ptr);
public fileinfo *filedetails(filesystem*, ptr);

private bool fvalidname(filename*,type);
private bool pvalidname(int8*);
private bool fvalidchar(int8);
private bool pvalidchar(int8);
private bool examinepath(int8* , path*, int16);

private ptr readdir(filesystem*, ptr, filename*);
private path *crtpath(int8*, filesystem*);