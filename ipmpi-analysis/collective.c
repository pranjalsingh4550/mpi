#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "/users/btech/prsingh/mpich-install/include/mpi.h"

int main (int argc, char ** argv){
	int rank, size;
	if (argc == 1) {
		printf ("Arg missing!\nUsage: $ ./a.out <message size> <b, g, t - choose collective; default broadcast> <#iterations, default 1>\n");
		exit (-1);
	}
	char collective = 'b';
	int iter = 1;
	if (argv[2]) collective = argv[2][0] ;
	if (argc >= 4) iter = atoi (argv[3]) ;
	int msg_size = atoi (argv[1]);
	MPI_Init (&argc, &argv);
	MPI_Comm_size (MPI_COMM_WORLD, &size);
	void *buf ;
	void *recvbuf ;
	if (collective == 't') { // alltoall
		buf = malloc (size * msg_size);
		memset (buf, 255, size*msg_size);
		recvbuf = malloc (size * msg_size);
		memset (recvbuf, 255, size * msg_size);
	} else if (collective == 'b') { // bcast
		buf = malloc (msg_size);
		memset (buf, 255, msg_size);
		recvbuf = malloc (msg_size);
		memset (recvbuf, 255, msg_size);
	} else if (collective == 'g') { // allgather 
		buf = malloc (msg_size);
		memset (buf, 255, msg_size);
		recvbuf = malloc ( size * msg_size);
		memset (recvbuf, 255, size * msg_size);
	} else {
		printf ("invalid arg %c\n", collective);
		exit (10);
	}

	double st, end;
	MPI_Barrier (MPI_COMM_WORLD);
	st = MPI_Wtime ();
	for (; iter > 0; iter --) {
		if (collective == 'b') MPI_Bcast (buf, msg_size, MPI_BYTE, 0, MPI_COMM_WORLD);
		else if (collective == 't') MPI_Alltoall (buf, msg_size, MPI_BYTE, recvbuf, msg_size, MPI_BYTE, MPI_COMM_WORLD) ;
		else if (collective == 'g') MPI_Allgather (buf, msg_size, MPI_BYTE, recvbuf, msg_size, MPI_BYTE, MPI_COMM_WORLD);
		else ;
	}
	end = MPI_Wtime ();
	if (MPI_WTIME_IS_GLOBAL == 0) printf ("Not global\n");
	end = end - st;
	double max;
	MPI_Reduce (&end, &max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	// printf ("Terminating rank %d\n", rank);
	if (rank == 0) printf ("%d,%lf\n", msg_size, max);
	MPI_Finalize ();
	return 0;
}
