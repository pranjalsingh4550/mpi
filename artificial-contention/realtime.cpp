#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>
#include <string.h>
#include <vector>

int rfib (int n) {
	if (n == 0) return 0;
	if (n == 1) return 1;
	return (rfib(n-1) + rfib(n-2))%10000000;
}

void measure (std::vector<std::vector<double>> &messages, std::vector<std::vector<int>> &sizes, int msg, int rank, int mask, void *buf, int tag, FILE *wr) {
	MPI_Barrier (MPI_COMM_WORLD);
	int size;
	MPI_Comm_size (MPI_COMM_WORLD, &size);
	if (size <= (rank ^ mask)) {printf ("rank %d mask %d - error\n", rank, mask); return ;}

	int lower, partner;
	partner = mask ^ rank;
	if ((rank^mask) < rank) lower = 1; else lower = 0;
	if (messages[mask].size() % 2 == 1) lower = lower ^ 1;
	double start, end, other;
	MPI_Status sts;

	start = MPI_Wtime ();
	if (lower) MPI_Send (buf, msg, MPI_BYTE, (rank^mask), tag, MPI_COMM_WORLD);
	else MPI_Recv (buf, msg, MPI_BYTE, (rank^mask), tag, MPI_COMM_WORLD, &sts);
	end = MPI_Wtime();
	end = end - start;
	
	(messages[mask]).push_back (end);
	(sizes[mask]).push_back(msg);
	// printf ("rank %d msg[%d][%d] is %d %lf\n", rank, mask, messages[mask].size() - 1, msg, messages[mask][messages[mask].size() - 1] );
	
	// rfib (30);
	return ;
}

int main (int argc, char ** argv) {
	int rank, size, j, k, tag = 0;
	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	
	char name[256];
	MPI_Get_processor_name (name, &k);
	char file_name[256];
	sprintf (file_name, "time_from_%s", name);
	FILE *output = fopen (file_name, "w");
	
	std::vector <std::vector<double>> messages_to_mask ;
	std::vector <std::vector<int>> size_list ;
	std::vector <double> empty;
	std::vector <int> empty_int;
	for (j = 0; j < 8; j++) {
		messages_to_mask.push_back (empty);
		size_list.push_back (empty_int);
	}

	// call to measure
	void *buf = malloc (1048576);
	memset (buf, 255, 1048576);

	for (k = 0; k < 2; k++); {
		measure (messages_to_mask, size_list, 16384, rank, 2, buf, tag++, output);
		measure (messages_to_mask, size_list, 4096, rank, 2, buf, tag++, output);
		measure (messages_to_mask, size_list, 4096, rank, 5, buf, tag++, output);
		measure (messages_to_mask, size_list, 4096, rank, 6, buf, tag++, output);
		measure (messages_to_mask, size_list, 131072, rank, 6, buf, tag++, output);
		measure (messages_to_mask, size_list, 16384, rank, 5, buf, tag++, output);
		measure (messages_to_mask, size_list, 1048576, rank, 3, buf, tag++, output);
		measure (messages_to_mask, size_list, 131072, rank, 1, buf, tag++, output);
		measure (messages_to_mask, size_list, 16384, rank, 5, buf, tag++, output);
		measure (messages_to_mask, size_list, 4096, rank, 5, buf, tag++, output);
		measure (messages_to_mask, size_list, 16384, rank, 3, buf, tag++, output);
		measure (messages_to_mask, size_list, 4096, rank, 1, buf, tag++, output);
		measure (messages_to_mask, size_list, 131072, rank, 5, buf, tag++, output);
		measure (messages_to_mask, size_list, 1048576, rank, 4, buf, tag++, output);
		measure (messages_to_mask, size_list, 65536, rank, 2, buf, tag++, output);
		measure (messages_to_mask, size_list, 131072, rank, 7, buf, tag++, output);
		measure (messages_to_mask, size_list, 1048576, rank, 2, buf, tag++, output);
		measure (messages_to_mask, size_list, 4096, rank, 7, buf, tag++, output);
		measure (messages_to_mask, size_list, 65536, rank, 7, buf, tag++, output);
		measure (messages_to_mask, size_list, 16384, rank, 1, buf, tag++, output);
		measure (messages_to_mask, size_list, 65536, rank, 5, buf, tag++, output);
		measure (messages_to_mask, size_list, 1048576, rank, 2, buf, tag++, output);
		measure (messages_to_mask, size_list, 131072, rank, 2, buf, tag++, output);
		measure (messages_to_mask, size_list, 16384, rank, 1, buf, tag++, output);
		measure (messages_to_mask, size_list, 65536, rank, 1, buf, tag++, output);
		measure (messages_to_mask, size_list, 1048576, rank, 7, buf, tag++, output);
		measure (messages_to_mask, size_list, 65536, rank, 4, buf, tag++, output);
		measure (messages_to_mask, size_list, 16384, rank, 7, buf, tag++, output);
		measure (messages_to_mask, size_list, 131072, rank, 5, buf, tag++, output);
		measure (messages_to_mask, size_list, 65536, rank, 6, buf, tag++, output);
		measure (messages_to_mask, size_list, 1048576, rank, 1, buf, tag++, output);
		measure (messages_to_mask, size_list, 4096, rank, 4, buf, tag++, output);
		measure (messages_to_mask, size_list, 1048576, rank, 3, buf, tag++, output);
		measure (messages_to_mask, size_list, 131072, rank, 3, buf, tag++, output);
		measure (messages_to_mask, size_list, 65536, rank, 7, buf, tag++, output);
		measure (messages_to_mask, size_list, 1048576, rank, 5, buf, tag++, output);
		measure (messages_to_mask, size_list, 4096, rank, 6, buf, tag++, output);
		measure (messages_to_mask, size_list, 4096, rank, 2, buf, tag++, output);
		measure (messages_to_mask, size_list, 131072, rank, 3, buf, tag++, output);
		measure (messages_to_mask, size_list, 131072, rank, 6, buf, tag++, output);
		measure (messages_to_mask, size_list, 16384, rank, 4, buf, tag++, output);
		measure (messages_to_mask, size_list, 1048576, rank, 4, buf, tag++, output);
		measure (messages_to_mask, size_list, 65536, rank, 3, buf, tag++, output);
		measure (messages_to_mask, size_list, 65536, rank, 2, buf, tag++, output);
		measure (messages_to_mask, size_list, 1048576, rank, 7, buf, tag++, output);
		measure (messages_to_mask, size_list, 65536, rank, 4, buf, tag++, output);
		measure (messages_to_mask, size_list, 65536, rank, 6, buf, tag++, output);
		measure (messages_to_mask, size_list, 1048576, rank, 5, buf, tag++, output);
		measure (messages_to_mask, size_list, 4096, rank, 3, buf, tag++, output);
		measure (messages_to_mask, size_list, 16384, rank, 3, buf, tag++, output);
		measure (messages_to_mask, size_list, 16384, rank, 4, buf, tag++, output);
		measure (messages_to_mask, size_list, 65536, rank, 3, buf, tag++, output);
		measure (messages_to_mask, size_list, 131072, rank, 7, buf, tag++, output);
		measure (messages_to_mask, size_list, 16384, rank, 2, buf, tag++, output);
		measure (messages_to_mask, size_list, 4096, rank, 1, buf, tag++, output);
		measure (messages_to_mask, size_list, 1048576, rank, 1, buf, tag++, output);
		measure (messages_to_mask, size_list, 16384, rank, 6, buf, tag++, output);
		measure (messages_to_mask, size_list, 16384, rank, 7, buf, tag++, output);
		measure (messages_to_mask, size_list, 65536, rank, 1, buf, tag++, output);
		measure (messages_to_mask, size_list, 4096, rank, 3, buf, tag++, output);
		measure (messages_to_mask, size_list, 4096, rank, 4, buf, tag++, output);
		measure (messages_to_mask, size_list, 131072, rank, 4, buf, tag++, output);
		measure (messages_to_mask, size_list, 65536, rank, 5, buf, tag++, output);
		measure (messages_to_mask, size_list, 131072, rank, 4, buf, tag++, output);
		measure (messages_to_mask, size_list, 1048576, rank, 6, buf, tag++, output);
		measure (messages_to_mask, size_list, 131072, rank, 2, buf, tag++, output);
		measure (messages_to_mask, size_list, 16384, rank, 6, buf, tag++, output);
		measure (messages_to_mask, size_list, 131072, rank, 1, buf, tag++, output);
		measure (messages_to_mask, size_list, 1048576, rank, 6, buf, tag++, output);
		measure (messages_to_mask, size_list, 4096, rank, 7, buf, tag++, output);
	}
	// unpack and reduce
	MPI_Status sts;
	int len = messages_to_mask.size();
	// std::vector<std::vector<double>>::iterator it;
	// for (it = messages_to_mask.begin(); it != messages_to_mask.end(); it ++) {
	for (int mask = 1; mask < 8; mask++) {
		// printf ("%d len is %d ", mask, messages_to_mask.size());
		empty.clear();
		// printf ("%d ", empty.size());
		empty = messages_to_mask[mask];
		len = empty.size();
		if (len == 0) continue;
		// printf ("rank %d mask %d len %d\n", rank, mask, len);
		double ar[len], other[len];
		int y;
		for (y = 0; y < len; y++) ar[y] = empty[y];
		int lower = ((rank^mask) > rank ? 1 : 0 ), partner = (rank ^ mask);
		if (lower) MPI_Send ( &ar, len, MPI_DOUBLE, partner, tag, MPI_COMM_WORLD);
		else MPI_Recv (&other, len, MPI_DOUBLE, partner,tag, MPI_COMM_WORLD, &sts);
		if (!lower) MPI_Send ( &ar, 1, MPI_DOUBLE, partner, tag, MPI_COMM_WORLD);
		else MPI_Recv (&other, len, MPI_DOUBLE, partner,tag, MPI_COMM_WORLD, &sts);
		for (y = 0; y < len; y++) {
			ar[y] = (ar[y] > other[y] ? ar[y] : other[y] ) ;
			size = size_list[mask][y];
			fprintf (output, "%d,%lf\n", size, ar[y]);
		}
	}

	fclose (output);
	MPI_Finalize();
	return 0;
}
