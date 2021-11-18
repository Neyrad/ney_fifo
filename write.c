#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

const char* COMM_FIFO_PATH = "fifo_common";

#define BUF_SIZE  4096
#define MAX_NAME  32 //characters

#define ERROR(X) {perror("ERROR! "#X"()\n"); exit(EXIT_FAILURE);}

int main(int argc, char* argv[])
{
	if ( argc != 2 )											   ERROR(usage: ./write <file_name>)

	int comm_fd = 0;
	if ( (mkfifo(COMM_FIFO_PATH, 0666) < 0) && (errno != EEXIST) ) ERROR(mkfifo)
	if ( (comm_fd = open(COMM_FIFO_PATH, O_RDONLY)) < 0 )		   ERROR(common open)

	pid_t pid = 0;
	if ( read(comm_fd, &pid, sizeof(pid_t)) < 0 )				   ERROR(read)

	char* uniq_name = calloc(MAX_NAME, sizeof(char));
	if ( uniq_name == NULL )									   ERROR(calloc)
	sprintf(uniq_name, "fifo_%d", pid);

	int uniq_fd = 0;
	if ( (uniq_fd = open(uniq_name, O_WRONLY | O_NONBLOCK)) < 0 )  ERROR(unique open)
	if ( fcntl(uniq_fd, F_SETFL, O_WRONLY) < 0 )				   ERROR(fcntl)

	int file_fd = 0;
	if ( (file_fd = open(argv[1], O_RDONLY)) < 0 )				   ERROR(file open)

	int n_chars = 0; char buf[BUF_SIZE] = {};	
	while( (n_chars = read(file_fd, buf, BUF_SIZE)) > 0 )
		if ( write(uniq_fd, buf, n_chars) != n_chars)			   ERROR(write)
	if ( n_chars < 0 )											   ERROR(read)

	close(comm_fd); close(file_fd); close(uniq_fd);
	free(uniq_name);
}
