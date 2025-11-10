#include "command.h"
#include <string.h>

void usage(char*);
void usage_format(char*);
void cmd_format(char*,char*);

// It is not complete yet.
void cmd_format(char *arg1, char *arg2){
    int8 driveNo;
    int8 *drivestr;
    bool bootable;
    char force;
    bool bforce;
    disk *dd;
    filesystem *fs;

    if (!arg1) 
        usage_format("osutil");
    else if(!arg2) {
        bootable = false;
        drivestr = $1 arg1;
    }
    else {
        if ((*arg1 == '-') && (arg1[1] == 's')) {
            bootable = true;
            drivestr = $1 arg2;
        }
        else    
            usage_format("osutil");
    }
    
    switch (*drivestr) {
        case 'c':
            driveNo = 1;
            break;
        case 'd':
            driveNo = 2;
            break;
        default:
            usage_format("osutil");
            break;
    }
    if(bootable) {
        fprintf(stderr, "Bootable currently not supported\n");
        exit(-1);
    }

    printf("This will FORMAT and ERASE disk %s\nForce?(Y/N): ", drivestr);
    fflush(stdout);

    scanf("%c", &force);
    bforce = ((force == 'y') || (force == 'Y')) ? true : false;
    if(!bforce){ return;}
    printf("Formatting disk %s\n", drivestr);
    dinit();
    dd = DiskDescriptor[driveNo-1];
    

    if (!dd) {
        fprintf(stderr, "Bad disk\n");
        exit(-1);
    }
    
    
    fs = fsformat(dd, (bootsector *)0, bforce);
    
    if (fs){}
       // fsshow(fs, true);
    else 
        fprintf(stderr, "Formatting failed\n");
    
    return;
}

void usage(char* arg) {
    fprintf(stderr, "Usage: %s <command> [arguments]\n"
       "Available commands:  format\n",
        arg);

    exit(-1);

}

void usage_format(char*arg) {
    fprintf(stderr, "Usage: %s (-s) <driveNo>\nExample: %s format c:\n",
        arg, arg);

    exit(-1);

}

int main(int argc, char *argv[]) {
    char *cmd, *arg1, *arg2;
    if (argc < 2){
        usage(*argv);
    }
    else if (argc == 2){
        arg1 = arg2 = 0;
    }
    else if (argc == 3) {
        arg1 = argv[2];
        arg2 = 0;
    }
    else {
        arg1 = argv[2];
        arg2 = argv[3];
    }
    cmd = argv[1];

    if(!strcmp(cmd, "format"))
        cmd_format(arg1, arg2);
    else   
        usage(*argv) ;

    return 0;
}
