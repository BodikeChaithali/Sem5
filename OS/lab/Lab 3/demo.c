#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *filename = "demo.txt";
    const char *message  = "Hello, file handling!\n";
    char buffer[100];
    int fd, n;

    /* ---------- STEP 1: OPEN a file for writing ---------- */
    printf("Step 1: Opening '%s' for writing...\n", filename);
    fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open (write)");
        return 1;
    }
    printf("   -> Success. File descriptor = %d\n\n", fd);

    /* ---------- STEP 2: WRITE to the file ---------- */
    printf("Step 2: Writing message to the file...\n");
    n = write(fd, message, strlen(message));
    if (n == -1) {
        perror("write");
        close(fd);
        return 1;
    }
    printf("   -> Wrote %d bytes.\n\n", n);

    /* ---------- STEP 3: CLOSE the file ---------- */
    printf("Step 3: Closing the file descriptor %d...\n", fd);
    close(fd);
    printf("   -> Closed.\n\n");

    /* ---------- STEP 4: OPEN the same file for reading ---------- */
    printf("Step 4: Reopening '%s' for reading...\n", filename);
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open (read)");
        return 1;
    }
    printf("   -> Success. File descriptor = %d\n\n", fd);

    /* ---------- STEP 5: READ from the file ---------- */
    printf("Step 5: Reading contents back into a buffer...\n");
    n = read(fd, buffer, sizeof(buffer) - 1); /* leave room for '\0' */
    if (n == -1) {
        perror("read");
        close(fd);
        return 1;
    }
    buffer[n] = '\0'; /* manually null-terminate since read() doesn't */
    printf("   -> Read %d bytes: \"%s\"\n\n", n, buffer);

    /* ---------- STEP 6: CLOSE the file again ---------- */
    printf("Step 6: Closing the file descriptor %d...\n", fd);
    close(fd);
    printf("   -> Done!\n");

    return 0;
}
