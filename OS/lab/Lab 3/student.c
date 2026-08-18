#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *filename = "student.txt";

    const char *student_info =
        "Name: Chaithali\n"
        "Roll No: 101\n"
        "Course: B.Tech.\n"
        "Subject: Operating System\n";

    char buffer[200];
    int fd, n;

    printf("Step 1: Opening '%s' for writing...\n", filename);

    fd = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0644);

    if (fd == -1) {
        perror("open (write)");
        return 1;
    }

    printf("   -> Success. File descriptor = %d\n\n", fd);

    printf("Step 2: Writing student information to the file...\n");

    n = write(fd, student_info, strlen(student_info));

    if (n == -1) {
        perror("write");
        close(fd);
        return 1;
    }

    printf("   -> Wrote %d bytes.\n\n", n);

    printf("Step 3: Closing the file descriptor %d...\n", fd);

    close(fd);

    printf("   -> Closed.\n\n");

    printf("Step 4: Reopening '%s' for reading...\n", filename);

    fd = open(filename, O_RDONLY);

    if (fd == -1) {
        perror("open (read)");
        return 1;
    }

    printf("   -> Success. File descriptor = %d\n\n", fd);


    printf("Step 5: Reading contents from the file...\n");

    n = read(fd, buffer, sizeof(buffer) - 1);

    if (n == -1) {
        perror("read");
        close(fd);
        return 1;
    }

    buffer[n] = '\0';

    printf("   -> Read %d bytes.\n\n", n);


    printf("Step 6: Student information:\n");
    printf("--------------------------------\n");
    printf("%s", buffer);
    printf("--------------------------------\n\n");


    printf("Step 7: Closing the file descriptor %d...\n", fd);

    close(fd);

    printf("   -> Done!\n");

    return 0;
}
