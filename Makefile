SRCDIR = $(realpath .)

SUBDIR_CFLAGS = -I$(SRCDIR)/include

BINTARGETS = efi2disk
TARGETS = $(BINTARGETS)

all : $(TARGETS)
ALL_SOURCES = efi2disk.c

all : $(ALL_SOURCES)
	$(CC) SOURCES="$(ALL_SOURCES)" SUBDIR_CFLAGS="$(SUBDIR_CFLAGS)"

clean : 
	@rm -rfv *.o *.a *.so $(TARGETS)
	@rm -rfv .*.d

.PHONY : all deps clean

