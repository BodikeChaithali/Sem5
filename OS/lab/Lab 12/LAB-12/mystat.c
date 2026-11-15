#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>

/* Turn st_mode into a string like "-rwxr-x---" */
void mode_to_string(mode_t m, char s[11]) {
    s[0] = S_ISDIR(m) ? 'd' : S_ISLNK(m) ? 'l' : S_ISFIFO(m) ? 'p' : '-';
    s[1] = (m & S_IRUSR) ? 'r' : '-';
    s[2] = (m & S_IWUSR) ? 'w' : '-';
    s[3] = (m & S_IXUSR) ? 'x' : '-';
    s[4] = (m & S_IRGRP) ? 'r' : '-';
    s[5] = (m & S_IWGRP) ? 'w' : '-';
    s[6] = (m & S_IXGRP) ? 'x' : '-';
    s[7] = (m & S_IROTH) ? 'r' : '-';
    s[8] = (m & S_IWOTH) ? 'w' : '-';
    s[9] = (m & S_IXOTH) ? 'x' : '-';
    s[10] = '\0';
}

void show(const char *path) {
    struct stat st;
    if (stat(path, &st) < 0) { perror(path); exit(1); }

    char perms[11];
    mode_to_string(st.st_mode, perms);
    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);

    printf("File        : %s\n", path);
    printf("Inode       : %lu\n", (unsigned long)st.st_ino);
    printf("Type        : %s\n", S_ISREG(st.st_mode) ? "regular file" :
                                 S_ISDIR(st.st_mode) ? "directory" :
                                 S_ISFIFO(st.st_mode) ? "pipe (FIFO)" : "other");
    printf("Size        : %ld bytes (%ld blocks)\n", (long)st.st_size, (long)st.st_blocks);
    printf("Hard links  : %lu\n", (unsigned long)st.st_nlink);
    printf("Owner/Group : %s / %s\n", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");
    printf("Permissions : %s (octal %03o)\n", perms, st.st_mode & 0777);
    printf("Last access : %s", ctime(&st.st_atime));
    printf("Last modify : %s", ctime(&st.st_mtime));
    printf("Last change : %s", ctime(&st.st_ctime));   /* metadata change */
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file> [new-octal-mode]\n", argv[0]);
        return 1;
    }
    show(argv[1]);

    if (argc == 3) {                               /* optional: change permissions */
        mode_t mode = (mode_t)strtol(argv[2], NULL, 8);
        if (chmod(argv[1], mode) < 0) { perror("chmod"); return 1; }
        printf("\nAfter chmod %s:\n", argv[2]);
        show(argv[1]);
    }
    return 0;
}
