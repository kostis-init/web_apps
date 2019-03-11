#ifndef DEFS3_H
#define DEFS3_H

#define MAX_THREADS 50
#define POOL_SIZE   100

///typedefs
typedef enum { false, true } bool;
typedef struct {
        int data[POOL_SIZE];
        int start;
        int end;
        int count;
} pool_t;

///global variables
extern int     s_port;//serving port
extern int     c_port;//command port
extern char*      dir;//root_dir
extern int   nthreads;//num_of_threads
extern int       quit;//flag to quit

extern pthread_t *ths;//array of threads
extern pthread_mutex_t mtx;
extern pthread_cond_t cond_nonempty;
extern pthread_cond_t cond_nonfull;
extern pool_t pool;

///functions

//args.c
void parse_args(int,char**);

//help.c
void initialize_pool(void);
int file_exists (char*);
int file_perm(char*);
int is_dir(char*);
int digits(int);
int count_lines(char*);
void perror_exit(char*);

//server.c
void server(void);

//threads.c
void threads(void);

//net.c
long unsigned int response_ok(int socket,char *file);
void response_ne(int socket);
void response_np(int socket);



#endif
