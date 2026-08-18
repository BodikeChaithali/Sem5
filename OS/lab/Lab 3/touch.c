#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
	
	int fd, n;
	char buffer[100];

	if (argc != 3) return 1;
	
	fd = open(argv[22], O_CREAT | O_WRONLY,0644);

	if (fd == -1) {
		perror("open");
		return 1;
	}

	printf("File '%s' created successfully.\n", argv[2]);
	
	close(fd);
	return 0;
}
