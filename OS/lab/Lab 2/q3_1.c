#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    printf("Executing cat command:\n");

    execlp("cat", "cat", argv[1], (char *)NULL);

    // If exec fails, this line will execute
    perror("exec failed");

    return 1;
}
