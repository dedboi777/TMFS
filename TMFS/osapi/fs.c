#include <fs.h>
#include <mystd.h>
#include <string.h>

internal bitmap *mkbitmap(filesystem *fs, bool scan){
    bitmap *bm;
    int16 size;
    int16 n,x;
    fsblock block;
    bool ret;
    int16 index;
    bool valid;

    if(!fs)
        return (bitmap *) false;

    size = (fs->dd->blocks / 8);
    if(fs->dd->blocks % 8)
        size++;
    bm = (bitmap *)alloc(size);
    if (!bm)
        return (bitmap *)false;
    zero($1 bm, size);
    
    if(!scan) 
        return bm; 

    index=0;
    for (n=1; n<=(fs->metadata.inodeblocks+1); n++){
        zero($1 &block, BlockSize);
        ret = dread(fs->dd, &block, n);
        if(!ret){
            destroy(bm);
            return (bitmap *)false;
        }
        for(x=0; x<InodesPerBlock; x++){
            valid = (bool)(block.inodes[x].validtype & 0x01);
            if(valid)
                setbit($1 bm, index, true);
            else
                setbit($1 bm, index, false);
            
            index++;
        }
    }

    return bm;
}

internal int16 bitmapalloc(filesystem *fs, bitmap *bm){
    int16 n;
    int16 bl;

    if(!bm || !fs)
        return 0;
    
    for(n=0; n<=fs->dd->blocks; n++)
        if(!getbit($1 bm, n)) {
            setbit($1 bm, n, true);
            bl = (n+1);

            return bl;
        }
    return 0;

}

internal void bitmapfree(filesystem *fs, bitmap *bm, int16 bl){
    int16 n;

    if(!bm || !fs)
        return ;

    n = (bl-1);
    setbit($1 bm, n ,false);

    return;
}

public filesystem *fsformat(disk *dd, bootsector *mbr, bool force) {
    filesystem *fs;
    int16 size;
    int16 inodeblocks;
    int16 blocks;
    superblock super;
    inode idx;
    block idxblock;
    bool allowed;
    int16 n;
    fsblock fsb;
    bitmap *bm;

    if(!dd)
        reterr(ErrNotMounted);
    if(openfiles(dd)){
        if(!force)
            reterr(ErrBusy);
    }
    else
        closeallfiles(dd);


    blocks = dd->blocks;
    inodeblocks = (blocks / 10);
    if ((blocks % 10))
        inodeblocks++;

    idx.validtype = TypeDir;
    idx.size = 0;
    zero($1 &idx.name, 11);
    idx.indirect = 0;
    size = (sizeof(ptr)*PtrPerInode);
    zero($1 &idx.direct, size);

    super.magic2 = Magic2;
    super.magic1 = Magic1;
    super.inodes = 1;
    super.inodeblocks = inodeblocks;
    super.blocks = blocks;
    super._ = 0;

    if(mbr)
        copy($1 &super.boot, $1 mbr, 500);
    else    
        zero($1 &super.boot, 500);
    
    allowed = dwrite(dd, &super, 1);
    if(!allowed)
        reterr(ErrIO);

    zero($1 &idxblock, BlockSize);
    copy($1 &idxblock, $1 &idx, sizeof(idx));
    allowed = dwrite(dd, &idxblock, 2);
    if (!allowed)
        reterr(ErrIO);

    zero($1 &fsb, BlockSize);
    for(n=0; n<inodeblocks; n++){
        allowed = dwrite(dd, &fsb, (n+3));
        if(!allowed)
            reterr(ErrIO);
    }

    size = sizeof(struct s_filesystem);
    fs = (filesystem *)alloc(size);
    zero($1 fs, size);
    if (!fs)
        reterr(ErrNoMem);

    fs->driveNo = dd->driveNo;
    fs->dd = dd;
    copy($1 &fs->metadata,$1 &super, BlockSize);

    bm = mkbitmap(fs, false);
    size = 
        1 //superblock
      + fs->metadata.inodeblocks; // # of inode blocks
    for(n=0; n<=size; n++)
        setbit($1 bm, n ,true);

    fs->bitmap = bm;
    
    fsshow(fs, true);

    return fs;
}

internal void fsshow(filesystem *fs, bool showbm) {
    int8 drivechar; //maps the drive number
    ptr inodeNo, n; 
    inode *ino;

    if (!fs) 
        return;

    drivechar = (fs->driveNo == 1) ? (int8)'c':
     (fs->driveNo == 2) ? (int8)'d': (int8)'?';

    printf("Disk 0x%.02hhx, mounted on %c:\n",
        (char)fs->driveNo, (char)drivechar);
    printf(" %d total blocks, 1 super block and %d inode blocks\n"
        "containing %d inodes\n\n", 
        $i fs->metadata.blocks, $i fs->metadata.inodeblocks, $i fs->metadata.inodes);
    
    for (inodeNo = 0; inodeNo<fs->metadata.inodes; inodeNo++) {
        ino = findinode(fs, inodeNo);
     
        if (ino->validtype & 0x01)
            printf("Inode %d is valid (type=%s)\n"
                "name is %s\n"
                "%d size in bytes\n",
                $i inodeNo,
                (ino->validtype == TypeFile) ?
                "file" :
                (ino->validtype == TypeDir) ?
                "dir" :
                "unknown",
                (!inodeNo) ?
                "/" :
                $c file2str(&ino->name),
                $i ino->size 
            );
    }
    printf("\n");

    if(showbm) {
        printf("bitmap=");
        fflush(stdout);

        for(n=0; n<fs->dd->blocks; n++){
            if(getbit($1 fs->bitmap, n)){
                printf("1");
            }
            else
                printf("0");

            if(n>0){
                if(!(n%8))
                    printf(" ");
                if (!(n%64))
                    printf("\n");
            }
        }
        printf("\n\n");

    }
    return;
}

internal inode *findinode(filesystem *fs, ptr idx) {
    fsblock bl;
    int16 n, size;
    bool res;  //result
    inode *ret;
    ptr x, y;

    if (!fs)
        return (inode *)0;

    ret = (inode *)0;
    for (n=0, x=2; x<(fs->metadata.inodeblocks); x++) {
        zero($1 &bl, BlockSize);
        res = dread(fs->dd, $1 &bl.data, x);
        if (!res)
            return ret;
        
        for (y=0; y<InodesPerBlock; y++, n++) {
            if (n == idx) {
                size = sizeof(struct s_inode);
                ret = (inode *)alloc(size);
                if(!ret)
                    return (inode *)0;

                zero($1 ret, size);
                copy($1 ret, $1 &bl.inodes[y], size);
                x = $2 fs->metadata.inodeblocks;

                break;
            }
        }
    }

    return ret;
}

internal int8 *file2str(filename *fname) {
    static int8 buf[16];
    int8 *p;
    int16 n;

    if (!fname)
        return $1 0;

    zero(buf, 16);
    copy($1 &buf, fname->name, $2 8);
    if (!(*fname->ext)){
        p = buf;
        return p;
    }
    n = stringlen(buf);
    buf[n++] = '.';
    p = buf + n;

    copy(p, fname->ext, $2 3);
    p = buf;

    return p;

}

internal filename *str2file(int8 *str) {
    int8 *p;
    static filename name;
    filename *fnptr;
    int16 size;
    path* filepath;

    errnumber = ErrNoErr;
    
    if (!str)
        reterr(ErrArg);

    size = sizeof(struct s_path);
    zero($1 &name, size);
    p = findcharl(str, '.');
    if (!p)
        copy($1 &name.name, $1 str, $2 8);
    else {
        copy($1 &name.ext, $1 (p+1), $2 3);
        *p = (int8)0;
        stringcopy($1 &name.name, $1 str, $2 8);
    }
    fnptr = &name;

    return fnptr;
}                                    

public filesystem *FSdescriptor[Maxdrive];
internal filesystem *fsmount(int8 driveNo) {
    ptr idx;
    disk *dd;
    filesystem *fs;
    int16 size;

    if (driveNo > Maxdrive)
        return (filesystem *)0;

    idx = (driveNo - 1);
    dd = DiskDescriptor[idx];
    if(!dd) 
        return (filesystem *)0;

    size = sizeof(struct s_filesystem);
    fs = (filesystem *)alloc(size);
    if (!fs)
        return (filesystem *)0;
    zero($1 fs, size);

    fs->driveNo = driveNo;
    fs->dd = dd;
    fs->bitmap = mkbitmap(fs, true);
    if (!fs->bitmap){
        destroy(fs);
        return (filesystem *)0;
    }
    
    if (!dread(fs->dd, &fs->metadata, 1)) {
        destroy(fs);
        return (filesystem *)0; 
    }
    kprintf("Mounted disk 0x%x on drive %c:",
        driveNo, (driveNo == 1) ?
                    'c':
                 (driveNo == 2) ?
                    'd':
                    '?'
    );

    FSdescriptor[idx] = fs;

    return fs;
}

internal void fsunmount(filesystem *fs) {
    ptr idx;
    int16 driveNo;
    
    if(!fs) 
        return;
    
    driveNo = fs->dd->driveNo;
    idx = (driveNo-1);
    FSdescriptor[idx] = (filesystem *)0;
    destroy(fs);

    kprintf("Unmounted drive %c:",
        (driveNo == 1) ?
                    'c':
        (driveNo == 2) ?
                    'd':
                    '?'
    );

    return;
}

internal ptr fssaveinode(filesystem *fs, inode *ino, ptr idx) {
    fsblock bl;
    ptr blockNo;
    int16 size;
    bool ret;
    ptr mod;

    if(!fs || !ino)
        return 0;

    blockNo = ((idx/16)+2);
    mod = (idx%16);
    printf("blockNo=%d\n", $i blockNo);
    ret = dread(fs->dd, &bl.data, blockNo);
    if (!ret)
        return 0;
    size = sizeof(struct s_inode);
    copy($1 &bl.inodes[mod], $1 ino, size);

    ret = dwrite(fs->dd, &bl.data, blockNo);
    if (!ret)
        return 0;

    return blockNo;
}

internal ptr inalloc(filesystem *fs) {
    ptr idx, n, max;
    inode *p;
    if(!fs)
        return 0;

    max = (fs->metadata.inodeblocks * InodesPerBlock);
    for(idx=n=0; n<max; n++) {
        p = findinode(fs, n);
        if (!p)
            break;
        
        if (!(p->validtype & 0x01)) {
            idx = n;
            p->validtype = 1;
            if(!fssaveinode(fs, p, idx)) {
                idx = 0;
                break;
            }
            fs->metadata.inodes++;
            (void)dwrite(fs->dd, &fs->metadata, 1);

            break;
        }

    }
    if(!p)
        destroy(p);

    return idx;
}

internal bool inunalloc(filesystem *fs, ptr idx) {
    inode *p;
    ptr blockNo;
    if (!fs)
        return false;

    p = findinode(fs, idx);
    if (!p)
        return false;

    p->validtype = 0;
    blockNo = fssaveinode(fs, p , idx);
    destroy(p);

    return(blockNo) ?
            true:
            false;

}

internal ptr increate(filesystem *fs, filename *name, type filetype) {
    ptr idx;
    inode *ino;
    bool valid;
    int16 size;

    errnumber = ErrNoErr;

    if (!fs || !name || !filetype)
        reterr(ErrArg);

    valid = validfname(name, filetype);
    if (!valid)
        reterr(ErrFilename);

    idx = inalloc(fs);
    if (!idx)
        reterr(ErrInode);

    size = sizeof(struct s_filename);
    ino = (inode *)alloc(size);
    if (!ino)
        reterr(ErrNoMem);

    ino->validtype = filetype;
    ino->size = 0;

    size = sizeof(struct s_filename);
    copy($1 &ino->name, $1 name, size);

    valid = (bool)fssaveinode(fs, ino, idx);  
    destroy(ino);
    if (!valid) {
        reterr(ErrIO);
    }

    return idx;

}

//public fileinfo *fsstat( path* pathname)
public fileinfo *fsstat(filesystem *fs, ptr idx) {
    inode *ino;
    fileinfo *info;
    int16 size;

    if (!fs)
        reterr(ErrArg);

    ino = findinode(fs, idx);
    if (!ino)
        reterr(ErrInode);

    size = sizeof(struct s_fileinfo);
    info = (fileinfo *)alloc(size);
    if (!info)
        reterr(ErrNoMem);
    zero($1 info, size);

    info->idx = idx;
    info->size = ino->size;
    free(ino);

    return info;

}

private ptr readdir(filesystem* fs, ptr haystack, filename *needle){
    inode *p, *p2;
    ptr idx, n, blockNo;
    fsblock bl;
    bool ret;
 
    errnumber = ErrNoErr;
    if(!fs || !needle) {
        reterr(ErrArg);
    }
    
    p = findinode(fs, haystack);
    if (!p)
        reterr(ErrInode);
    
    if(p->validtype != TypeDir) {
        destroy(p);
        reterr(ErrBadDir);
    }

    filename2low(needle);

    for(n=0; n<PtrPerInode; n++) {
        idx = p->direct[n];
        if (!idx)
            continue;
        
        p2 = findinode(fs, idx);
        if (!p2)
            continue;

        if (cmp($1 needle, $1 &p2->name, $2 11)) {
            destroy(p);
            destroy(p2);

            return idx;
        }
        destroy(p2);
    }

    if (!p->indirect) {
        destroy(p);
        reterr(ErrNotFound);
    }

    blockNo = p->indirect;
    zero($1 &bl, BlockSize);
    ret = dread(fs->dd, &bl.data, blockNo);
    if (!ret) {
        destroy(p);
        reterr(ErrNotFound);
    }
    
    for(n=0; n<PtrPerBlock; n++) {
        idx = bl.pointers[n];
        if (!idx)
            continue;
        
        p2 = findinode(fs, idx);
        if (!p2)
            continue;

        if (cmp($1 needle, $1 &p2->name, $2 11)) {
            destroy(p);
            destroy(p2);

            return idx;
        }
        destroy(p2);
    }
    free(p);

    reterr(ErrNotFound);

}

private bool validchar(int8 c) {
    int8 *p;
    bool ret;

    ret = false;
    for(p=ValidChars; *p; p++) 
        if (c == *p) {
            ret = true;
            break;
        }
    return ret;
}

private bool validchar_(int8 c) {
    int8 *p;
    bool ret;

    ret = false;
    for(p=ValidPathChars; *p; p++) 
        if (c == *p) {
            ret = true;
            break;
        }
    return ret;
}


private bool validfname(filename *name, type filetype) {
    int16 n;
    bool ret;
    int8 *p;

    if(!name || !filetype) {
        reterr(ErrArg);
    }
    else if ((filetype == TypeDir) && (*name->ext))
        reterr(ErrFilename);

    n = (filetype == TypeFile) ?
            11:
            8;

    for (ret=true, p = $1 name; n; n--, p++)
        if (!validchar(*p)){
            ret = false;
            break;
        }
        
    return ret;
}

private bool validpname(int8 *name) {
    int16 n;
    bool ret;
    int8 *p;

    if(!name) {
        reterr(ErrArg);
    }
    n = stringlen(name);

    for (ret=true, p = $1 name; n; n--, p++)
        if (!validchar_(*p)){
            ret = false;
            break;
        }
        
    return ret;
}

private bool parsepath(int8 *str, path *pptr, int16 idx_) {
    int8 *p;
    int8 str_[255];
    int16 idx;

    idx = idx_;
    if (!str || !pptr)
        return false;

    if (idx >= MaxDepth){
        *pptr->dirPath[idx] = (int8)0;
        return true;
    }
    p = findcharl($1 str, (int8)'/');
    if (!p) {
        stringcopy($1 pptr->dirPath[idx], $1 str, $2 8);
        idx++;
        *pptr->dirPath[idx] = (int8)0;

        return true;
    }

    *p = 0;
    stringcopy($1 pptr->dirPath[idx], $1 str, $2 8);
    p++;
    idx++;
    

    return parsepath(p, pptr, idx);
}
// THIS FUNCTION IS NOT FINISHED AND THE FUNCTIONS IT USES MAY HAVE FLAWS
// (THERE IS STILL WORK TO DO)
/*
 private path *mkpath(int8 *str, filesystem *fs) {
    static path path_;
    path *pptr;
    int8 c, driveNo;
    int8 *p;
    filename *name;
    int16 size;
    bool ret;

    errnumber = ErrNoErr;
    if (!str || (*str))
        reterr(ErrArg);

    if (str[1] == ':') {
        c = low(*str);
        if ((c < (int8)'a') || (c > (int8)'z'));
            reterr(ErrDriveNo);

        driveNo = (c - 0x62);

        str+=2;;
    }
    else if (!fs){
        reterr(ErrArg);
    }
    else 
        driveNo = fs->dd->driveNo;

    if (!validpname(str))
        reterr(ErrPath);
    path_.fs = FSdescriptor[driveNo-1];
    if (!path_.fs)
        path_.fs = fs;
    if (!path_.fs)
        reterr(ErrDriveNo);
    if (!driveNo)
        reterr(ErrDriveNo);

    p = findcharr(str, (int8)'/');
    if (!p)
        reterr(ErrPath);
    p++;
    
    name = str2file(p);
    if (!name)
        reterr(ErrFilename);

    size = sizeof(struct s_filename);
    stringcopy($1 &path_.target, $1 name, size);

    p--;
    *p = (int8)0;
    str++;

    p = findcharl(str, (int8)'/');
    if (!p) {
        stringcopy($1 &path_.dirPath[0], $1 str, $2 8);
    }
    else {
        ret = parsepath($1 str, &path_, $2 0);
        if (!ret)
            reterr(ErrPath);
    }
   
    pptr = &path_;

    return pptr;

}
*/