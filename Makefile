#
# make file for tffs
#
# knightray@gmail.com
# 10/27 2008

# build target definiation.
LIB_TARGET  := libtffs.a
TEST_TARGET := test_tffs
TSH_TARGET  := tsh
TFFS_TARGET := tffs

# source file definiation
LIBSRC      := src/hai_file.c src/initfs.c src/debug.c src/fat.c src/dir.c \
	src/dirent.c src/common.c src/file.c src/crtdef.c src/cache.c
TESTSRC     := src/main.c
TSHSRC      := tshell/tsh.c
TFFSSRC     := tshell/tffs.c

# object files definiation
LIBOBJ      := $(LIBSRC:.c=.o)
TESTOBJ     := $(TESTSRC:.c=.o)
TSHOBJ      := $(TSHSRC:.c=.o)
TFFSOBJ     := $(TFFSSRC:.c=.o)

# makefile parameters
CFLAGS      += -Wall -g -I inc
LDFLAGS     += -L. -ltffs

all:$(LIB_TARGET) $(TSH_TARGET) $(TFFS_TARGET)

test:$(TEST_TARGET)

$(LIB_TARGET): $(LIBOBJ)
	$(AR) rcs $@ $^

$(TEST_TARGET): $(TESTOBJ) $(LIB_TARGET)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TSH_TARGET): $(TSHOBJ) $(LIB_TARGET)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TFFS_TARGET): $(TFFSOBJ) $(LIB_TARGET)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	-$(RM) src/*.o tshell/*.o $(TEST_TARGET) $(LIB_TARGET) $(TSH_TARGET) \
		$(TFFS_TARGET)
