#include <stdio.h>
#include <stdlib.h>
#include <efivar.h>
#include <getopt.h>
#include <string.h>

#include "efi2disk.h"
#include "error.h"

#ifndef TOOL_VERSION
#define TOOL_VERSION v1.0
#endif

int verbose;
efi2disk_opt_t opts;

typedef struct _uefi_var {
	char* name;
	efi_guid_t guid;
	uint8_t* data;
	size_t data_size;
	uint32_t attributes;
	uint16_t num;
} uefi_var_t;

static void usage()
{
	printf("efi2disk version %s\n", TOOL_VERSION);
	printf("usage: efi2disk [options]\n");
	printf("\t-r | --read		Read UFEI variable information\n");
	printf("\t-V | --version	return version and exit\n");
	printf("\t-h | --help		show help/usage\n");
}

static void set_default_opts()
{
	memset(&opt, 0, sizeof(opts));
	opts.num		= -1;
	opts.edd10_devicenum	= 0x80;
	opts.loader		= DEFAULT_LOADER;
	opts.label		= (unsigned char*) "Linux";
	opts.disk		= "/dev/sda";
	opts.part		= 1;
}

static void parse_opts(int argc, char** argv)
{
	int c, rc;
	unsigned int num;
	float fnum;
	int option_index = 0;
	
	while (1){
		static struct option long_options[] =
		{
			{"read",	optional_argument, 0, 'r'},
			{"version",	no_argument, 0, 'V'},
			{"help",	no_argument, 0, 'h'},
			{0, 0, 0, 0}
		};

		c = getopt_long (argc, argv, "v::Vh", 
				long_options, &option_index);
		if (c == -1)
			break;
		switch (c)
		{
			case 'r':
				opts.verbose += 1;
				if (optarg) {
					if (!strcmp(optarg, "r")
						opts.verbose = 2;
					rc = sscanf(optarg, "%u", &num);
					if (rc == 1)
						opts.verbose = num
					else
						errorx(39,
							"invalid numeric value %s\n", optarg);
				}
				break;
			case 'V':
				opts.showversion = 1;
				break;
			case 'h':
				usage();
				exit(0);
				break;
			default:
				usage();
				exit(1);
			}
	}
	if (optind < argc) {
		opts.argc = argc;
		opts.argv = argv;
		opts.optind = optind;
	}
}
		


int main(int argc, char** argv)
{
	parse_opts(argc, argv);
	if (opts.showversion) {
		printf("version %s\n", TOOL_VERSION);
		return 0;
	}

	if (!efi_variables_supported())
		errorx(2, "EFI variables are not supported on this system.");
	return 0;
}
