#ifndef _EFI2DISK_H
#define _EFI2DISK_H

typedef struct {
	int argc;
	char** argv;
	int optind;
	char* disk;
	char* loader;
	unsigned char* label;
	int edd_version;
	uint32_t edd10_devicenum;
	int num;
	int verbose;
	unsigned int showversion:1; 
} efi2disk_opt_t opts;

extern efi2disk_opt_t opts;

#endif
