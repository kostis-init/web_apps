#ifndef DEFS_H
#define DEFS_H

#define DEFAULT_NUMWOR 3	//default number of workers (if not given explicitly)
#define MAXQ	20	//max number of keywords in query
#define MAXBUFF 100000
#define FIFO1	"tmp/f.1"
#define FIFO2	"tmp/f.2"
#define FLEN	7 //length of fifo strings
#define PERMS	0666

//dirs.c
void parse_args(int argc,char** argv,char*** dirs,int* numWor);//parse starting arguments (docfile, numWor)
void parse_file(FILE* docfile,char** map);//save file in memory

//executor.c
void start(char** dirs,int numWor);//start executor
void replacew();//replace dead child

//worker.c
void worker(char* file1,char* file2);//start worker
void clear_fl(struct filelist* fl);

//help.c
int digits(int n);//get number of digits of n
int fcount_lines(FILE* file);//count lines of file
int count_lines(char* buf);//count lines of buf
int exists(char* buf,char* name,int line);
long long timeInMilliseconds(void);
int allset(char* flags,int numWor);

//trie.c
int create_index(struct filelist* fl,struct TrieRoot* trie);//save words from list in the trie
int TrieInit(struct TrieRoot* trie);
int TrieNodeInit(struct TrieNode** node);
int TrieInsert(struct TrieRoot* trie,char* word,int line,char* filename);//insert word into trie
void TrieDelete(struct TrieRoot);
void TriePrintAllDF(struct TrieNode* node,char* word,int depth);

//ui.c
int user_input(int* fdw,int* fdr);

//pos.c
void PosDelete(struct PosList* pos);
int PosInsert(struct PosList* pos,int line,char* pathname);

#endif
