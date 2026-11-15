#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

void show(const char *name) {
    struct stat st;
    if (lstat(name, &st) < 0) {                  /* lstat: do not follow soft links */
        printf("  %-10s : (does not exist)\n", name);
        return;
    }
    printf("  %-10s : inode %-8lu links %lu  size %3ld  %s", name,
           (unsigned long)st.st_ino, (unsigned long)st.st_nlink, (long)st.st_size,
           S_ISLNK(st.st_mode) ? "soft link" : "regular file");
    if (S_ISLNK(st.st_mode)) {
        char target[256];
        ssize_t n = readlink(name, target, sizeof target - 1);
        target[n] = '\0';
        printf(" -> %s", target);
    }
    printf("\n");
}

void try_read(const char *name) {
    char buf[64];
    int fd = open(name, O_RDONLY);               /* open follows soft links */
    if (fd < 0) { printf("  read %-10s : %s\n", name, strerror(errno)); return; }
    ssize_t n = read(fd, buf, sizeof buf - 1);
    buf[n > 0 ? n - 1 : 0] = '\0';               /* drop trailing newline */
    printf("  read %-10s : \"%s\"\n", name, buf);
    close(fd);
}

int main(void) {
    unlink("a.txt"); unlink("ahard.txt"); unlink("asoft.txt");   /* clean start */

    int fd = open("a.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    write(fd, "hello links\n", 12);
    close(fd);

    link("a.txt", "ahard.txt");                  /* $ ln a.txt ahard.txt    */
    symlink("a.txt", "asoft.txt");               /* $ ln -s a.txt asoft.txt */

    printf("1. After creating both links:\n");
    show("a.txt"); show("ahard.txt"); show("asoft.txt");

    unlink("a.txt");                             /* $ rm a.txt */
    printf("\n2. After removing a.txt:\n");
    show("a.txt"); show("ahard.txt"); show("asoft.txt");

    printf("\n3. Reading through each name:\n");
    try_read("ahard.txt");
    try_read("asoft.txt");
    return 0;
}
