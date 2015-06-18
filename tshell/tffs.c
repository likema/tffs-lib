/*!
 * \file ttfs.c
 * \brief
 * \author Like Ma <likemartinma@gmail.com>
 */

#include "tffs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum {
    MAX_ARG_NUM    = 3,
    BUF_SIZE = 4096
};

typedef int (*do_cmd_func)(tffs_handle_t, int, char* []);

typedef struct _cmd_handler{
    const char * cmd;
    do_cmd_func handler;
} cmd_handler_t;

int do_ls(tffs_handle_t htff, int argc, char* argv[]);
int do_mkdir(tffs_handle_t htff, int argc, char* argv[]);
int do_rm(tffs_handle_t htff, int argc, char* argv[]);
int do_cat(tffs_handle_t htff, int argc, char* argv[]);
int do_write(tffs_handle_t htff, int argc, char* argv[]);

const cmd_handler_t chs[] = {
    { "ls", do_ls },
    { "mkdir", do_mkdir },
    { "rm", do_rm },
    { "cat", do_cat },
    { "write", do_write },
};

enum {
    CHS_NUM = sizeof(chs) /sizeof (chs[0])
};

void print_error(char* tffs_func, int32 rc)
{
    static const char* const errmsg[] = {
        "Invalid parameters.",            //ERR_TFFS_INVALID_PARAM
        "Device access failed.",          //ERR_TFFS_DEVICE_FAIL
        "Bad boot sector.",               //ERR_TFFS_BAD_BOOTSECTOR
        "Bad fat table.",                 //ERR_TFFS_BAD_FAT
        "Invalid path.",                  //ERR_TFFS_INVALID_PATH
        "Got the last directory entry.",  //ERR_TFFS_LAST_DIRENTRY
        "Invalid open mode.",             //ERR_TFFS_INVALID_OPENMODE
        "File is not exist.",             //ERR_TFFS_FILE_NOT_EXIST
        "Open file failed.",              //ERR_TFFS_FILE_OPEN_FAIL
        "No free disk space.",            //ERR_TFFS_NO_FREE_SPACE
        "Try to write a readonly file.",  //ERR_TFFS_READONLY
        "Reached the end of the file.",   //ERR_TFFS_FILE_EOF
        "Access fat table error.",        //ERR_TFFS_FAT
        "Directory is already exist.",    //ERR_TFFS_DIR_ALREADY_EXIST
        "Initialize directory failed.",   //ERR_TFFS_INITIAL_DIR_FAIL
        "No such a file or directory.",   //ERR_TFFS_NO_SUCH_FILE
        "This is not a file.",            //ERR_TFFS_IS_NOT_A_FILE
        "Remove file failed.",            //ERR_TFFS_REMOVE_FILE_FAIL
        "This is not a directory.",       //ERR_TFFS_IS_NOT_A_DIRECTORY
        "This is not a empty directory.", //ERR_TFFS_NOT_EMPTY_DIR
        "Remove directory failed."        //ERR_TFFS_REMOVE_DIR_FAIL
    };

    fprintf(stderr, "TFFS: <%s> failed for error message [%s]\n",
            tffs_func, errmsg[-(rc + 1)]);
}

int main(int argc, char* argv[])
{
    int res = EXIT_SUCCESS;
    tffs_handle_t htffs;
    int32 rc;
    int i;

    if (argc < 3) {
        fprintf(stderr, "Usage: ttfs <image name> <cmd>\n");
        return EXIT_FAILURE;
    }

    if ((rc = TFFS_mount(argv[1], &htffs)) != TFFS_OK) {
        print_error("TFFS_mount", rc);
        return EXIT_FAILURE;
    }

    for (i = 0; i < CHS_NUM && strcmp(chs[i].cmd, argv[2]); ++i)
        ;

    if (i == CHS_NUM) {
        fprintf(stderr, "Invalid command.\n");
        res = EXIT_FAILURE;
    } else {
        res = chs[i].handler(htffs, argc - 3, argv + 3) < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    if ((rc = TFFS_umount(htffs)) != TFFS_OK) {
        print_error("TFFS_umount", rc);
        return EXIT_FAILURE;
    }

    return res;
}

void _show_dirent(dirent_t * dir)
{
    printf("%8d byte\t%2d/%02d/%02d - %02d:%02d  ", dir->dir_file_size,
           dir->crttime.year, dir->crttime.month, dir->crttime.day,
           dir->crttime.hour, dir->crttime.min);

    if (dir->dir_attr & DIR_ATTR_DIRECTORY) {
        printf("\033[32m%s\033[0m", dir->d_name);
    } else {
        printf("%s", dir->d_name);
    }

    printf("\n");
}

int do_ls(tffs_handle_t htffs, int argc, char* argv[])
{
    int res = 0;
    int32 rc;
    tdir_handle_t hdir;
    char* dir;

    if (argc < 0) {
        fprintf(stderr, "Usage: ls <directory>\n");
        return -1;
    }

    dir = argc == 1 ? *argv : "/";
    if ((rc = TFFS_opendir(htffs, dir, &hdir)) != TFFS_OK) {
        print_error("TFFS_opendir", rc);
        return -1;
    }

    while (1) {
        dirent_t dirent;
        if ((rc = TFFS_readdir(hdir, &dirent)) == TFFS_OK) {
            _show_dirent(&dirent);
        } else if (rc == ERR_TFFS_LAST_DIRENTRY) {
            break;
        } else {
            print_error("TFFS_readdir", rc);
            res = -1;
        }
    }

    if ((rc = TFFS_closedir(hdir)) != TFFS_OK) {
        print_error("TFFS_closedir", rc);
        return -1;
    }

    return res;
}

int do_mkdir(tffs_handle_t htffs, int argc, char* argv[])
{
    int32 rc;

    if (argc != 1) {
        fprintf(stderr, "Usage: mkdir <directory>\n");
        return -1;
    }

    if ((rc = TFFS_mkdir(htffs, *argv)) != TFFS_OK) {
        print_error("TFFS_mkdir", rc);
        return -1;
    }

    return 0;
}

int do_rm(tffs_handle_t htffs, int argc, char* argv[])
{
    int32 rc;
    if (argc == 2 && !strcmp(*argv, "-r")) {
        if ((rc = TFFS_rmdir(htffs, argv[1])) != TFFS_OK) {
            print_error("TFFS_rmdir", rc);
            return -1;
        }
    } else if (argc == 1){
        if ((rc = TFFS_rmfile(htffs, *argv)) != TFFS_OK) {
            print_error("TFFS_rmfile", rc);
            return -1;
        }
    } else {
        fprintf(stderr, "Usage: rm [-r] <directory/file name>\n");
        return -1;
    }

    return 0;
}

int do_cat(tffs_handle_t htffs, int argc, char* argv[])
{
    int res = 0;
    int32 rc;
    tfile_handle_t hfile;
    char buf[BUF_SIZE];

    if (argc != 1) {
        printf("Usage: cat <file>, %d\n", argc);
        return -1;
    }

    if ((rc = TFFS_fopen(htffs, *argv, "r", &hfile)) != TFFS_OK) {
        print_error("TFFS_fopen", rc);
        return -1;
    }

    while ((rc = TFFS_fread(hfile, sizeof(buf), (unsigned char*) buf)) >= 0) {
        printf("%s", buf);
    }

    if (rc != ERR_TFFS_FILE_EOF) {
        print_error("TFFS_fread", rc);
        res = -1;
    }

    if ((rc = TFFS_fclose(hfile)) != TFFS_OK) {
        print_error("TFFS_fclose", rc);
        return -1;
    }

    return res;
}


int do_write(tffs_handle_t htffs, int argc, char* argv[])
{
    int res = 0;
    int32 rc;
    size_t n;
    const char* dir;
    tfile_handle_t hfile;
    char mode[] = "w";
    char buf[BUF_SIZE];

    if (argc == 2 && !strcmp(*argv, "-a")) {
        *mode = 'a';
        dir = argv[1];
    } else if (argc == 1) {
        *mode = 'w';
        dir = *argv;
    } else {
        fprintf(stderr, "Usage: write [-a] <file name>\n");
        return -1;
    }

    if ((rc = TFFS_fopen(htffs, (char*) dir,
                         (char*) mode, &hfile)) != TFFS_OK) {
        print_error("TFFS_fopen", rc);
        return -1;
    }

    while ((n = fread(buf, 1, sizeof(buf), stdin)) > 0) {
        rc = TFFS_fwrite(hfile, n, (unsigned char*) buf);
        if (rc < 0 || rc != n) {
            print_error("TFFS_fwrite", rc);
            res = -1;
            break;
        }
    }

    if ((rc = TFFS_fclose(hfile)) != TFFS_OK) {
        print_error("TFFS_fclose", rc);
        return -1;
    }

    return 0;
}
/* vim: set ts=4 sw=4 sts=4 et: */
