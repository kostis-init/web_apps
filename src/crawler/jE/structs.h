#ifndef STRUCTS_H
#define STRUCTS_H

///POSTINGS LIST

struct PosNode
{
	char* pathname;
	int* lines;//array of lines that have this word
	int lsz;//size of lines array
	struct PosNode* next;
};

struct PosList
{
	int wordfreq;
	struct PosNode* first;
};

///TRIE
struct TrieNode
{
	char c;
	struct TrieNode* child;
	struct TrieNode* next;
	struct PosList*  pos;
};

struct TrieRoot
{
	struct TrieNode* first;
};

///FILELIST
struct filelist //list that holds filenames and their data
{
	char** map;//in lines (i.e. map[3] contains the 4th line)
	char* pathname;//filename
	int nlines;//number of lines
	struct filelist* next;
};


#endif
