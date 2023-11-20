- `text_2.sh` generates and shuffles lists of source/destination/message size. Lists with 8 messages each may be generated and used multiple times.
- I used `sizestime` for the sizes file field
- `run_text.sh` and `run_opposite.sh` are used to send lists of messages generated. `run_silent.sh` may be used when the script is being run over `ssh`, to minimise the noise generated by its network packets.
- The `cat $1` in these may be replaced by `cat $1 $1 $1 ...`
- `pkill -u prsingh` is so that the user gets logged out upon termination of the script. Replace `prsingh` with your username.

- `median.py` in this directory is different from `../median.py`. The CSV format for this script is `source,destination,size,time`.
- Only the median-containing files have been saved to the repository as the entire data takes up 3 MB. I did not do away with the redundant `csews`s in each line as some nodes are named `csewXX` and there are teething issues