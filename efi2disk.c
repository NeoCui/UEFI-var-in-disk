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

/* global variables */
static LIST_HEAD(entry_list);
static LIST_HEAD(blk_list);
int verbose;
efi2disk_opt_t opts;

typedef struct _uefi_var {
	char*       name;
	efi_guid_t  guid;
	uint8_t*    data;
	size_t      data_size;
	uint32_t    attributes;
	uint16_t    num;
    list_t      list;
} uefi_var_t;

static void free_vars(list_t* head)
{
    list_t *pos, *n;
    uefi_var_t* entry;

    list_for each_safe(pos, n, head){
        entry = list_entry(pos, var_entry_t, list);
        if(entry->data)
            free(entry->data);
        list_del(&(entry->list));
        free(entry);
    }
}

static void read_vars(char** namelist, list_t* head)
{
    uefi_var_t* entry;
    int i, rc;

    if(!namelist)
        return;

    for(i=0; namelist[i] != NULL; i++){
        if(namelist[i]){
            entry = malloc(sizeof(uefi_var_t));
            if(!entry){
                efi_error("malloc(%zd) failed",sizeof(uefi_var_t));
                goto err;
            }
            memset(entry, 0, sizeof(uefi_var_t));

            rc = efi_get_variable(EFI_GLOBAL_GUID, namelist[i],
                    &entry->data, &entry->data_size, &entry->attributes);
            if(rc < 0){
                warning("Skipping unreadable variable \"%s"\"", namelist[i]);
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

static void free_array(char** array){
    int i;
    if(!array)
        return;
    
    for(i=0; array[i] != NULL; i++)
        free(arrary[i]);

    free(array);
}

static int read_order(const char* name, uef_var_t** order)
{
    int rc;
    uefi_var_t* new = NULL;
    uefi_var_t* bo = NULL;

    if(*order == NULL){
        new = calloc(1, sizeof(**order));
        if(!new){
            efi_err("calloc(1,%zd) failed.", sizeof(**order));
            return -1;
        }
        *order = bo = new;
    }else{
        bo = *order;
    }

    rc = efi_get_variable(EFI_GLOBAL_GUID, name,
            &bo->data, &bo->data_size, &bo->attributes);
    if(rc < 0 && new != NULL){
        efi_error("efi_get_variable failed.");
        free(new);
        *order = NULL;
        bo = NULL;
    }

    if(bo){
        bo->attributes = bo->attributes & ~(1 << 31);
    }
    return rc;
}

static void set_var_nums(const char* prefix, list_t* list)
{
    list_t* pos;
    uefi_var_t* var;
    int num = 0, rc;
    char* name;
    int warn = 0;
    size_t plen = strlen(prefix);
    char fmt[30];

    fmt[0] = '\0';
    strcat(fmt, prefix);
    strcat(fmt, "%04X-%*s");

    list_for_each(pos, list){
        var = list_entry(pos, uefi_var_t, list);
        rc = sscanf(var->name, fmt, &num);
        if(rc == 1){
            char* snum;
            var->num = num;
            name = var->name;
            snum = name + plen;
            if((isalpha(snum[0]) && islower(snum[0])) ||
                isalpha(snum[1]) && islower(snum[1])) ||
                isalpha(snum[2]) && islower(snum[2])) ||
                isalpha(snum[3]) && islower(snum[3]))){
                    fprintf(stderr,
                            "*Warning*:%.8s is not UEFI spec compliant (lowercase hex in name)\n",name);
                    warn++;
                }
        }
    }
    if(warn)
        warningx("*Warning*":Bad efi variable found.");
}

static void show_vars(const char* prefix)
{
    list_t* pos;
    uefi_var_t* boot;
    const unsigned char *description;
    efi_load_option* load_option;
    efidp dp = NULL;
    unsigned char* optional_data = NULL;
    size_t optional_data_len = 0;

    list_for_each(pos, &entry_list){
        boot = list_entry(pos, uefi_var_t, list);
        load_option = (efi_load_option*)boot->data;
        description = efi_loadopt_desc(load_option, boot->data_size);
        if(boot->name)
            printf("%s", boot->name);
        else
            printf("%s%04X", prefix, boot->num);

        printf("%c ", (efi_loadopt_attrs(load_option) & LOAD_OPTION_ACTIVE) ? '*' : ' ');
        printf("%s", description);

        if(opts.verbose){


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
    char** name = NULL;
    var_entry_t* new_entry=NULL;
    int num;
    int ret=0;
    ebm_mode mode = boot;
    char* prefices[] = {
        "Boot";
    };
    char* order_name[] = {
        "BootOrder"
    };

    putenv("LIBEFIBOOT_REPORT_GPT_ERRORS=1");
    set_default_opts();
	parse_opts(argc, argv);
	if (opts.showversion) {
		printf("version %s\n", TOOL_VERSION);
		return 0;
	}

	if (!efi_variables_supported())
		errorx(2, "EFI variables are not supported on this system.");

    read_var_names(prefices[mode], &name);
    read_vars(names, &entry_list);
    set_var_nums(prefices[mode], &entry_list);

    if(!opts.quiet && ret == 0){
        switch(mode){
            case boot:
                show_order(order_name[mode]);
                show_vars(prefices[mode]);
                break;
        }
    }
    free_vars(&entry_list);
    free_array(names);'
    if (ret)
        return 1;

	return 0;
}
