- The user's home directory should have `.ipmpi` for the scripts in this directory to run
- `ipmpi.txt` contains a brief summary of functions and global variables in the modified version of IPMPI (before I made changes).
- `summary.txt` summarises the observations from IPMPI profiles on the choice of algorithms (may not be fully accurate)
- `compare.sh` was used for sanity checks on IPMPI. Remember to modify the paths in it, and uncomment the `fprintf (stderr, * );` parts of `time_collective_from_steps()` and `time_collective_from_steps_realtime ()` to use it.
- `gather-data.sh` generates CSVs with rows in the format \<message size\>,\<Model 1 prediction\>,\<Model 2 prediction\>,actual1,actual2,.. . The final argument is redundant - on the CSEWS cluster, changes to the `temp` folder did not show up fast, and `read_profiles.py` was using old profiles. So `gather-data.sh` was modified to create a new folder for each profile.
- `log_alltoall.sh` is a minsnomer - change `coll` in line 7 to `b`, `g` or `t` to run `MPI_Bcast ()`, `MPI_Allgather ()` or `MPI_Alltoall` ()`. (Or better, add another command-line argument.) It is executed by `gather-data.sh`, which is the top-level script to be run by the user.
- `sedd.sh` uses GNU `sed` to present the output of `gather-data.sh` as a TSV. Going through the numbers is easier that way.
- `plot-all-3.py` plots predictions of both models and actual execution times for 5 - 10 messages (modify `gather-data.sh` to change this number).