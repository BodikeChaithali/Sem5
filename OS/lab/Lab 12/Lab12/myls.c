#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <sys/stat.h>

/* mode_to_string() copied from Tutorial 1 (mystat.c) */
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

int main(int argc, char *argv[]) {
    const char *dir = (argc > 1) ? argv[1] : ".";
    DIR *dp = opendir(dir);
    if (dp == NULL) { perror(dir); return 1; }
    struct dirent *e;
    int count = 0; long total = 0;

    while ((e = readdir(dp)) != NULL) {
        if (e->d_name[0] == '.') continue;            /* skip hidden, . and .. */
        char path[1024];
        snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
        struct stat st;
        if (lstat(path, &st) < 0) { perror(path); continue; }

        char perms[11];
        mode_to_string(st.st_mode, perms);
        struct passwd *pw = getpwuid(st.st_uid);
        struct group  *gr = getgrgid(st.st_gid);

        printf("%8lu %s %2lu %-8s %-8s %8ld %s",
               (unsigned long)st.st_ino, perms, (unsigned long)st.st_nlink,
               pw ? pw->pw_name : "?", gr ? gr->gr_name : "?",
               (long)st.st_size, e->d_name);

        if (S_ISLNK(st.st_mode)) {
            char target[1024];
            ssize_t n = readlink(path, target, sizeof target - 1);
            if (n >= 0) { target[n] = '\0'; printf(" -> %s", target); }
        }
        printf("\n");
        count++;
        total += st.st_size;
    }
    closedir(dp);
    printf("%d entries, %ld bytes in total\n", count, total);
    return 0;
}
