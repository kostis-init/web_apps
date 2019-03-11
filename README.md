# web_apps
3 cooperating apps. Web creator, web server, web crawler (Using threads)

webcreator.sh: Only absolute paths for root_dir. Do not put ‘/’ in the end of root_dir.Example:
$ cd ./src
$ ./webcreator.sh /……/src/websites textfile 3 3

server: Example:
$ cd ./src; make
$ ./myhttpd -p 8080 -c 9090 -t 2 -d /……/src/websites

crawler: Do not put ‘/’ in the end of save_dir. Put ‘/’ in the beginning of starting url (relative to roor_dir). Example:
$ cd ./src/crawler; make;
$ ./mycrawler -h localhost -p 8080 -c 9999 -t 4 -d save_dir /site0/page0_715.html


This was an assignment for the Systems Programming class of DIT of UoA (2018)
