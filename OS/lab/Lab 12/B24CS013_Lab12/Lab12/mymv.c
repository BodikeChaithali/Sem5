#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        return 1;
    }
    const char *src = argv[1], *dst = argv[2];
    struct stat st;
    if (stat(src, &st) < 0) { perror(src); return 1; }
    printf("Before: %s inode %lu links %lu\n", src,
           (unsigned long)st.st_ino, (unsigned long)st.st_nlink);

    /* Step 1: add a second name for the same inode */
    if (link(src, dst) < 0) {
        if (errno == EEXIST)
            fprintf(stderr, "mymv: '%s' already exists\n", dst);
        else if (errno == EXDEV)
            fprintf(stderr, "mymv: '%s' and '%s' are on different file systems; "
                            "a hard link cannot cross them\n", src, dst);
        else if (errno == EPERM && S_ISDIR(st.st_mode))
            fprintf(stderr, "mymv: '%s' is a directory; "
                            "hard links to directories are not allowed\n", src);
        else
            perror("link");
        return 1;
    }

    /* Step 2: remove the old name */
    if (unlink(src) < 0) { perror("unlink"); return 1; }

    if (stat(dst, &st) < 0) { perror(dst); return 1; }
    printf("After : %s inode %lu links %lu\n", dst,
           (unsigned long)st.st_ino, (unsigned long)st.st_nlink);
    printf("Moved '%s' -> '%s' (same inode, no data copied)\n", src, dst);
    return 0;
}
