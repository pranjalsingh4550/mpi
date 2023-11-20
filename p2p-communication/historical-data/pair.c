#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "mpi.h"
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
	memset (buf, 255, msg_size);
	buf = malloc (msg_size);
	// printf ("buf is %p, size is %d %s\n", buf, msg_size, argv[1]);
	MPI_Init (&argc, &argv);
	MPI_Comm_size (MPI_COMM_WORLD, &size); // size = 2 for us
	if (size != nproc_req){
		printf ("Number of processes is %d, should be %d\n", size, nproc_req);
		exit (-1);
	}
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	char name[40]; 
	int len;
	MPI_Get_processor_name (buf, &len);
	MPI_Get_processor_name (name, &len);
	/* write the processor number to buffer
	if (name[4] != '1'){
		buf[0] = name[5] ;
		buf[1] = name[6] ;
		buf[2] = name[7] ;
		buf[3] = name[8] ;
	}
	else {
		buf[0] = name[4];
		buf[1] = name[5];
		buf[2] = name[6];
		buf[3] = name[7];
	}*/
	// printf ("rank %d name %s buf %s\n", rank, name, buf);
	MPI_Barrier (MPI_COMM_WORLD);
	start = MPI_Wtime ();
	if (rank == 0){
		MPI_Send (buf, msg_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
	}
	else
		MPI_Recv (buf, msg_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);
	end = MPI_Wtime ();
	double tmax, tm = end - start;
	// printf ("processor %s rank %d start %lf end %lf time taken %lf\n", name,rank, start, end, tm);

	MPI_Reduce (&tm, &tmax, 1, MPI_DOUBLE, MPI_MAX, 1, MPI_COMM_WORLD);
	int flag = 0;
	if (rank == 1) printf ("%s,%s,%d,%lf\n", buf, name, msg_size, tmax); // from, to
	if (MPI_WTIME_IS_GLOBAL == 0) printf ("not global!");
	MPI_Finalize ();
	exit (0);
}

