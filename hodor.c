#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/fs.h>

#define MAX_BYTES       262144

struct hfile {
        char    *filename;
        char    contents[MAX_BYTES];
        size_t  read_size;
        off_t   st_size;
        uid_t   st_uid;
        gid_t   st_gid;
        mode_t  st_mode;
};

static int
im(size_t fd, int action)
{
        int current_attrs, new_attrs;

        /* immutable */
        int mask = 0x00000010;

        /* Get the current flags */
        if (ioctl(fd, FS_IOC_GETFLAGS, &current_attrs) == -1) {
                return 0;
        }

        if (action) {
                new_attrs = current_attrs | mask;
        } else {
                new_attrs = current_attrs & ~mask;
        }

        ioctl(fd, FS_IOC_SETFLAGS, &new_attrs);

        return 0;
}

static int
rd(struct hfile *file)
{
        size_t fd;
        struct stat statbuf;
        ssize_t size_r;

        fd = open(file->filename, O_RDONLY | __O_NOATIME);
        if (fd < 0) {
                return 1;
        }

        if (fstat(fd, &statbuf) == -1) {
                close(fd);
                return 1;
        }

        file->st_uid = statbuf.st_uid;
        file->st_gid = statbuf.st_gid;
        file->st_size = statbuf.st_size;
        file->st_mode = statbuf.st_mode;

        file->read_size = read(fd, file->contents, sizeof file->contents);

        close(fd);

        return 0;
}

static int
wr(struct hfile *file)
{
        size_t fd;
        struct stat statbuf;
        ssize_t size_r;


        fd = open(file->filename, O_WRONLY | O_CREAT);
        if (fd < 0) {
                return 1;
        }

        im(fd, 0);

        fchown(fd, file->st_uid, file->st_gid);
        fchmod(fd, file->st_mode);

        write(fd, file->contents, file->read_size);

        im(fd, 1);
        close(fd);

        return 0;
}

static int
h(struct hfile *file)
{
        struct hfile file_tmp;
        file_tmp.filename = file->filename;
        rd(&file_tmp);

        if (
                file->st_mode == file_tmp.st_mode &&
                file->st_uid == file_tmp.st_uid &&
                file->st_gid == file_tmp.st_gid &&
                file->st_size == file_tmp.st_size &&
                strncmp(file->contents, file_tmp.contents, file->read_size) == 0
                )
        return 0;

        wr(file);
        return 0;
}

int main(int argc, char **argv)
{
        struct hfile file[MAX_FILES];
        size_t len = 0;
        int i, lines;

        FILE *fp;
        fp = fopen("config", "r");
        if (fp == NULL)
            exit(EXIT_FAILURE);
        
        for (lines=0; lines<MAX_FILES; lines++) {
            file[lines].filename = NULL;
            if (getline(&file[lines].filename, &len, fp) == -1)
                break;
            file[lines].filename[strcspn(file[lines].filename, "\n")] = 0;
            printf("%s", file[lines].filename);
        }
        fclose(fp);

        for (i=0; i<lines; i++) {
            rd(&file[i]);
        }

        while (1) {
            for (i=0; i<lines; i++) {
                h(&file[i]);
            }
            usleep(50000);
        }
}
