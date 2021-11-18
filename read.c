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

#define WAIT_TIME 5  //seconds
#define BUF_SIZE  4096
#define MAX_NAME  32 //characters

#define ERROR(X) {perror("ERROR! "#X"()\n"); exit(EXIT_FAILURE);}

bool can_read_fifo(int fd);

int main()
{
	pid_t pid = getpid();
	char* uniq_name = calloc(MAX_NAME, sizeof(char));
	if ( uniq_name == NULL ) 	  						           ERROR(calloc)
	sprintf(uniq_name, "fifo_%d", pid);

	int uniq_fd = 0;
	if( (mkfifo(uniq_name, 0666) < 0) && (errno != EEXIST) )       ERROR(unique mkfifo)
	if( (uniq_fd = open(uniq_name, O_RDONLY | O_NONBLOCK)) < 0 )   ERROR(unique open)
	if( fcntl(uniq_fd, F_SETFL, O_RDONLY) < 0 ) 			       ERROR(fcntl)

	int comm_fd = 0;
	if ( (mkfifo(COMM_FIFO_PATH, 0666) < 0) && (errno != EEXIST) ) ERROR(common mkfifo)
	if ( (comm_fd = open(COMM_FIFO_PATH, O_WRONLY)) < 0) 		   ERROR(common open)
	
	if ( write(comm_fd, &pid, sizeof(pid_t)) < 0 )	     		   ERROR(write)
	if ( !can_read_fifo(uniq_fd) )
	{
		if ( unlink(uniq_name) < 0 ) 							   ERROR(unlink)
		free(uniq_name);										  
																   ERROR(time is up)
	}

	int n_chars = 0; char buf[BUF_SIZE] = {};
	while( (n_chars = read(uniq_fd, buf, BUF_SIZE)) > 0 )
		if ( write(STDOUT_FILENO, buf, n_chars) != n_chars )       ERROR(write)
	if ( n_chars < 0 )											   ERROR(read)

	close(uniq_fd); close(comm_fd);
	if ( unlink(uniq_name) < 0) 								   ERROR(unlink)
	free(uniq_name);
}

bool can_read_fifo(int fd)
{
	fd_set read_fds = {};
	FD_ZERO(	&read_fds);
	FD_SET (fd, &read_fds);

	struct timeval tv_fifo = {};
	tv_fifo.tv_sec  = WAIT_TIME;
	tv_fifo.tv_usec = 0;

	return select(fd + 1, &read_fds, NULL, NULL, &tv_fifo) > 0 ? true : false;	
}
