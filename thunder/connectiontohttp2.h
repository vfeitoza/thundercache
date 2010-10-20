#ifndef CONNECTIONTOHTTP2_H
#define CONNECTIONTOHTTP2_H

#include "connectiontohttp.h"
#include "utils.h"
#include "database_mysql.h"
#include "fstream"

using namespace std;

class ConnectionToHTTP2 : public ConnectionToHTTP {
    private:
        string pluginsdir, cachedir, subdir, completefilepath, completepath;
		int cache_limit;
        fstream cachefile;
        fstream outfile;
        double timerecord,timerecord2,timeout;
        ConnectionToHTTP downloader;
        Database domaindb;
        bool passouheader;
    public:
        string domain,request,msghit;
		int port;
        resposta r;
        int64_t filesize,filedownloaded,filesended,partial,expiration;
        bool hit,downloading,rewrited,resuming,general,etag;
        void Cache();
        bool SetDomainAndPort( string domainT, int portT, string requestT="" );
        bool ConnectToServer();
        bool SendHeader( string header, bool ConnectionClose, string requestT="" );
        string GetIP();
        bool ReadHeader(string &headerT);
        bool AnalyseHeader( string &linesT );
        bool IsItKeepAlive();
        int64_t GetContentLength();
        bool IsItChunked();
        string PrepareHeaderForBrowser();
        int GetResponse();
        bool CheckForData( int timeout );
        ssize_t ReadBodyPart( string &bodyT, bool Chunked );
        void Close();
};

#endif

