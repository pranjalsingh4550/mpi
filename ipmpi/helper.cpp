#include "include/helper.h"

// std::vector<int> vals;

void reset_comm_matrix(std::vector<std::vector<long>> &comm_matrix, int value) {
  int i,j;
  // Resetting the comm_matrix with zero
  comm_matrix.clear();
  for(i=0;i<comm_size;i++) {
    comm_matrix.push_back({value});
    for(j=1;j<comm_size;j++) {
      comm_matrix[i].push_back(value);
    }
  }
}
void reset_count_matrix(std::vector<std::vector<std::vector<int>>> &comm_matrix) {
  int i,j,k;
  // Resetting the comm_matrix with zero
  comm_matrix.clear();
  for(i=0;i<comm_size;i++) {
    comm_matrix.push_back({{0}});
    for(j=1;j<comm_size;j++) {
      comm_matrix[i].push_back({0});
    }
  }
}
// long getBinNumber(long number) {
//   return number / BIN_SIZE;
// }

// inline void updateCommMatrix(long msg_size, int src, int dst) {
  
//   /*****************Updating the comm_matrix******************/
//   sends[parent_func][src][dst]++;
  
//   if(comm_matrix_sum[src][dst] == -1)
//     comm_matrix_sum[src][dst]++;
//   if(comm_matrix_sum[dst][src] == -1)
//     comm_matrix_sum[dst][src]++;
  
//   // Updating the sum matrix
//   comm_matrix_sum[src][dst] += msg_size;
//   comm_matrix_sum[dst][src] += msg_size;
  
//   // Updating the max matrix
//   comm_matrix_max[src][dst] = std::max(comm_matrix_max[src][dst], msg_size);
//   comm_matrix_max[dst][src] = std::max(comm_matrix_max[src][dst], msg_size);

//   // Updating the min matrix
//   comm_matrix_min[src][dst] = (comm_matrix_min[src][dst] != -1) ? std::min(comm_matrix_min[src][dst], msg_size) : msg_size;
//   comm_matrix_min[dst][src] = (comm_matrix_min[dst][src] != -1) ? std::min(comm_matrix_min[dst][src], msg_size) : msg_size;
//   // return;
//   // Updating the count matrix
//   long bin = getBinNumber(msg_size);
//   long sender_size = comm_matrix_count[src][dst].size();
//   long receiver_size = comm_matrix_count[dst][src].size();
//   if(bin > sender_size)
//     comm_matrix_count[src][dst].resize(BIN_RESIZE_FACTOR*bin, BIN_RESIZE_VALUE);
//   if(bin > receiver_size)
//     comm_matrix_count[dst][src].resize(BIN_RESIZE_FACTOR*bin, BIN_RESIZE_VALUE);
//   comm_matrix_count[src][dst][bin]++;
//   comm_matrix_count[dst][src][bin]++;
// }
void populateCommMatrix(std::vector<std::vector<long>> &comm_matrix, std::string& s, std::string msg) {
  /**************For populating the sum, max, min matrix****************/  
  s="";
  s += msg;
  s += "\n";
  // Populating the sum, max or min matrix as specified:
  int m = comm_matrix.size(), n = comm_matrix[0].size(), i, j;
  for(i=0;i<m;i++) {
    for(j=0;j<n;j++) {
      s += std::to_string(comm_matrix[i][j]);
      s += "\t\t\t";
    }
    s += "\n"; 
  }
}
void populateCountMatrix(std::vector<std::vector<std::vector<int>>> &comm_matrix, std::string& s, std::string msg) {
  s="\n";
  s += msg;
  s += "\n";
  // Populating the sum, max or min matrix as specified:
  int m = comm_matrix.size(), n = comm_matrix[0].size(), i, j, k;
  for(i=0;i<m;i++) {
    for(j=0;j<n;j++) {
      s += std::to_string(i);
      s += " ";
      s += std::to_string(j);
      s += " ";
      for(k=0;k<comm_matrix[i][j].size();k++) {
        s += std::to_string(comm_matrix[i][j][k]);
        s += " ";
      }
      s += "\n"; 
    }
  }
}

// Method Complete => Verified
void profile_Bcast_intra_binomial(int root, int type_size, int count, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  // void profile_Bcast_intra_binomial(int root, int type_size, int count) { 
  int mask, relative_rank, src,dst;
  long msg_size = type_size * count;
  relative_rank = (rank >= root) ? (rank - root) : (rank - root + comm_size);
  mask = 0x1;
  int step=0;
  while(mask < comm_size) {
    if(relative_rank & mask) {
      src = (rank - mask + comm_size) % comm_size;
      // step++;
      break;
    }
    mask <<= 1;
  }
  mask >>= 1;
  int diff;
  while (mask > 0) {
    if (relative_rank + mask < comm_size) {
      dst = rank + mask;
      if (dst >= comm_size)
        dst -= comm_size;
      int relative_dst=(dst>=root)?(dst-root):(dst-root+comm_size);
      diff=abs(relative_dst-relative_rank);
      // step=(int)log2(diff); // commented by pranjal
      step = (int) log2 (comm_size / diff)  - 1;
      // printf("rank: %d, relative_rank: %d, dst: %d, relative_dst: %d, diff: %d, step: %d\n",rank,relative_rank,dst,relative_dst,diff,step);
      // std::vector<long> tmp={total_calls, step, rank, dst};
      std::vector<long> tmp={total_calls, step, rank, dst, msg_size};
      temp_steps.push_back(tmp);
      updateCommMatrix(msg_size, rank, dst);
    }
    mask >>= 1;
    // vals.push_back(rank);
  }  
}


void profile_Scatter_for_bcast(int root, int nbytes, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  // printf ("hello there, profile_Scatter_for_bcast. rank %d\n", rank); // pranjal
  MPI_Barrier (MPI_COMM_WORLD) ; // pranjal
  int mask, relative_rank, src, scatter_size, curr_size, recv_size, send_size, dst;
  relative_rank = (rank >= root) ? (rank - root) : (rank - root + comm_size);
  // nbytes = type_size * count;
  scatter_size = (nbytes + comm_size - 1) / comm_size;  // Ceiling division
  curr_size = (rank == root) ? nbytes : 0;
  mask = 0x1;
  int step=0;
  while(mask < comm_size) {
	// printf ("In the while loop, mask is %d, comm size is %d. rank %d, relative_rank %d\n", mask, comm_size,rank, relative_rank); // pranjal
    if(relative_rank & mask) {
      // std::cout<<"src: "<<src<<" rank: "<<rank<<"\n";
      src = (rank - mask + comm_size) % comm_size;
      recv_size = nbytes - relative_rank * scatter_size;
      if (recv_size <= 0) {
        curr_size = 0;  /* this process doesn't receive any data
                            * because of uneven division */
      } else {
        curr_size = recv_size;
        // mpi_errno = MPIC_Recv(((char *) tmp_buf +
        //                         relative_rank * scatter_size),
        //                       recv_size, MPI_BYTE, src,
        //                       MPIR_BCAST_TAG, comm_ptr, &status, errflag);
        // } else
        //     /* query actual size of data received */
        //     MPIR_Get_count_impl(&status, MPI_BYTE, &curr_size);
      }
      break;
    }
    mask <<= 1;
  }
  curr_size = nbytes * mask ; // pranjal // NOTE - THIS PART DEALS WITH THE SEND VALUES
  curr_size /= (comm_size); // pranjal // ONLY FOR POWER-OF-2 SIZE
  mask >>= 1;
  int diff;
  MPI_Barrier(MPI_COMM_WORLD) ; // pranjal
  // printf ("After the while loop - mask %d, relative rank %d, curr_size %d recv_size %d\n", mask, relative_rank, curr_size, recv_size); // pranjal
  sleep(1); // pranjal
  MPI_Barrier (MPI_COMM_WORLD); // pranjal
  step = 0;
  while (mask > 0) {
    if (relative_rank + mask < comm_size) { // if condition is always true
      send_size = curr_size - scatter_size * mask; // ERROR HERE
      // printf ("Second while loop - rank %d, mask %d, send_size %d | curr %d scatter %d\n", rank, mask, send_size, curr_size, scatter_size); // pranjal
      /* mask is also the size of this process's subtree */
      // if (send_size > 0) { // pranjal commented this
      if (scatter_size * mask * 2 == nbytes) {
	  // printf ("rank %d going to send step %d mask %d\n", rank, step, mask);
          dst = rank + mask;
          if (dst >= comm_size)
              dst -= comm_size;
          
          // std::cout << "\nSender: " << rank << " Receiver "<< dst <<" send_size: " << send_size << " Mask: "<< mask <<"\n";
          // if (rank == 0) std::cout<<"rank: "<<rank<<" dst: "<<dst<<"\n";
	  // MPI_Barrier (MPI_COMM_WORLD); // pranjal
          
          // // std::vector<long> tmp={total_calls, step, rank, dst};
          int relative_dst=(dst>=root)?(dst-root):(dst-root+comm_size);
          diff=abs(relative_dst-relative_rank);
          // step=(int)log2(diff); // commented bt pranjal
	  // step = 55; // pranjal
          // printf("mask: %d, step: %d, rank: %d, dst: %d\n", mask, step,rank,dst); // uncommented by pranjal
          std::vector<long> tmp={total_calls, step, rank, dst, send_size};
          temp_steps.push_back(tmp);
          updateCommMatrix(send_size, rank, dst);
          curr_size -= send_size;
          mask >>= 1;
      }
      step ++ ;
    }
    nbytes = nbytes >> 1 ;
  }
  // printf ("Done rank %d\n", rank); // pranjal
}
// Method Coding Done
void profile_Bcast_intra_scatter_recursive_doubling_allgather(int root, int type_size, int count, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  
  int mask, relative_rank, src, nbytes, scatter_size, curr_size, recv_size, relative_dst, dst;
  long msg_size = type_size * count;
  relative_rank = (rank >= root) ? (rank - root) : (rank - root + comm_size);
  nbytes = type_size * count;
  scatter_size = (nbytes + comm_size - 1) / comm_size;
  curr_size = std::min(scatter_size, (nbytes - (relative_rank * scatter_size)));
  // Scatter First
  profile_Scatter_for_bcast(root, nbytes, temp_steps, total_calls);

  // Apply recursive Doubling
  MPI_Barrier (MPI_COMM_WORLD) ; // PRANJAL
  // if (rank == 0) printf ("BEGINNING ALLGATHER\n"); // pranjal
  int step;
  int diff;
  int prev_steps=(int)log2(comm_size);
  mask = 0x1;
  while(mask < comm_size) {
    relative_dst = (relative_rank + mask)%comm_size; // was bitwise xor earlier
    dst = (relative_dst + root) % comm_size;
    if(relative_dst < comm_size) {
      // std::vector<long> tmp={total_calls, step, rank, dst};
      // int relative_dst=(dst>=root)?(dst-root):(dst-root+comm_size);
      diff=abs(relative_dst-relative_rank);
      step=(int)log2(diff)+prev_steps;
      // printf("ALLGATHER: step: %d, rank: %d, dst: %d\n", step,rank,dst); // modified pranjal
      std::vector<long> tmp={total_calls, step, rank, dst, curr_size};
      temp_steps.push_back(tmp);
      // updateCommMatrix(curr_size, rank, dst);

      curr_size += curr_size;
    }
    mask <<=1;
  }
}

void profile_Bcast_intra_scatter_ring_allgather(int root, int type_size, int count, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  int mask, relative_rank, src, nbytes, scatter_size, curr_size, recv_size, relative_dst, dst;
  int left, right, j, i, jnext, recvd_size;
  relative_rank = (rank >= root) ? (rank - root) : (rank - root + comm_size);
  nbytes = type_size * count;
  scatter_size = (nbytes + comm_size - 1) / comm_size;
  curr_size = std::max(0,std::min(scatter_size, (nbytes - (relative_rank * scatter_size))));
  // Scatter First
  profile_Scatter_for_bcast(root, nbytes, temp_steps, total_calls);
  // Ring
  left = (comm_size + rank - 1) % comm_size;
  right = (rank + 1) % comm_size;
  j = rank;
  jnext = left;
  int prev_steps=(int)log2(comm_size);
  for (i = 1; i < comm_size; i++) {
      int left_count, right_count, rel_j, rel_jnext;
      rel_j = (j - root + comm_size) % comm_size;
      rel_jnext = (jnext - root + comm_size) % comm_size;
      left_count = std::max(0, std::min(scatter_size, (nbytes - rel_jnext * scatter_size)));
      right_count = std::max(0,std::min(scatter_size, (nbytes - rel_j * scatter_size)));
      
      int step=i-1+prev_steps;
      // printf("step: %d, rank: %d, dst: %d\n", step,rank,right);
      std::vector<long> tmp={total_calls, step, rank, right, right_count};
      temp_steps.push_back(tmp);
      
      updateCommMatrix(right_count, rank, right);
      
      
      curr_size += recvd_size;
      j = jnext;
      jnext = (comm_size + jnext - 1) % comm_size;
  }
}

// Method Completed => Verified
void profile_Allgather_intra_recursive_doubling(int &recvcount, int &type_size, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  // Use the comm_matrix defined globally
  // Each process rank will update its own communication with other process
  int mask, dst, curr_cnt;
  int step, diff;
  curr_cnt = recvcount;
  mask = 0x1;
  while (mask < comm_size) {
    dst = rank ^ mask;
    if (dst < comm_size) {
      step++;
      // std::vector<long> tmp={total_calls, step, rank, dst};
      // int relative_dst=(dst>=root)?(dst-root):(dst-root+comm_size);
      // diff=abs(relative_dst-relative_rank);
      diff=abs(dst-rank);
      step=(int)log2(diff);
      std::vector<long> tmp={total_calls, step, rank, dst, curr_cnt*mask*type_size};
      // printf("step: %d, rank: %d, dst: %d\n", step, rank, dst);
      temp_steps.push_back(tmp);
      updateCommMatrix(curr_cnt*mask*type_size,rank, dst);
    }
    mask <<= 1;
  }
}
// Method Completed => Verified
void profile_Allgather_intra_brucks(int &recvcount, int &type_size, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  int pof2, src, dst, rem, curr_cnt;
  curr_cnt = recvcount;
  pof2 = 1;
  int step, diff;
  while (pof2 <= comm_size / 2) {
      src = (rank + pof2) % comm_size;
      dst = (rank - pof2 + comm_size) % comm_size;
      //in case of bruck's algorithm, we need to calculate the distance in the left direction
      if(dst<rank)
        diff=rank-dst;
      else{
        diff=(rank-dst+comm_size);
      }
      step=(int)log2(diff);
      printf("step: %d, rank: %d, dst: %d\n", step, rank, dst);
      // std::vector<long> tmp={total_calls, step, src, dst};
      std::vector<long> tmp={total_calls, step, rank, dst, curr_cnt*type_size};
      temp_steps.push_back(tmp);
      updateCommMatrix(curr_cnt*type_size, rank, dst);
      curr_cnt *= 2;
      pof2 *= 2;
  }
  /* if comm_size is not a power of two, one more step is needed */
  int prev_steps=(int)log2(comm_size);
  rem = comm_size - pof2;
  if (rem) {
      src = (rank + pof2) % comm_size;
      dst = (rank - pof2 + comm_size) % comm_size;
      // step++;
      step=prev_steps;
      // std::vector<long> tmp={total_calls, step, src, dst};
      std::vector<long> tmp={total_calls, step, rank, dst, rem * recvcount * type_size};
      temp_steps.push_back(tmp);
      updateCommMatrix(rem * recvcount * type_size, rank, dst);
  }
}

// Method Completed => Verified
void profile_Allgather_intra_ring(int &recvcount, int &type_size, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  // int dst = (rank+1)%comm_size;
  // updateCommMatrix(recvcount*(comm_size-1)*type_size, rank, dst);
  int left, right, j, jnext, i;
  left = (comm_size + rank - 1) % comm_size;
  right = (rank + 1) % comm_size;
  j = rank;
  jnext = left;
  for (i = 1; i < comm_size; i++) {
    // printf("step: %d, rank: %d, dst: %d\n", i-1, rank, right);
    std::vector<long> tmp={total_calls, i-1, rank, right, recvcount * type_size};
    temp_steps.push_back(tmp);
    updateCommMatrix(recvcount*type_size, rank, right);
    j = jnext;
    jnext = (comm_size + jnext - 1) % comm_size;
  }
}

// Method Completed => Verified
void profile_Allreduce_intra_recursive_doubling(int count, int type_size, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  int pof2, rem, newrank, mask, newdst, dst;
  long msg_size;
  pof2 = (int)pow(2, (int)log2(comm_size));
  rem = comm_size - pof2;
  // int step=0;
  msg_size = count*type_size;
  // std::cout<<count<<" "<<type_size<<"\n";
  if (rank < 2 * rem) {
      if (rank % 2 == 0) {    /* even */
          updateCommMatrix(msg_size, rank, rank+1);
          newrank = -1;
      } else {        /* odd */
        newrank = rank / 2;
      }
  } else      /* rank >= 2*rem */
    newrank = rank - rem;
  int step, diff;
  if (newrank != -1) {
        mask = 0x1;
        while (mask < pof2) {
            newdst = newrank ^ mask;
            /* find real rank of dest */
            dst = (newdst < rem) ? newdst * 2 + 1 : newdst + rem;
            diff=abs(dst-rank);
            step=(int)log2(diff);
            std::vector<long> tmp={total_calls, step, rank, dst, msg_size};
            temp_steps.push_back(tmp);
            // printf("rank: %d, dest: %d\n", rank, dst);
            updateCommMatrix(msg_size, rank, dst);  // Sendrecv -> Will be counted twice overall
            
            mask <<= 1;
        }
    }
    /* In the non-power-of-two case, all odd-numbered
     * processes of rank < 2*rem send the result to
     * (rank-1), the ranks who didn't participate above. */
    if (rank < 2 * rem) {
      if (rank % 2)   /* odd */
        updateCommMatrix(msg_size, rank, rank-1);
      else{    /* even */
      }
    }
}
// Method Completed => Verified
void profile_Allreduce_intra_reduce_scatter_allgather(int count, int type_size, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  int mask, dst, pof2, newrank, rem, newdst, i,
    send_idx, recv_idx, last_idx, send_cnt, recv_cnt, *cnts;
  // int step=0;
  long msg_size;
  pof2 = (int)pow(2, (int)log2(comm_size));
  rem = comm_size - pof2;
  cnts = (int *)malloc(pof2*sizeof(int));
  int diff, step, prev_steps;
  msg_size = count*type_size;
  std::cout<<count<<" "<<type_size<<"\n";
  if (rank < 2 * rem) {
      if (rank % 2 == 0) {    /* even */
          // printf("rank: %d, dst: %d\n", rank, rank+1);
          diff=1;
          step=(int)log2(diff);
          std::vector<long> tmp={total_calls, step, rank, rank+1, msg_size};
          temp_steps.push_back(tmp);
          updateCommMatrix(msg_size, rank, rank+1);
          newrank = -1;
      } else {        /* odd */
        newrank = rank / 2;
      }
  } else      /* rank >= 2*rem */
    newrank = rank - rem;
  
  if (newrank != -1) {
        for (i = 0; i < pof2; i++)
            cnts[i] = count / pof2;
        if ((count % pof2) > 0) {
            for (i = 0; i < (count % pof2); i++)
                cnts[i] += 1;
        }
        mask = 0x1;
        send_idx = recv_idx = 0;
        last_idx = pof2;
        while (mask < pof2) {
            newdst = newrank ^ mask;
            /* find real rank of dest */
            dst = (newdst < rem) ? newdst * 2 + 1 : newdst + rem;

            send_cnt = recv_cnt = 0;
            if (newrank < newdst) {
                send_idx = recv_idx + pof2 / (mask * 2);
                for (i = send_idx; i < last_idx; i++)
                    send_cnt += cnts[i];
                for (i = recv_idx; i < send_idx; i++)
                    recv_cnt += cnts[i];
            } else {
                recv_idx = send_idx + pof2 / (mask * 2);
                for (i = send_idx; i < recv_idx; i++)
                    send_cnt += cnts[i];
                for (i = recv_idx; i < last_idx; i++)
                    recv_cnt += cnts[i];
            }
            // printf("rank: %d dst: %d\n", rank, dst);
            diff=abs(dst-rank);
            step=(int)log2(diff);
            std::vector<long> tmp={total_calls, step, rank, dst, send_cnt*type_size};    
            temp_steps.push_back(tmp);
            updateCommMatrix(send_cnt*type_size, rank, dst);
            /* update send_idx for next iteration */
            send_idx = recv_idx;
            mask <<= 1;

            /* update last_idx, but not in last iteration
             * because the value is needed in the allgather
             * step below. */
            if (mask < pof2)
                last_idx = recv_idx + pof2 / mask;
        }

        /* now do the allgather */
        prev_steps=(int)log2(comm_size);

        mask >>= 1;
        while (mask > 0) {
            newdst = newrank ^ mask;
            /* find real rank of dest */
            dst = (newdst < rem) ? newdst * 2 + 1 : newdst + rem;

            send_cnt = recv_cnt = 0;
            if (newrank < newdst) {
                /* update last_idx except on first iteration */
                if (mask != pof2 / 2)
                    last_idx = last_idx + pof2 / (mask * 2);

                recv_idx = send_idx + pof2 / (mask * 2);
                for (i = send_idx; i < recv_idx; i++)
                    send_cnt += cnts[i];
                for (i = recv_idx; i < last_idx; i++)
                    recv_cnt += cnts[i];
            } else {
                recv_idx = send_idx - pof2 / (mask * 2);
                for (i = send_idx; i < last_idx; i++)
                    send_cnt += cnts[i];
                for (i = recv_idx; i < send_idx; i++)
                    recv_cnt += cnts[i];
            }
            // printf("rank: %d, dst: %d\n", rank, dst);
            diff=abs(rank-dst); 
            step=(int)log2(diff);
            step+=prev_steps;
            std::vector<long> tmp={total_calls, step, rank, dst, send_cnt*type_size};
            temp_steps.push_back(tmp);
            updateCommMatrix(send_cnt*type_size, rank, dst);
            
            if (newrank > newdst)
                send_idx = recv_idx;

            mask >>= 1;
        }
    }
    /* In the non-power-of-two case, all odd-numbered
     * processes of rank < 2*rem send the result to
     * (rank-1), the ranks who didn't participate above. */
    if (rank < 2 * rem) {
        if (rank % 2)   /* odd */
          updateCommMatrix(msg_size, rank, rank-1);
        else{    /* even */
        }
    }
}

/*Problem with non-power of two comm_size values*/
void profile_Scatter_binomial(int root, int sendtype_size, int sendcount, int recvtype_size, int recvcount, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  int relative_rank, nbytes, mask, src, dst, curr_cnt = 0, send_subtree_cnt, tmp_buf_size;
  relative_rank = (rank >= root) ? rank - root : rank - root + comm_size;
  nbytes = (rank == root) ? sendtype_size * sendcount : recvtype_size * recvcount;
  if (relative_rank && !(relative_rank % 2)) {
    tmp_buf_size = (nbytes * comm_size) / 2;
  }
  // Setting the curr_cnt for root process
  if (rank == root) {
    if (root != 0) {
      tmp_buf_size = nbytes * comm_size;
      curr_cnt = nbytes * comm_size;
    } else
      curr_cnt = sendcount * comm_size;
  }
  // Recv
  mask = 0x1;
  while (mask < comm_size) {
      if (relative_rank & mask) {
          src = rank - mask;
          if (src < 0)
              src += comm_size;
          if(relative_rank % 2)
            curr_cnt = recvcount;
          else {
            curr_cnt = nbytes * mask;
          }
          break;
      }
      mask <<= 1;
  }
  // Send
  mask >>= 1;
  int step, diff;
  while (mask > 0) {
      if (relative_rank + mask < comm_size) {
          dst = rank + mask;
          if (dst >= comm_size)
              dst -= comm_size;
          int relative_dst = (dst >= root) ? dst - root : dst - root + comm_size;
          if ((rank == root) && (root == 0)) {
              send_subtree_cnt = curr_cnt - sendcount * mask;
              // printf("rank: %d, dst: %d\n", rank, dst);
              diff=abs(relative_dst-relative_rank);
              step=(int)log2(diff);
              std::vector<long> tmp={total_calls, step, rank, dst, send_subtree_cnt*sendtype_size};
              temp_steps.push_back(tmp);
              updateCommMatrix(send_subtree_cnt*sendtype_size, rank, dst);
              /* mask is also the size of this process's subtree */
          } else {
              /* non-zero root and others */
              send_subtree_cnt = curr_cnt - nbytes * mask;
              /* mask is also the size of this process's subtree */
              diff=abs(relative_dst-relative_rank);
              step=(int)log2(diff);
              std::vector<long> tmp={total_calls, step, rank, dst, send_subtree_cnt*sendtype_size};
              temp_steps.push_back(tmp);
              // printf("rank: %d, dst: %d\n", rank, dst);
              updateCommMatrix(send_subtree_cnt*1, rank, dst);
          }
          curr_cnt -= send_subtree_cnt;
      }
      mask >>= 1;
  }
}
// Method Completed => Verified
void profile_Gather_binomial(int root, MPI_Datatype sendtype, int sendcount, MPI_Datatype recvtype, int recvcount, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  int relative_rank, nbytes, curr_cnt, mask, src, recvblks, copy_offset=0, copy_blks = 0, relative_src, dst, missing;
  MPI_Aint tmp_buf_size;
  int sendtype_size, recvtype_size;
  PMPI_Type_size(sendtype, &sendtype_size); /* Compute size */
  PMPI_Type_size(recvtype, &recvtype_size); /* Compute size */
  int blocks[2], displs[2];
  MPI_Datatype types[2], tmp_type;


  relative_rank = (rank >= root) ? rank - root : rank - root + comm_size;
  nbytes = (rank == root) ? recvtype_size * recvcount : sendtype_size * sendcount;
  curr_cnt = nbytes;
  
  /* Find the number of missing nodes in my sub-tree compared to
    * a balanced tree */
  for (mask = 1; mask < comm_size; mask <<= 1);
  --mask;
  while (relative_rank & mask)
      mask >>= 1;
  missing = (relative_rank | mask) - comm_size + 1;
  if (missing < 0)
      missing = 0;
  tmp_buf_size = (mask - missing);

  /* If the message is smaller than the threshold, we will copy
    * our message in there too */
  if (nbytes < MPIR_CVAR_GATHER_VSMALL_MSG_SIZE)
      tmp_buf_size++;

  tmp_buf_size *= nbytes;

  /* For zero-ranked root, we don't need any temporary buffer */
  if ((rank == root) && (!root || (nbytes >= MPIR_CVAR_GATHER_VSMALL_MSG_SIZE)))
      tmp_buf_size = 0;

  int step, diff;
  mask = 0x1;
  while (mask < comm_size) {
        if ((mask & relative_rank) == 0) {
            src = relative_rank | mask;
            if (src < comm_size) {
                src = (src + root) % comm_size;

                if (rank == root) {
                    recvblks = mask;
                    if ((2 * recvblks) > comm_size)
                        recvblks = comm_size - recvblks;

                    if ((rank + mask + recvblks == comm_size) ||
                        (((rank + mask) % comm_size) < ((rank + mask + recvblks) % comm_size))) {
                        
                    } else if (nbytes < MPIR_CVAR_GATHER_VSMALL_MSG_SIZE) {
                        // copy_offset = rank + mask;
                        // copy_blks = recvblks;
                    } else {
                        blocks[0] = recvcount * (comm_size - root - mask);
                        // displs[0] = recvcount * (root + mask);
                        blocks[1] = (recvcount * recvblks) - blocks[0];
                    }
                } else {        /* Intermediate nodes store in temporary buffer */

                    MPI_Aint offset;

                    /* Estimate the amount of data that is going to come in */
                    recvblks = mask;
                    relative_src = ((src - root) < 0) ? (src - root + comm_size) : (src - root);
                    if (relative_src + mask > comm_size)
                        recvblks -= (relative_src + mask - comm_size);

                    // if (nbytes < MPIR_CVAR_GATHER_VSMALL_MSG_SIZE)
                    //     offset = mask * nbytes;
                    // else
                    //     offset = (mask - 1) * nbytes;
                    curr_cnt += (recvblks * nbytes);
                }
            }
        }
        else {
            dst = relative_rank ^ mask;
            dst = (dst + root) % comm_size;

            int relative_dst=(dst>=root)?(dst-root):(dst-root+comm_size);
            diff=abs(relative_dst-relative_rank);
            step=(int)log2(diff);

            if (!tmp_buf_size) {
                /* leaf nodes send directly from sendbuf */
                // printf("rank: %d, dst: %d\n", rank, dst);
                std::vector<long> tmp={total_calls, step, rank, dst, sendcount*sendtype_size};
                temp_steps.push_back(tmp);
                updateCommMatrix(sendcount*sendtype_size ,rank, dst);
                
            } else if (nbytes < MPIR_CVAR_GATHER_VSMALL_MSG_SIZE) {
              // printf("rank: %d, dst: %d\n", rank, dst);
              std::vector<long> tmp={total_calls, step, rank, dst, curr_cnt*1};
              temp_steps.push_back(tmp);
              updateCommMatrix(curr_cnt*1, rank, dst);
               
            } else {
                blocks[0] = sendcount;
                long msg_size = sendcount*sendtype_size;
                // currcnt-nbytes can lead to overflow - So remember this
                msg_size += (curr_cnt-nbytes);
                // printf("rank: %d, dst: %d\n", rank, dst);
                std::vector<long> tmp={total_calls, step, rank, dst, msg_size};
                temp_steps.push_back(tmp);
                updateCommMatrix(msg_size, rank, dst);
            }

            break;
        }
        mask <<= 1;
    }
}

/*************Start Profiling Reduce************/
// Method Completed => Verified
void profile_Reduce_intra_reduce_scatter_gather(MPI_Datatype datatype, int root, int count, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  int rem, pof2, type_size, newrank, i, mask, send_idx, recv_idx, last_idx, newdst, dst, send_cnt, recv_cnt,
  newroot,j, newdst_tree_root, newroot_tree_root;
  PMPI_Type_size(datatype, &type_size); /* Compute size */
  pof2 = (int)pow(2, (int)log2(comm_size));
  rem = comm_size - pof2;
  // int step=0;
  int *cnts = (int *)malloc(pof2*sizeof(int));
  int step, diff, prev_steps;
    if (rank < 2 * rem) {
        if (rank % 2 != 0) {    /* odd */
            // printf("rank: %d, dst: %d\n", rank, rank-1);
            diff=1;
            step=(int)log2(diff);
            std::vector<long> tmp={total_calls, step, rank, rank-1, count*type_size};
            temp_steps.push_back(tmp);
            updateCommMatrix(count*type_size, rank, rank-1);
            newrank = -1;
        } else {        /* even */
            /* do the reduction on received data. */
            /* This algorithm is used only for predefined ops
             * and predefined ops are always commutative. */
            /* change the rank */
            newrank = rank / 2;
        }
    } else      /* rank >= 2*rem */
        newrank = rank - rem;

    /* for the reduce-scatter, calculate the count that
     * each process receives and the displacement within
     * the buffer */

    /* We allocate these arrays on all processes, even if newrank=-1,
     * because if root is one of the excluded processes, we will
     * need them on the root later on below. */
    
    if (newrank != -1) {
        for (i = 0; i < pof2; i++)
            cnts[i] = count / pof2;
        if ((count % pof2) > 0) {
            for (i = 0; i < (count % pof2); i++)
                cnts[i] += 1;
        }
        mask = 0x1;
        send_idx = recv_idx = 0;
        last_idx = pof2;
        while (mask < pof2) {
            newdst = newrank ^ mask;
            /* find real rank of dest */
            dst = (newdst < rem) ? newdst * 2 : newdst + rem;

            send_cnt = recv_cnt = 0;
            if (newrank < newdst) {
                send_idx = recv_idx + pof2 / (mask * 2);
                for (i = send_idx; i < last_idx; i++)
                    send_cnt += cnts[i];
                for (i = recv_idx; i < send_idx; i++)
                    recv_cnt += cnts[i];
            } else {
                recv_idx = send_idx + pof2 / (mask * 2);
                for (i = send_idx; i < recv_idx; i++)
                    send_cnt += cnts[i];
                for (i = recv_idx; i < last_idx; i++)
                    recv_cnt += cnts[i];
            }
            // printf("rank: %d, dst: %d\n", rank, dst);
            diff=abs(rank-dst);
            step=(int)log2(diff);
            std::vector<long> tmp={total_calls, step, rank, dst, send_cnt*type_size};
            temp_steps.push_back(tmp);
            updateCommMatrix(send_cnt*type_size, rank, dst);
            /* Send data from recvbuf. Recv into tmp_buf */
            
            /* update send_idx for next iteration */
            send_idx = recv_idx;
            mask <<= 1;

            /* update last_idx, but not in last iteration
             * because the value is needed in the gather
             * step below. */
            if (mask < pof2)
                last_idx = recv_idx + pof2 / mask;
        }
    }
    prev_steps=(int)log2(comm_size);
    /* now do the gather to root */

    /* Is root one of the processes that was excluded from the
     * computation above? If so, send data from newrank=0 to
     * the root and have root take on the role of newrank = 0 */
    int rel_rank=(rank>=root)?(rank-root):(rank-root+comm_size);
    if (root < 2 * rem) {
        if (root % 2 != 0) {
            if (rank == root) { /* recv */
                /* initialize the arrays that weren't initialized */
                for (i = 0; i < pof2; i++)
                    cnts[i] = count / pof2;
                if ((count % pof2) > 0) {
                    for (i = 0; i < (count % pof2); i++)
                        cnts[i] += 1;
                }
                newrank = 0;
                send_idx = 0;
                last_idx = 2;
            } else if (newrank == 0) {  /* send */
                // printf("rank: %d, root: %d\n", rank, root);
                diff=abs(rel_rank-0);
                step=(int)log2(diff);
                step+=prev_steps;
                std::vector<long> tmp={total_calls, step, rank, root, cnts[0]*type_size};
                temp_steps.push_back(tmp);
                updateCommMatrix(cnts[0]*type_size, rank, root);
                newrank = -1;
            }
            newroot = 0;
        } else
            newroot = root / 2;
    } else
        newroot = root - rem;

    if (newrank != -1) {
        j = 0;
        mask = 0x1;
        while (mask < pof2) {
            mask <<= 1;
            j++;
        }
        mask >>= 1;
        j--;
        while (mask > 0) {
            newdst = newrank ^ mask;

            /* find real rank of dest */
            dst = (newdst < rem) ? newdst * 2 : newdst + rem;
            /* if root is playing the role of newdst=0, adjust for
             * it */
            if ((newdst == 0) && (root < 2 * rem) && (root % 2 != 0))
                dst = root;

            /* if the root of newdst's half of the tree is the
             * same as the root of newroot's half of the tree, send to
             * newdst and exit, else receive from newdst. */

            newdst_tree_root = newdst >> j;
            newdst_tree_root <<= j;

            newroot_tree_root = newroot >> j;
            newroot_tree_root <<= j;

            send_cnt = recv_cnt = 0;
            if (newrank < newdst) {
                /* update last_idx except on first iteration */
                if (mask != pof2 / 2)
                    last_idx = last_idx + pof2 / (mask * 2);

                recv_idx = send_idx + pof2 / (mask * 2);
                for (i = send_idx; i < recv_idx; i++)
                    send_cnt += cnts[i];
                for (i = recv_idx; i < last_idx; i++)
                    recv_cnt += cnts[i];
            } else {
                recv_idx = send_idx - pof2 / (mask * 2);
                for (i = send_idx; i < last_idx; i++)
                    send_cnt += cnts[i];
                for (i = recv_idx; i < send_idx; i++)
                    recv_cnt += cnts[i];
            }

            if (newdst_tree_root == newroot_tree_root) {
              // std::cout << "Sender: " << rank << " Receiver: " << dst << " Count: " << send_cnt << "\n"; 
                // printf("rank: %d, dst: %d\n", rank, dst);
                int rel_dst=(dst>=root)?(dst-root):(dst-root+comm_size);
                diff=abs(rel_rank-rel_dst);
                step=(int)log2(diff);
                step+=prev_steps;
                std::vector<long> tmp={total_calls, step, rank, dst, send_cnt*type_size};
                temp_steps.push_back(tmp);
                updateCommMatrix(send_cnt*type_size, rank, dst);
                break;
            } else {}
            if (newrank > newdst)
                send_idx = recv_idx;
            mask >>= 1;
            j--;
        }
    }
}

/*is_commutative = MPIR_Op_is_commutative(op); - this line is a problem - find the definition of the specified line*/
// Method Completed => Verified
void profile_Reduce_intra_binomial(MPI_Datatype datatype, int root, int count, MPI_Op op, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  int mask, lroot, is_commutative, relrank, type_size, source;
  PMPI_Type_size(datatype, &type_size); /* Compute size */
  // is_commutative = MPIR_Op_is_commutative(op);
  is_commutative = 1;
  mask = 0x1;
  if (is_commutative)
      lroot = root;
  else
      lroot = 0;
  relrank = (rank - lroot + comm_size) % comm_size;

  while (/*(mask & relrank) == 0 && */ mask < comm_size) {
      /* Receive */
      if ((mask & relrank) == 0) {
          source = (relrank | mask);
          if (source < comm_size) {
              source = (source + lroot) % comm_size;
              /* The sender is above us, so the received buffer must be
                * the second argument (in the noncommutative case). */
          }
      } else {
          /* I've received all that I'm going to.  Send my result to
            * my parent */
          source = ((relrank & (~mask)) + lroot) % comm_size;
          // printf("rank: %d, source: %d\n", rank, source);
          int relative_dst=(source>=root)?(source-root):(source-root+comm_size);
          int diff=abs(relative_dst-relrank);
          int step=(int)log2(diff);
          std::vector<long> tmp={total_calls, step, rank, source, count*type_size};
          temp_steps.push_back(tmp);
          updateCommMatrix(count*type_size, rank, source);
          break;
      }
      mask <<= 1;
  }

  if (!is_commutative && (root != 0)) {
      if (rank == 0) {
        updateCommMatrix(count*type_size, rank, root);
      } else if (rank == root) {
          
      }
      
  }
}
/***************End Profiling Reduce************/

void profile_Barrier() {
  int mask, src, dst;
  mask = 0x1;
    while (mask < comm_size) {
        dst = (rank + mask) % comm_size;
        src = (rank - mask + comm_size) % comm_size;
        updateCommMatrix(0, rank, dst);
        mask <<= 1;
    }
}
void profile_Alltoall_intra_pairwise_sendrecv_replace(MPI_Datatype sendtype, MPI_Datatype recvtype, int sendcount, int recvcount, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  int i, j, sendtype_size, recvtype_size;
  int step=0;
  PMPI_Type_size(sendtype, &sendtype_size); /* Compute size */
  PMPI_Type_size(recvtype, &recvtype_size); /* Compute size */
  for (i = 0; i < comm_size; ++i) {
    /* start inner loop at i to avoid re-exchanging data */
    for (j = i; j < comm_size; ++j) {
      if (rank == i) {
        /* also covers the (rank == i && rank == j) case */
        // printf("rank: %d, dst: %d\n", rank, j);
        updateCommMatrix(recvcount*recvtype_size, rank, j);
      } else if (rank == j) {
        /* same as above with i/j args reversed */
        // printf("rank: %d, dst: %d\n", rank, i);
        updateCommMatrix(recvcount*recvtype_size, rank, i);
      }
    }
  }
}
void profile_Alltoall_intra_brucks(MPI_Datatype sendtype, MPI_Datatype recvtype, int sendcount, int recvcount, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  /*New type is being used*/
}
void profile_Alltoall_intra_scattered(MPI_Datatype sendtype, MPI_Datatype recvtype, int sendcount, int recvcount, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  int i, ii, sendtype_size, recvtype_size, bblock, ss, dst;
  int step=0;
  PMPI_Type_size(sendtype, &sendtype_size); /* Compute size */
  PMPI_Type_size(recvtype, &recvtype_size); /* Compute size */
  bblock = MPIR_CVAR_ALLTOALL_THROTTLE;
  if (bblock == 0)
      bblock = comm_size;
  for (ii = 0; ii < comm_size; ii += bblock) {
        ss = comm_size - ii < bblock ? comm_size - ii : bblock;
        /* do the communication -- post ss sends and receives: */
        for (i = 0; i < ss; i++) {
            dst = (rank + i + ii) % comm_size;
            // mpi_errno = MPIC_Irecv((char *) recvbuf +
            //                        dst * recvcount * recvtype_extent,
            //                        recvcount, recvtype, dst,
            //                        MPIR_ALLTOALL_TAG, comm_ptr, &reqarray[i]);
        }

        for (i = 0; i < ss; i++) {
            dst = (rank - i - ii + comm_size) % comm_size;
            updateCommMatrix(sendcount*sendtype_size, rank, dst);
            // printf("rank: %d, dst: %d\n", rank, dst);
            std::vector<long> tmp={total_calls, i, rank, dst, sendcount*sendtype_size};
            temp_steps.push_back(tmp);
            // mpi_errno = MPIC_Isend((char *) sendbuf +
            //                        dst * sendcount * sendtype_extent,
            //                        sendcount, sendtype, dst,
            //                        MPIR_ALLTOALL_TAG, comm_ptr, &reqarray[i + ss], errflag);
        }
    }

}
void profile_Alltoall_intra_pairwise(MPI_Datatype sendtype, MPI_Datatype recvtype, int sendcount, int recvcount, std::vector<std::vector<long>> &temp_steps, long &total_calls) {
  int i, sendtype_size, recvtype_size, src, dst, pof2;
  PMPI_Type_size(sendtype, &sendtype_size); /* Compute size */
  PMPI_Type_size(recvtype, &recvtype_size); /* Compute size */
  pof2 = !(comm_size & (comm_size-1));
  /* Do the pairwise exchanges */
  for (i = 1; i < comm_size; i++) {
      if (pof2 == 1) {
          /* use exclusive-or algorithm */
          src = dst = rank ^ i;
      } else {
          src = (rank - i + comm_size) % comm_size;
          dst = (rank + i) % comm_size;
      }
      std::vector<long> tmp={total_calls, i-1, rank, dst, sendcount*sendtype_size};
      temp_steps.push_back(tmp);
      updateCommMatrix(sendcount*sendtype_size, rank, dst);     
  }
}
