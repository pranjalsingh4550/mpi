- `run.sh` is an older script, `../ipmpi-analysis` was used instead.
- `realtime.cpp` checks network health and generates a file for each node. Number of processes should be a multiple of 8. Run `$ mpic++ -o realtime realtime.cpp` to compile it.
- `noise.sh` runs `a.out` which generates contention. Usage: `$ ./noise.sh <hosts file, preferably 8>`
- `combine.py` finds medians from the output of `realtime.cpp`.

