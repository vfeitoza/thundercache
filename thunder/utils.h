#ifndef UTILS_H
#define UTILS_H

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <vector>

using namespace std;

struct resposta {
        bool match;
        string domain;
        string file;
};

static const std::string base64_chars = 
			"nopq?rst@#u789+RST&UyzMNOPQhi"
             "abcdefgvwx$FGHIJKL"
             "012!34jklmA*BCDEVWXYZ56/";

string UpperCase( string CaseString );
void SearchReplace( string &source, string search, string replace );
int select_eintr( int fds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout );
bool MatchSubstr(string &hay, const char* needle, int startpos);
bool MatchBegin(string &hay, const char *needle, int needlelength);
void stringexplode(string str, string separator, vector<string>* results);
string getdomain(string url);
bool file_exists(string strFilename);
int64_t file_size( string szFileName );
void mkdir_p(const string &pathname);
string getfilepath(string path);
string getfilename(string path);
string regex_match(string er, string line);
string itoa(int val);
double now();
long file_getmodif( string szFileName );
int file_setmodif( string szFileName,long fdate =0); 
string url2host(string &url);
string url2request(string &url);
string ConvertChar(string lineT);
int disk_use(string path);
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
std::string XOR(string value,string key);
const string getFileExtension(string file);
const string getFileName(string file);
const string sqlconv(string sql);

#endif
