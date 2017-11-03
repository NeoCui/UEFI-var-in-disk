#include <stdio.h>
#include <stdlib.h>
#include <efivar.h>
#include <efiboot.h>
#include <getopt.h>
#include <string.h>
#include <inttypes.h>

#include "list.h"
#include "efi2disk.h"
#include "unparse_path.h"

#ifndef TOOL_VERSION
#define TOOL_VERSION "v1.0"
#endif

#define DEFAULT_LOADER "hello"

int verbose;
static LIST_HEAD(entry_list);
static LIST_HEAD(blk_list);
efi2disk_opt_t opts;

typedef struct _var_entry{
	char*       name;
	efi_guid_t  guid;
	uint8_t*    data;
	size_t      data_size;
	uint32_t    attributes;
	uint16_t    num;
    list_t      list;
} var_entry_t;

#define ev_bits(val, mask, shift) \
    (((val) & ((mask) << (shift))) >> (shift))

static inline char* ucs2_to_utf8(const uint16_t* const chars, ssize_t limit)
{
    ssize_t i, j;
    char* ret;

    ret = alloca(limit * 6 + 1);
    if(!ret)
        return NULL;
    memset(ret, 0, limit * 6 + 1);

    for(i = 0, j = 0; chars[i] && i < (limit >= 0 ? limit : i+1); i++, j++){
        if(chars[i] <= 0x7f){
            ret[j] = chars[i];
        }else if(chars[i] > 0x7f && chars[i] <= 0x7ff){
            ret[j++] = 0xc0 | ev_bits(chars[i], 0x1f, 6);
            ret[j] = 0x80 | ev_bits(chars[i], 0x3f, 0);
        }else if(chars[i] > 0x7ff){
            ret[j++] = 0xe0 | ev_bits(chars[i], 0xf, 12);
            ret[j++] = 0x80 | ev_bits(chars[i], 0x3f, 6);
            ret[j] = 0x80 | ev_bits(chars[i], 0x3f, 0);
        }
    }
    ret[j] = '\0';
    return strdup(ret);
}

static void free_vars(list_t* head)
{
    list_t* pos;
    list_t* n;
    var_entry_t* entry;

    list_for_each_safe(pos, n, head){
        entry = list_entry(pos, var_entry_t, list);
        if(entry->data)
            free(entry->data);
        list_del(&(entry->list));
        free(entry);
    }
}

static void read_vars(char** namelist, list_t* head)
{
    var_entry_t* entry;
    int i, rc;
    
    if(!namelist)
        return;

    for(i=0; namelist[i] != NULL; i++){
        if(namelist[i]){
            entry = malloc(sizeof(var_entry_t));
            if(!entry){
                efi_error("malloc(%zd) failed", sizeof(var_entry_t));
                goto err;
            }
            memset(entry, 0, sizeof(var_entry_t));

            rc = efi_get_variable(EFI_GLOBAL_GUID, namelist[i],
                            &entry->data, &entry->data_size,
                            &entry->attributes);
            if(rc < 0){
                printf("Skipping unreadable variable \"%s\"",
                        namelist[i]);
                free(entry);
                continue;
            }

            entry->attributes = entry->attributes & ~(1 << 31);
            entry->name = namelist[i];
            entry->guid = EFI_GLOBAL_GUID;
            list_add_tail(&entry->list, head);
        }
    }
    return;
err:
    exit(1);
}

static void show_vars(const char* prefix)
{
    list_t* pos;
    var_entry_t* boot;
    const unsigned char* description;
    efi_load_option* load_option;
    efidp dp = NULL;
    unsigned char* optional_data = NULL;
    size_t optional_data_len = 0;

    list_for_each(pos, &entry_list){
        boot = list_entry(pos, var_entry_t, list);
        load_option = (efi_load_option*)boot->data;
        description = efi_loadopt_desc(load_option, boot->data_size);
        if(boot->name)
            printf("%s", boot->name);
        else
            printf("%s%04X", prefix, boot->num);

        if(opts.verbose){
            char* text_path = NULL;
            size_t text_path_len = 0;
            uint16_t pathlen;
            ssize_t rc;

            pathlen = efi_loadopt_pathlen(load_option, boot->data_size);
            dp = efi_loadopt_path(load_option, boot->data_size);
            rc = efidp_format_device_path(text_path, text_path_len, dp, pathlen);
            if(rc < 0)
                perror("Couldn't parse device path.");
            rc += 1;

            text_path_len = rc;
            text_path = calloc(1, rc);
            if(!text_path)
                perror("Could not parse device path");

            rc = efidp_format_device_path(text_path, text_path_len, dp, pathlen);
            if(rc < 0)
                perror("Could not parse device path");
            printf("\t%s", text_path);
            free(text_path);
            text_path_len = 0;

            rc = efi_loadopt_optional_data(load_option,
                                boot->data_size,
                                &optional_data,
                                &optional_data_len);
            if( rc<0 )
                perror("Could not parse optional data.");
            
            if(opts.unicode){
                text_path = ucs2_to_utf8((uint16_t*)optional_data, optional_data_len/2);
            }else{
                rc = unparse_raw_text(NULL, 0, optional_data,
                                optional_data_len);
                if(rc < 0)
                    perror("Couldn't parse optional data");
                rc += 1;
                text_path_len = rc;
                text_path = calloc(1, rc);
                if(!text_path)
                    perror("Could not parse optional data");
                rc = unparse_raw_text(text_path, text_path_len,
                                optional_data, optional_data_len);
                if(rc < 0)
                    perror("could not parse device path.");
            }
            printf("%s", text_path);
            free(text_path);
        }
        printf("\n");
        fflush(stdout);
    }
}

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
	memset(&opts, 0, sizeof(opts));
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

		c = getopt_long(argc, argv, "r::Vh", long_options, &option_index);
		if (c == -1)
			break;
		switch (c)
		{
			case 'r':
				opts.verbose += 1;
				if (optarg) {
					if (!strcmp(optarg, "r"))
						opts.verbose = 2;
					rc = sscanf(optarg, "%u", &num);
					if (rc == 1)
						opts.verbose = num;
					else
						perror("invalid numeric value.\n");
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

    verbose = opts.verbose;

//	if (!efi_variables_supported())
//		perror("EFI variables are not supported on this system.");
	return 0;
}
