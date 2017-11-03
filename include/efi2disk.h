#ifndef _EFI2DISK_H
#define _EFI2DISK_H

typedef struct {
	int argc;
	char** argv;
	int optind;
	char* disk;
	char* loader;
	unsigned char* label;
    uint32_t part;
	int edd_version;
	uint32_t edd10_devicenum;
	int num;
	int verbose;
    unsigned int unicode:1;
	unsigned int showversion:1; 
} efi2disk_opt_t;

extern efi2disk_opt_t opts;

#endif
