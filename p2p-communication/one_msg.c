#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include </users/btech/prsingh/mpich-install/include/mpi.h>
#define nproc_req 2
#define BUF_SIZE 4096

int main (int argc, char ** argv){
	int rank, size;
	if (argc != 2) {
		printf ("Usage: $ ./a.out <#bytes>\n");
		exit (-1);
	}
	int msg_size = atoi (argv[1]) ;
	double start, end;
	MPI_Status status;
	int i = 0;
	char *buf;
	buf = malloc (msg_size);
	// printf ("buf is %p, size is %d %s\n", buf, msg_size, argv[1]);
	MPI_Init (&argc, &argv);
	MPI_Comm_size (MPI_COMM_WORLD, &size); // size = 2 for us
	if (size != nproc_req){
		printf ("Number of processes is %d, should be %d\n", size, nproc_req);
		exit (-1);
	}
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Barrier (MPI_COMM_WORLD);
	start = MPI_Wtime ();
	if (rank == 0){
		MPI_Send (buf, msg_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
	}
	else
		MPI_Recv (buf, msg_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);
	end = MPI_Wtime ();
	double tmax, tm = end - start;
	// char name[20];
	int len;
	// MPI_Get_processor_name (name, &len);
	// printf ("processor %s start %lf end %lf time taken %lf\n", name, start, end, tm);

	MPI_Reduce (&tm, &tmax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	if (rank == 0) printf ("%d,%lf\n", msg_size, tmax);
	if (MPI_WTIME_IS_GLOBAL == 0) printf ("not global!");
	MPI_Finalize ();
	exit (0);
}

