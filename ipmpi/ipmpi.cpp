#include <stdio.h>
#include<execinfo.h>

#include<limits.h>  // INT_MIN and INT_MAX
#include "include/helper.h"

std::unordered_map<long, std::string> fmap;

// Used to log all MPI calls
// #define DEBUG

// #define START_FUNC

// Used to log #MPI calls
#define LOG_MPI_CALLS


int rank = -1;
const char *homedir;
static FILE *logger;
static FILE *info_file;
// static FILE *test_file;
// static FILE *node_file;
enum IpmpiEvent { Error, Compute, Recv, Send, Bcast, Scatter, Gather };
// const char *homedir;

int comm_size = 0;
long BIN_SIZE=102400;
std::vector<std::vector<long>> comm_matrix_sum, comm_matrix_max, comm_matrix_min, p2p_mat;
std::vector<std::vector<std::vector<int>>> comm_matrix_count;

std::unordered_map<std::string, int> log_calls;
std::unordered_map<std::string, int> log_calls_w_cs;
// double median_array[25][25] ; // stores medians for the current node // pranjal
std::unordered_map <int, std::unordered_map <int, double> > median_array ; // median_array[dest][size] = median; // pranjal
// std::unordered_map <int, std::unordered_map <int, double> > dynamic_median_array ; // median_array[dest][size] = median; // pranjal
std::unordered_map <int, double> dynamic_median_array ; // median_array[dest][size] = median; // pranjal
std::unordered_map <std::string , int > node_to_rank_map ; // pranjal
#define NODE_NAME_LENGTH 40 // pranjal
char *all_names ; // pranjl

int init_median_lookup (){
	// initialises node_to_rank_map and median_array
	char pname[MPI_MAX_PROCESSOR_NAME];
	int name_len;
	MPI_Get_processor_name(pname, &name_len);
	char lookup_csv[NODE_NAME_LENGTH + 128] ; // modify
	char lookup_dir[] = "/users/btech/prsingh/mpicode/run_random/node_to_node" ;
	sprintf (lookup_csv, "%s/median_from_%s", lookup_dir, pname); // for portability
	if (pname[NODE_NAME_LENGTH - 1] != '\0') printf ("Node %s - use a longer string! NODE_NAME_LENGTH chars exceeded\n", pname );
	// std::unordered_map <int, std::string> rank_to_name;
	all_names = (char *)malloc (NODE_NAME_LENGTH * comm_size);
	PMPI_Allgather (pname, NODE_NAME_LENGTH, MPI_CHAR, all_names, NODE_NAME_LENGTH, MPI_CHAR, MPI_COMM_WORLD) ;
	FILE *data = fopen (lookup_csv, "r");
	if (data == NULL) {printf ("Error in opening %s\n.", lookup_csv); return 2;}
	int size; char destination[NODE_NAME_LENGTH] ;
	double median_time;
	int j;
	for (j = 0; j < comm_size; j++) {
		// std::cout << "hello j is " << j << std::endl;
		// printf ("At node %s rank %d: found node %s rank %d\n", pname, rank, all_names + j * NODE_NAME_LENGTH, j);
		node_to_rank_map [all_names + j * NODE_NAME_LENGTH ] = j ;
		std::string st = all_names + j * NODE_NAME_LENGTH; 
		// printf ("at rank %d: assigned rank %d to string ", rank, j, node_to_rank_map [st]); std::cout << st << std::endl;
		// initialising median_array;
		std::unordered_map <int, double> dummy = { {0, 0.00}};
		median_array[j] = dummy;
		// std::cout << "hello oo \n" ;
		while (fscanf (data, "%s\t%d\t%lf\n" , destination, &size, &median_time) > 0) {
			if (strcmp (destination, all_names + NODE_NAME_LENGTH * j) !=0 ) continue ;
			// if (rank == 1) printf ("scanning rank %d j %d. read dest %s size %d time %lf\n", rank, j, destination, size, median_time);
			median_array [ node_to_rank_map[destination] ][size] = median_time ;
			// if (rank == 1) printf ("added to map: %s rank %d, size %d,, time %lf\n", destination, node_to_rank_map[destination], size, median_array[node_to_rank_map[destination] ] [size] ) ;
		}
		fseek (data, 0, SEEK_SET);
		// std::cout << "bye j is " << j << "rank is " << rank << std::endl;
	}
	// printf ("added value - rank %d to rank 2, 16384 B: %lf s\n", rank, median_array[2][16384] ) ;
	return 0;
} // pranjal

void init_realtime_median_lookup () { // pranjal
	char dir[300] = "/users/btech/prsingh/mpicode/background-contention/median_from_";
	char name[256];
	int len;
	MPI_Get_processor_name (name, &len);
	strcat (dir, name);
	FILE *med = fopen (dir, "r");
	if (med == NULL) {printf ("error opening %s\n",dir); return ;}
	int size; double time;
	while (fscanf (med, "%d,%lf\n", &size, &time) > 0) {
		dynamic_median_array [size] = time;
		// printf ("Node %s msg size %d time %lf\n", name, size, dynamic_median_array[size]);
	}
	fclose (med);
}

double find_time (int size, int dest) { // pranjal
	// estimates time needed to send message from dynamic_median_array
	// thresholds decided heuristically
	// printf ("Call to find_time rank %d size %d\n", rank, size);
	if (strcmp ( all_names+NODE_NAME_LENGTH*rank, all_names+NODE_NAME_LENGTH*dest) == 0) {
		// printf ("Intranode message %ld to %ld size %d\n", rank, dest, size);
		// Use historical data?
		return median_array [rank][size];
	}
	if (size <= 8192) return dynamic_median_array[4096]; 
	else if (size <= 131072) {
		// a + bn
			double latency = (4*dynamic_median_array[16384] - dynamic_median_array[65536] ) / 3.0 ; // // // HOCKNEY
		double rate = (dynamic_median_array[65536] - dynamic_median_array[16384] )*size ; // // // HOCKNEY
		// printf ("105:: rank %d found latency %lf bandwidth %lf, size %d\n", rank, latency, rate, size);
		rate = rate / ((double)(3 * 16384)) ; // // // HOCKNEY
		// printf ("105:: rank %d found latency %lf + %lf, size %d\n", rank, latency, rate, size);
		return (latency + rate);
	}
	else {
		// a + bn
		double latency = (8*dynamic_median_array[131072] - dynamic_median_array[1048576]) / 7 ; // // // HOCKNEY
		double rate = (dynamic_median_array[1048576] - dynamic_median_array[131072] ) * size;
		rate = rate / (7 * 131072) ; // // // HOCKNEY
		// printf ("105:: rank %d found latency %lf + %lf for size %d\n", rank, latency, rate, size);
		return (latency + rate);
	}
	return -1.0;
}

// For computing #sends per collective
std::unordered_map<std::string, std::unordered_map<int, std::unordered_map<int, int>>> sends;

std::string parent_func="";
// static std::vector<std::vector<vector<long>>> comm_matrix_sum;


// std::vector<int> vals;
std::vector<std::vector<std::vector<long> > > comm_steps;

std::vector< std::vector<long> > sr_calls;

std::vector< std::vector<long> > send_calls; 
// long total_calls=0;
long fid=0;
long send_count=0;
long scall=0;

int time_collective_from_temp_steps (std::vector <std::vector <long>> &temp_steps) { // pranjal
	return 0; /*
	// simple estimate of global #steps
	int nsteps = comm_size*2 + 5 ;
	// double time[comm_size*2 + 5] = {-1};
	double *time = (double *)malloc ( sizeof (double) * (3 * comm_size));
	int i, j, k;
	srandom (205 + rank * rank);
	for (i = 0; i < 2*comm_size + 5; i++) time[i] = 0.0;
	int nsteps_actual = 0;
	char list_steps[1000];
	// sprintf (list_steps, "steps rank %d : ", rank);
	for (auto &e : temp_steps) {
		if (e[1] > nsteps_actual) nsteps_actual = e[1] ;
		time[e[1]] = median_array[e[3]][e[4]] ;
		e.push_back ( (long) (10000000 * time[e[1]]));
		char temp[100];
		// sprintf (temp, "[%ld, %ld, %lx, %lf, %ld] ", e[1], e[3], e[4], time[e[1]] , e[e.size() - 1]);
		// strcat (list_steps, temp);
	} // printf ("+++\n");
	// strcat (list_steps, "+++\n");
	// if (rank == 0) printf ("%s", list_steps);
	PMPI_Allreduce (&nsteps_actual, &nsteps, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
	nsteps ++ ;
	// if (rank == 0) printf ("nsteps is %d\n", nsteps);
	double *minmatrix = (double *) malloc ( sizeof (double) * (3 * comm_size));
	// printf ("rank %d: matrix is ", rank);
	// for (i = 0; i < nsteps; i++) printf ("%.7lf; ", time[i]); printf ("\n");
	PMPI_Allreduce (time, minmatrix, nsteps, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD) ; // can reduce contention at by changing root to total_steps % comm_size 

	double total = 0.0;
	for (i = 0; i < nsteps; i ++) {
		// if (rank == 0) printf ("step %d of collective - time taken %lf\n", i, minmatrix[i] ) ;
		total += minmatrix[i] ;
	} // find a way to deal with the excess GARABGE values
	// printf ("Rank %d - exiting time function\n", rank);
	if (rank == 0) fprintf (stderr, "Historical data: %lf\n", total);
	free (minmatrix); free (time);
	// if (rank == 0) printf ("rank 0 returning\n");
	return 0; */

} // pranjal

int time_collective_from_temp_steps_realtime (std::vector <std::vector <long>> &temp_steps) { // pranjal
	// simple estimate of global #steps
	// commented blocks are for the first approach mentioned under historical data in the report
	int nsteps = comm_size*2 + 5 ;
	double *time = (double *) malloc (3 * sizeof (double) * comm_size);
	int i, j, k;
	srandom (205 + rank * rank);
	for (i = 0; i < 2*comm_size + 5; i++) time[i] = 0.0;
	int nsteps_actual = 0;
	// char list_steps[300];
	// sprintf (list_steps, "steps rank %d : ", rank);
	// if (rank == 0) printf ("HELLO R0 --------------------------------\nsteps here is %d\n", temp_steps.size());
	for (auto &e : temp_steps) {
		if (e[1] > nsteps_actual) nsteps_actual = e[1] ;
		time[e[1]] = find_time (e[4], e[3]);
		e.push_back ( (long) (10000000 * time[e[1]]));
		// char temp[100];
		// sprintf (temp, "[%ld, %ld, %lx, %lf, %ld, %ld] ", e[1], e[3], e[4], time[e[1]] , e[5], e[6]);
		// strcat (list_steps, temp);
	} // printf ("+++\n");
	// if (rank == 0) printf ("HELLO R0 --------------------------------\n");
	// strcat (list_steps, "+++\n");
	// printf ("%s", list_steps);
	// if (temp_steps.size()) printf ("rank %d first step [%ld, %ld, %lx, %ld, %ld]\n", rank, temp_steps[0][1], temp_steps[0][3], temp_steps[0][4], temp_steps[0][5], temp_steps[0][6]);
	// PMPI_Allreduce (&nsteps_actual, &nsteps, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
	// nsteps ++ ;
	// if (rank == 0) printf ("nsteps id %d\n", nsteps);
	// double *minmatrix = (double *) malloc (3 * sizeof (double) * comm_size);
	// printf ("rank %d: matrix is ", rank);
	// for (i = 0; i < nsteps; i++) printf ("%.7lf; ", time[i]); printf ("\n");
	// PMPI_Allreduce (time, minmatrix, nsteps, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD) ; // can reduce contention at by changing root to total_steps % comm_size 

	// double total = 0.0;
	// for (i = 0; i < nsteps; i ++) {
		// if (rank == 0) printf ("step %d of collective - time taken %lf\n", i, minmatrix[i] ) ;
		// total += minmatrix[i] ;
	// } // find a way to deal with the excess GARABGE values
	// printf ("Rank %d - exiting time function\n", rank);
	free (time);
	// if (rank == 0) fprintf (stderr, "%lf\n", total);
	return 0;   

} // pranjal

std::string getFunc(char *temp) {
  std::string s = temp;
  std::string res="";
  int i;
  bool flag = false;
  for(i=0;i<s.size();i++) {
    if(flag and s[i] != '+')
      res += s[i]; 
    if(s[i] == '(')
      flag = true;
    if(s[i] == '+')
      flag = false;
  }
  return res;
}
void logging(std::string str, int count, int type_size) {
  void *array[10];
  char **strings;
  int size;
  size = backtrace (array, 10);
  strings = backtrace_symbols (array, size);
  if (strings != NULL)
  {
    #ifdef DEBUG
      std::string func_name1 = getFunc(strings[1]);
      std::string func_name2 = getFunc(strings[2]);
      log_calls_w_cs[func_name1 + " " + func_name2]++;
      fprintf(logger,"%s @ %s %s %d %s %d\n", str.c_str(), strings[2], " Count: ",count, " TypeSize: ", type_size);  
    #endif
  }
  free (strings);
}
void logging_dst(std::string str, int count, int type_size, std::string type, int dst) {
void *array[10];
  char **strings;
  int size;

  size = backtrace (array, 10);
  strings = backtrace_symbols (array, size);
  if (strings != NULL)
  {
#ifdef DEBUG
  std::string func_name1 = getFunc(strings[1]);
  std::string func_name2 = getFunc(strings[2]);
  log_calls_w_cs[func_name1 + " " + func_name2]++;
  fprintf(logger,"%s @ %s %s %d %s %d %s %d\n", str.c_str(), strings[2]," Count: ",count, " TypeSize: ", type_size, type.c_str(), dst);
#endif
  }
  free (strings);
}
long initBinSize(char * ptr) {
    long defaultBinSize=102400, binSize=0;
    if(ptr == NULL)
        return defaultBinSize;
    int digit=0;
    for(int idx=0; idx<std::strlen(ptr);idx++) {
        if(std::isdigit(ptr[idx])) {
            digit=ptr[idx]-'0';
            binSize = 10*binSize+digit;
        }
        else
            return defaultBinSize;
    }
    // std::cout << "BIN_SIZE: "<<binSize << "\n";
    return binSize > 0 ? binSize : defaultBinSize;
}
int MPI_Init(int *argc, char ***argv) {
  // std::cout<<"init\n";
  parent_func = __FUNCTION__;
  // std::cout << "In Init\n";
#ifdef LOG_MPI_CALLS
  log_calls[__FUNCTION__]++;
#endif
  // Default BIN_SIZE for count matrix
  BIN_SIZE = initBinSize(getenv("BIN_SIZE"));
  std::cout<<"BIN_SIZE: "<<BIN_SIZE<<"\n";
  /********Get HOME directory*******/
  struct passwd *pw = getpwuid(getuid());
  homedir = pw->pw_dir;
  // Get the PROFILE_DIR to store the profile files
  char *PROFILE_DIR = getenv("PROFILE_DIR");

  int result = PMPI_Init(argc, argv);
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  PMPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  char logName[100];

  char *INFO_DIR=getenv("INFO_DIR");
  char info_fname[200];
  sprintf(info_fname, "%s/info-%d.txt", INFO_DIR, rank);
  info_file=fopen(info_fname,"w");

  if (PROFILE_DIR != NULL) {
    // std::cout<<"PROFILE_DIR: "<<PROFILE_DIR<<"\n";
    sprintf(logName, "%s/ipmpi-%d.log", PROFILE_DIR, rank);
  } else {
    sprintf(logName, "%s/ipmpi_profiles/ipmpi-%d.log", homedir, rank);
  }
  // sprintf(test_fname,"/users/mtech/shivama/lbw_test/sample.txt");
  logger= fopen(logName, "w+");
  // std::cout << "In Init\n";

  // Reset the communication matrix with -1 implying no communication initially
  int val = -1;
  reset_comm_matrix(comm_matrix_sum, val);
  reset_comm_matrix(comm_matrix_max, val);
  reset_comm_matrix(comm_matrix_min, val);
  reset_comm_matrix(p2p_mat, val);
  reset_count_matrix(comm_matrix_count);
  init_median_lookup () ;
  init_realtime_median_lookup ();
  // printf ("done rank %d\n", rank); // pranjal debug`
  return result;
}
int MPI_Init_thread(int *argc, char ***argv, int required, int *provided) {
  parent_func = __FUNCTION__;
  // std::cout << "In Init\n";
#ifdef LOG_MPI_CALLS
  log_calls[__FUNCTION__]++;
#endif
#ifdef START_FUNC
  std::cout <<"In Init thread\n";
#endif
  // Default BIN_SIZE for count matrix
  BIN_SIZE = initBinSize(getenv("BIN_SIZE"));
  std::cout<<"BIN_SIZE: "<<BIN_SIZE<<"\n";
  /********Get HOME directory*******/
  struct passwd *pw = getpwuid(getuid());
  homedir = pw->pw_dir;
  // Get the PROFILE_DIR to store the profile files
  char *PROFILE_DIR = getenv("PROFILE_DIR");
  
  int result = PMPI_Init_thread(argc, argv, required, provided);
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  PMPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  char logName[100];
  
  if (PROFILE_DIR != NULL) {
    // std::cout<<"PROFILE_DIR: "<<PROFILE_DIR<<"\n";
    sprintf(logName, "%s/ipmpi-%d.log", PROFILE_DIR, rank);
  } else {
    sprintf(logName, "%s/ipmpi_profiles/ipmpi-%d.log", homedir, rank);
  }

  logger= fopen(logName, "w+");

  // Reset the communication matrix with -1 implying no communication initially
  int val = -1;
  reset_comm_matrix(comm_matrix_sum, val);
  reset_comm_matrix(comm_matrix_max, val);
  reset_comm_matrix(comm_matrix_min, val);
  reset_comm_matrix(p2p_mat, val);
  reset_count_matrix(comm_matrix_count);
  return result;
}
int MPI_Finalize() {
  char pname[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(pname, &name_len);

  // char *INFO_DIR=getenv("INFO_DIR");
  // char info_fname[200];
  // sprintf(info_fname, "%s/info-%d.txt", INFO_DIR, rank);
  // info_file=fopen(info_fname,"w");

  // std::cout<<"rank: "<<rank<<" sendrecv: "<<scall<<"\n";

  fprintf(info_file, "%d %s\n", rank, pname);

  // for(auto &e: fmap){
  //   fprintf(info_file, "%d %s\n", e.first, e.second.c_str());
  //   // std::cout<<e.first<<" "<<e.second<<"\n";
  // }

  for(auto &e: comm_steps){
    for(auto &vec: e){
      // fprintf(info_file, "%d %d %d %d\n", vec[0], vec[1], vec[2], vec[3]);
      // fprintf(info_file, "%d %d %d %d %ld\n", vec[0], vec[1], vec[2], vec[3], vec[4]); // pranjal`
      fprintf(info_file, "%d %d %d %d %ld %ld %ld\n", vec[0], vec[1], vec[2], vec[3], vec[4], vec[5], vec[6]); // pranjal
      // fprintf(stdout, "%d %d %d %d %ld %ld %ld\n", vec[0], vec[1], vec[2], vec[3], vec[4], vec[5], vec[6]); // pranjal
    }
  }

  fprintf(info_file, "*\n");

  for(auto &e : sr_calls){
    fprintf(info_file, "%d %d %d %d\n", e[0], e[1], e[2], e[3]);
  }

  fprintf(info_file, "*\n");

  for(auto &e: send_calls){
    fprintf(info_file, "%ld %ld %ld %ld %ld %ld\n", e[0], e[1], e[2], e[3], e[4], e[5]);
  }

  fclose(info_file);
  parent_func = __FUNCTION__;
#ifdef LOG_MPI_CALLS
  log_calls[__FUNCTION__]++;
#endif
#ifdef START_FUNC
  std::cout <<"In Finalize\n";
#endif
  // Logging comm_matrix:
  std::string s, st;
  populateCommMatrix(comm_matrix_sum, s, "Communication Sum Matrix");
  fprintf(logger,"%s", s.c_str());
  populateCommMatrix(comm_matrix_max, s, "Communication Max Matrix");
  fprintf(logger,"%s", s.c_str());
  populateCommMatrix(comm_matrix_min, s, "Communication Min Matrix");
  fprintf(logger,"%s", s.c_str());
  populateCommMatrix(p2p_mat, s, "P2P Matrix");
  fprintf(logger,"%s", s.c_str());

  populateCountMatrix(comm_matrix_count, st, "Count Matrix");
  
  // fprintf(logger,"%s", s.c_str());
  fprintf(logger,"%s\n", st.c_str());
  
  // Logging MPI calls
  for(auto &x:log_calls) {
    fprintf(logger, "%s %d\n", x.first.c_str(), x.second);
  }
  // // Logging MPI calls with callsites
  // for(auto &x:log_calls_w_cs) {
  //   fprintf(logger, "%s %d\n", x.first.c_str(), x.second);
  // }
  
  // // Logging #sends per MPI_Collectives
  // for(auto &x:sends) {
  //   fprintf(logger, "%s\n", x.first.c_str());
  //   for(int i=0;i<comm_size;i++) {
  //     for(int j=0;j<comm_size;j++) {
  //       fprintf(logger, "%d\t", x.second[i][j]);
  //     }
  //     fprintf(logger, "%s", "\n");
  //   }
  //   fprintf(logger, "%s", "\n");
  // }

  fclose(logger);
  free (all_names);
  // fprintf(stderr, "[%d] wrapping up\n", rank); // pranjal wants to use stderr for output
  return PMPI_Finalize();
}

int MPI_Send(const void *buffer, int count, MPI_Datatype datatype, int dest,
             int tag, MPI_Comm comm) {
  // printf("MPI Send called\n");
  parent_func = __FUNCTION__;
#ifdef LOG_MPI_CALLS
  log_calls[__FUNCTION__]++;
#endif

  // long fid=total_calls;
  send_count++;
  long total_calls=send_count;

  int size;
  PMPI_Type_size(datatype, &size); /* Compute size */  
  
#ifdef START_FUNC
  std::cout << "In Send "<< (size*count) <<"\n";
#endif
  // Creating a p2p matrix
  p2p_mat[rank][dest] += p2p_mat[rank][dest] == -1 ? 1+ size * count : size * count;
  p2p_mat[dest][rank] += p2p_mat[dest][rank] == -1 ? 1+ size * count : size * count;

  std::vector<long> tmp={total_calls, -1, 0, rank, dest, size*count};   // 0 is the step here, as there is only one step involved
  // temp_steps.push_back(tmp);

  // for(auto e: temp_steps){
  //   for(auto a: e)
  //     std::cout<<a<<" ";
  // }

  updateCommMatrix(size * count, rank, dest);

  int result = PMPI_Send(buffer, count, datatype, dest, tag, comm);
#ifdef DEST_LOG
  logging_dst("MPI_Send", count, size, " Dst: ", dest);
#endif
  // comm_steps.push_back(temp_steps);
  send_calls.push_back(tmp);
  return result;
  
}

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
             MPI_Comm comm, MPI_Status *status) {
  parent_func = __FUNCTION__;
#ifdef LOG_MPI_CALLS
  log_calls[__FUNCTION__]++;
#endif  

  int actual_count, size;
  int result = PMPI_Recv(buf, count, datatype, source, tag, comm, status);
  PMPI_Type_size(datatype, &size);                 /* Compute size */
  PMPI_Get_count(status, datatype, &actual_count); /* Compute count */
#ifdef DEST_LOG
  logging_dst("MPI_Recv", count, size, " Src: ", source);
#endif
  return result;
}

int MPI_Isend(const void *buffer, int count, MPI_Datatype datatype, int dest,
             int tag, MPI_Comm comm, MPI_Request *request) {
  parent_func = __FUNCTION__;
#ifdef LOG_MPI_CALLS
  log_calls[__FUNCTION__]++;
#endif
#ifdef START_FUNC
  std::cout <<"In " << parent_func << "\n";
#endif
  // std::cout << "In Send\n";
  send_count++;
  long total_calls=send_count;

  int size;
  PMPI_Type_size(datatype, &size); /* Compute size */  

  // Creating a p2p matrix
  p2p_mat[rank][dest] += p2p_mat[rank][dest] == -1 ? 1+ size * count : size * count;
  p2p_mat[dest][rank] += p2p_mat[dest][rank] == -1 ? 1+ size * count : size * count;

  // std::vector<std::vector<long> > temp_steps;
  std::vector<long> tmp={total_calls, -1, 0, rank, dest, size*count};   // 0 is the step here, as there is only one step involved
  // temp_steps.push_back(tmp);
  
  updateCommMatrix(size * count, rank, dest);

  int result = PMPI_Isend(buffer, count, datatype, dest, tag, comm, request);
#ifdef DEST_LOG
  logging_dst("MPI_Isend", count, size, " Dst: ", dest);
#endif
  // comm_steps.push_back(temp_steps);
  send_calls.push_back(tmp);
  return result;
}
int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
             MPI_Comm comm, MPI_Request * request) {
  parent_func = __FUNCTION__;
#ifdef LOG_MPI_CALLS
  log_calls[__FUNCTION__]++;
#endif  
  int actual_count, size;
  int result = PMPI_Irecv(buf, count, datatype, source, tag, comm, request);
  PMPI_Type_size(datatype, &size);                 /* Compute size */
  // PMPI_Get_count(status, datatype, &actual_count); /* Compute count */
#ifdef DEST_LOG
  logging_dst("MPI_Irecv", count, size, " Src: ", source);
#endif
  return result;
}

int MPI_Sendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, int dest, int sendtag, 
  void* recvbuf, int recvcount, MPI_Datatype recvtype, int source, int recvtag, MPI_Comm comm, MPI_Status *status) {
  parent_func = __FUNCTION__;
#ifdef LOG_MPI_CALLS
    log_calls[__FUNCTION__]++;
#endif  
  #ifdef START_FUNC
    std::cout <<"In " << parent_func << "\n";
  #endif

    scall++;

    long total_calls=scall;
    std::string fname="sendrecv";
    // fmap[total_calls]=fname;
    int sendtype_size;
    PMPI_Type_size(sendtype, &sendtype_size); /* Compute size */
    
    // Creating a p2p matrix
    p2p_mat[rank][dest] += p2p_mat[rank][dest] == -1 ? 1+ sendcount*sendtype_size : sendcount*sendtype_size;
    p2p_mat[dest][rank] += p2p_mat[dest][rank] == -1 ? 1+ sendcount*sendtype_size : sendcount*sendtype_size;

    std::vector<long> tmp={total_calls, rank, dest, sendcount*sendtype_size};   // 0 is the step here, as there is only one step involved

    updateCommMatrix(sendcount*sendtype_size, rank, dest);
    // fprintf(logger,"%s", "MPI_SendRecv\n");
    // logging("MPI_SendRecv", sendcount, sendtype_size);
#ifdef DEST_LOG
    logging_dst("MPI_Sendrecv", sendcount, sendtype_size, " Dst: ", dest);
#endif
    int result = PMPI_Sendrecv(sendbuf, sendcount, sendtype, dest, sendtag, recvbuf, recvcount, recvtype, source, recvtag, comm,status);
    // fprintf(logger,"%d %d\n", rank, status->MPI_SOURCE);
    // comm_steps.push_back(temp_steps);
    sr_calls.push_back(tmp);

    return result;
}

int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root,
              MPI_Comm comm) {
  std::vector<std::vector<long>>  temp_steps;
  fid++;
  long total_calls=fid;
  std::string fname="bcast";
  fmap[total_calls]=fname;
  parent_func = __FUNCTION__;
  #ifdef LOG_MPI_CALLS
    log_calls[__FUNCTION__]++;
  #endif
  #ifdef START_FUNC
    std::cout <<"In " << parent_func << "\n";
  #endif
  MPI_Aint nbytes = 0;
  int type_size;
  PMPI_Type_size(datatype, &type_size); /* Compute size */
  nbytes = type_size * count;
  /* Profiling Starts */
  if((nbytes > 0) && (comm_size > 1)) {
    if((nbytes < MPIR_CVAR_BCAST_SHORT_MSG_SIZE) || ((comm_size < MPIR_CVAR_BCAST_MIN_PROCS))) {
      // Condition: nbytes < 12KB || comm_size < 8
      // fprintf(logger, "%s", "MPI_Bcast_intra_binomial\n");
#ifdef DEST_LOG
      logging_dst("MPI_Bcast_intra_binomial", count, type_size, " Root: ", root);
#endif
      profile_Bcast_intra_binomial(root, type_size, count, temp_steps, total_calls);
      // profile_Bcast_intra_binomial(root, type_size, count);
    } else {
      if((nbytes < MPIR_CVAR_BCAST_LONG_MSG_SIZE) && !(comm_size & (comm_size - 1))){
        // Condition: nbytes < 512KB && comm_size is pof2
        // fprintf(logger, "%s", "MPI_Bcast_intra_scatter_recursive_doubling_allgather\n");
        // logging("MPI_Bcast_intra_scatter_recursive_doubling_allgather", count, type_size);
#ifdef DEST_LOG
        logging_dst("MPI_Bcast_intra_scatter_recursive_doubling_allgather", count, type_size, " Root: ", root);
#endif
        profile_Bcast_intra_scatter_recursive_doubling_allgather(root, type_size, count, temp_steps, total_calls);
      } else {
        // Implement profile_Bcast_intra_scatter_ring_allgather
#ifdef DEST_LOG
        logging_dst("MPI_Bcast_intra_scatter_ring_allgather", count, type_size, " Root: ", root);
#endif
        // logging("MPI_Bcast_intra_scatter_ring_allgather", count, type_size);
        // fprintf(logger, "%s", "MPI_Bcast_intra_scatter_ring_allgather\n");
        profile_Bcast_intra_scatter_ring_allgather(root, type_size, count, temp_steps, total_calls);
      }
    } 
  }
  // int result = PMPI_Bcast(buffer, count, datatype, root, comm);
  int result=0;
  time_collective_from_temp_steps (temp_steps);
  time_collective_from_temp_steps_realtime (temp_steps);
  comm_steps.push_back(temp_steps);

  // fprintf(stderr, "[%d] bcast %d %d\n", rank, count * size, root);
  // fprintf(logger,"%d %d %d %d\n", rank, Bcast, count * size, root);
  return result;
}
/* Some problem needs to be fixed */
int MPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
                MPI_Comm comm) {
  std::vector<std::vector<long>>  temp_steps;
  fid++;
  long total_calls=fid;
  std::string fname="scatter";
  fmap[total_calls]=fname;
  parent_func = __FUNCTION__;
  #ifdef LOG_MPI_CALLS
    log_calls[__FUNCTION__]++;
  #endif
  #ifdef START_FUNC
    std::cout <<"In " << parent_func << "\n";
  #endif
  int sendtype_size, recvtype_size;
  int result = PMPI_Scatter(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                            recvtype, root, comm);
  PMPI_Type_size(sendtype, &sendtype_size); /* Compute size */
  PMPI_Type_size(recvtype, &recvtype_size); /* Compute size */
  profile_Scatter_binomial(root, sendtype_size, sendcount, recvtype_size, recvcount, temp_steps, total_calls);
  // logging("MPI_Scatter_Binomial", sendcount, sendtype_size);
#ifdef DEST_LOG
  logging_dst("MPI_Scatter_Binomial", sendcount, sendtype_size, " Root: ", root);
#endif
  // fprintf(stderr, "[%d] scatter %d %d\n", rank, sendcount * size, root);
  // fprintf(logger,"%d %d %d %d\n", rank, Scatter, sendcount * size, root);
  comm_steps.push_back(temp_steps);
  // int result=0;
  return result;
}

int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
               void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
               MPI_Comm comm) {
  std::vector<std::vector<long>>  temp_steps;
  fid++;
  long total_calls=fid;
  std::string fname="gather";
  fmap[total_calls]=fname;
  parent_func = __FUNCTION__;
  #ifdef LOG_MPI_CALLS
    log_calls[__FUNCTION__]++;
  #endif
  #ifdef START_FUNC
    std::cout <<"In " << parent_func << "\n";
  #endif
  profile_Gather_binomial(root, sendtype, sendcount, recvtype, recvcount, temp_steps, total_calls);
  int result = PMPI_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                           recvtype, root, comm);
  int sendtype_size;
  PMPI_Type_size(sendtype, &sendtype_size);
  // logging("MPI_Gather_Binomial", sendcount, sendtype_size);
#ifdef DEST_LOG
  logging_dst("MPI_Gather_Binomial", sendcount, sendtype_size, " Root: ", root);
#endif
  // fprintf(logger,"MPI_Gather_Binomial\n");
  comm_steps.push_back(temp_steps);
  // int result=0;
  return result;
}
/*********************** START OF TUSHAR's Code ***********************************/


// Change function body from here
int MPI_Reduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
               MPI_Op op, int root, MPI_Comm comm) {
  std::vector<std::vector<long>>  temp_steps;
  fid++;
  long total_calls=fid;
  std::string fname="reduce";
  fmap[total_calls]=fname;
  parent_func = __FUNCTION__;
  #ifdef LOG_MPI_CALLS
    log_calls[__FUNCTION__]++;
  #endif
  #ifdef START_FUNC
    std::cout<<"in reduce\n";
  #endif
  int type_size;
  int result = PMPI_Reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
  
  // int result=0;
  PMPI_Type_size(datatype, &type_size); /* Compute size */
  
  int pof2 = (int)pow(2, (int)log2(comm_size));
  if ((count * type_size > MPIR_CVAR_REDUCE_SHORT_MSG_SIZE) &&
      (HANDLE_GET_KIND(op) == HANDLE_KIND_BUILTIN) && (count >= pof2)) {
      /* do a reduce-scatter followed by gather to root. */
      // Condition: count*type_size > 2KB and Built-in Op and count >=pof2
      profile_Reduce_intra_reduce_scatter_gather(datatype, root, count, temp_steps, total_calls);
      // fprintf(logger,"MPI_reduce_scatter_gather\n");
      // logging("MPI_reduce_scatter_gather", count, type_size);
#ifdef DEST_LOG
      logging_dst("MPI_reduce_scatter_gather", count, type_size, " Root: ", root);
#endif
  } else {
      /* use a binomial tree algorithm */
      profile_Reduce_intra_binomial(datatype, root, count, op, temp_steps, total_calls);
      // fprintf(logger,"MPI_reduce_intra_binomial\n");
#ifdef DEST_LOG
      logging_dst("MPI_reduce_intra_binomial", count, type_size, " Root: ", root);
#endif
      // logging("MPI_reduce_intra_binomial", count, type_size);
  }
  // profile_Reduce_intra_reduce_scatter_gather(datatype, root, count, temp_steps, total_calls);
  // time_collective_from_temp_steps (temp_steps); // pranjal
  // time_collective_from_temp_steps_realtime (temp_steps); // pranjal
  // comm_steps.push_back(temp_steps); // pranjal - not profiling reduce atm
  // fprintf(logger,"MPI_Reduce\n");
  return result;
}

int MPI_Allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
               void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {
  std::vector<std::vector<long>>  temp_steps;
  fid++;
  long total_calls=fid;
  std::string fname="allgather";
  fmap[total_calls]=fname;
  parent_func = __FUNCTION__;
  #ifdef LOG_MPI_CALLS
    log_calls[__FUNCTION__]++;
  #endif
  #ifdef START_FUNC
    std::cout <<"In " << parent_func << "\n";
  #endif
  MPI_Aint tot_bytes;
  // papi_log_compute();
  int type_size;
  PMPI_Type_size(recvtype, &type_size); /* Compute size */
  // Implement our algorithm for profiling the code:
  tot_bytes = (MPI_Aint) recvcount *comm_size * type_size;
  if((tot_bytes < MPIR_CVAR_ALLGATHER_LONG_MSG_SIZE) && !(comm_size & (comm_size - 1))) {
    // Condition: tot_bytes < 512KB and np is pof2
    // Intra_recursive_doubling
    profile_Allgather_intra_recursive_doubling(recvcount, type_size, temp_steps, total_calls);
    // fprintf(logger,"%s", "Allgather_RD\n");
    logging("Allgather_RD", sendcount, type_size);
  } else if (tot_bytes < MPIR_CVAR_ALLGATHER_SHORT_MSG_SIZE) {
    // Condition: tot_bytes < 80KB
    // INTRA_BRUCKS
    profile_Allgather_intra_brucks(recvcount, type_size, temp_steps, total_calls);
    // fprintf(logger,"%s", "Allgather_Brucks\n");
    logging("Allgather_Brucks", sendcount, type_size);
  } else {
    // INTRA_RING
    profile_Allgather_intra_ring(recvcount, type_size, temp_steps, total_calls);
    // fprintf(logger, "%s", "Allgather_Ring\n");
    logging("Allgather_Ring", sendcount, type_size);
  }
  
  // int result = PMPI_Allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount,
  //                          recvtype, comm);
  // printf("result: %d\n", result);
  int result=0;
  time_collective_from_temp_steps (temp_steps);
  time_collective_from_temp_steps_realtime (temp_steps);
  comm_steps.push_back(temp_steps);
  // fprintf(logger, "MPI_Allgather\n");
  return result;
}

int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
               MPI_Op op, MPI_Comm comm) {
  std::vector<std::vector<long>>  temp_steps;
  fid++;
  long total_calls=fid;
  std::string fname="allreduce";
  fmap[total_calls]=fname;
  parent_func = __FUNCTION__;
  #ifdef LOG_MPI_CALLS
    log_calls[__FUNCTION__]++;
  #endif
  #ifdef START_FUNC
    std::cout <<"In " << parent_func << "\n";
  #endif
  int type_size;
  int pof2 = (int)pow(2, (int)log2(comm_size));
  PMPI_Type_size(datatype, &type_size); /* Compute size */
  MPI_Aint nbytes = 0;
  nbytes = type_size * count;
  if ((nbytes <= MPIR_CVAR_ALLREDUCE_SHORT_MSG_SIZE) ||
        (HANDLE_GET_KIND(op) != HANDLE_KIND_BUILTIN) || (count < pof2)) {
    // nbytes <= 2KB || count < pof2
    profile_Allreduce_intra_recursive_doubling(count, type_size, temp_steps, total_calls);
    // fprintf(logger,"%s", "Allreduce_intra_recursive_doubling\n");
    logging("Allreduce_intra_recursive_doubling", count, type_size);
    // fprintf(logger,"%s %d %s", "Allreduce_intra_recursive_doubling ", count , "\n");
  } else {
    profile_Allreduce_intra_reduce_scatter_allgather(count, type_size, temp_steps, total_calls);
    logging("Allreduce_intra_reduce_scatter_allgather", count, type_size);
    // fprintf(logger,"%s %d %d %s", "Allreduce_intra_reduce_scatter_allgather - Count: ", count , " ", type_size, "\n");
  }
  int result = PMPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
  // int result=0;
  comm_steps.push_back(temp_steps);
  // fprintf(logger, "MPI_Allreduce\n");
  
  return result;
}
int MPI_Alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
               int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {
  std::vector<std::vector<long>>  temp_steps;
  fid++;
  
  long total_calls=fid;
  std::string fname="alltoall";
  fmap[total_calls]=fname;
  parent_func = __FUNCTION__;
  #ifdef LOG_MPI_CALLS
    log_calls[__FUNCTION__]++;
  #endif
  #ifdef START_FUNC
    std::cout <<"In " << parent_func << "\n";
  #endif
  int sendtype_size, nbytes;
  PMPI_Type_size(sendtype, &sendtype_size); /* Compute size */
  nbytes = sendtype_size * sendcount;
  
  if (sendbuf == MPI_IN_PLACE) {
    profile_Alltoall_intra_pairwise_sendrecv_replace(sendtype, recvtype, sendcount, recvcount, temp_steps, total_calls);
    fprintf(logger,"%s", "Alltoall_intra_pairwise_sendrecv_replace\n");
  } else if ((nbytes <= MPIR_CVAR_ALLTOALL_SHORT_MSG_SIZE) && (comm_size >= 8)) {
    profile_Alltoall_intra_brucks(sendtype, recvtype, sendcount, recvcount, temp_steps, total_calls);
    fprintf(logger,"%s", "Alltoall_intra_brucks\n");
  } else if (nbytes <= MPIR_CVAR_ALLTOALL_MEDIUM_MSG_SIZE) {
    profile_Alltoall_intra_scattered(sendtype, recvtype, sendcount, recvcount, temp_steps, total_calls);
    fprintf(logger,"%s", "Alltoall_intra_scattered\n");
  } else {
    profile_Alltoall_intra_pairwise(sendtype, recvtype, sendcount, recvcount, temp_steps, total_calls);
    fprintf(logger,"%s", "Alltoall_intra_pairwise\n");
  }
  
  // profile_Alltoall_intra_scattered(sendtype, recvtype, sendcount, recvcount, temp_steps, total_calls);
  // int result = PMPI_Alltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                           // recvtype, comm);
  int result=0;
  // fprintf(logger, "MPI_Alltoall\n");
  time_collective_from_temp_steps (temp_steps);
  time_collective_from_temp_steps_realtime (temp_steps);
  comm_steps.push_back(temp_steps);  // ordfer changed as time_collective* modifies temp_steps
  return result;
}

int MPI_Barrier(MPI_Comm comm) {
  parent_func = __FUNCTION__;
  #ifdef LOG_MPI_CALLS
    log_calls[__FUNCTION__]++;
  #endif
  // Profile Barrier
  // profile_Barrier();
  int result = PMPI_Barrier(comm);

  return result;  
}
