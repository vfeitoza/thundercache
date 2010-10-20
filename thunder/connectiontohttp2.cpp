#include "connectiontohttp2.h"
#include "logfile.h"
#include "utils.h"
#include "params.h"
#include "database_mysql.h"
#include "x64compat.h"

#include <stdio.h>
#include <iostream>
#include <vector>
#include <dlfcn.h>
#include <cstdlib>
#include <sstream>


extern int LL; //LogLevel

void ConnectionToHTTP2::Cache() {
    msghit = "MISS";
    hit = downloading = r.match = rewrited = resuming = passouheader = general = etag = false;
    filesize = filedownloaded = filesended = partial = expiration = 0;
    if (LL > 0) LogFile::AccessMessage("Url %s%s\n", domain.c_str(), request.c_str());
    string domaintmp = getdomain(domain + request);
    string pluginpath = pluginsdir + domaintmp + ".so";

    if (file_exists(pluginpath)) {
        if (LL > 0) LogFile::AccessMessage("Loading plugin %s\n", pluginpath.c_str());
        void* handle = dlopen(pluginpath.c_str(), RTLD_LAZY);
        if (!handle) {
            if (LL > 0) LogFile::ErrorMessage("Cannot open library: %s\n", dlerror());
        }
        typedef resposta(*plugin_t)(string);
        plugin_t plugin = (plugin_t) dlsym(handle, "getmatch");
        if (!plugin) {
            if (LL > 0) LogFile::ErrorMessage("Cannot load symbol: %s\n", dlerror());
            dlclose(handle);
        } else { // Plugin carregado corretamente, ok para atribuir valores ao struc
            r = plugin(domain + request);
            if (LL > 0) LogFile::AccessMessage("Resposta Match %d Domain %s File %s\n", r.match, r.domain.c_str(), r.file.c_str());
        }
    }

    if (r.match) {
        subdir = ConvertChar(r.file);
        completepath = cachedir + r.domain;
        completefilepath = completepath + "/" + subdir + "/" + r.file;
        if (LL > 1) LogFile::AccessMessage("Arquivo: %s\n", string(completefilepath).c_str());
        if (r.domain == "rewrite") {
            r.match = false;
            rewrited = true;
            if (LL > 1) LogFile::AccessMessage("Rewrite: %s%s \n", r.file.c_str());
        } else if (!file_exists(completefilepath)) {
            if (disk_use(completepath) <= cache_limit) {
                // cria diretorios e etc
                if (!file_exists(completepath + "/" + subdir + "/")) {
                    mkdir_p(completepath + "/" + subdir + "/");
                }
                domaindb.set("INSERT INTO thunder (domain, file, size, modified, downloaded, requested, last_request) VALUES ('" + r.domain + "', '" + domaindb.sqlconv(r.file) + "', 0, '1980-01-01 00:00:00',now(),0,now());");
                if (LL > 0) LogFile::AccessMessage("MISS: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
            } else {
                LogFile::ErrorMessage("Cache limit (%d/%d) %s%s\n", cache_limit, disk_use(completepath), cachedir.c_str(), r.domain.c_str());
            }
        } else if (file_exists(completefilepath)) {
            if (domaindb.get("SELECT size FROM thunder WHERE file='" + domaindb.sqlconv(r.file) + "' and domain='" + r.domain + "';") != 0) {
                LogFile::ErrorMessage("erro select mysql: %s\n", domaindb.getError().c_str());
                r.match = hit = false;
            }
            filesize = atol(domaindb.get("size", 1).c_str());

            if (LL > 1) LogFile::AccessMessage("Size db: "LLD" File size: "LLD" Timeout: %d\n", filesize, file_size(completefilepath), (now() - file_getmodif(completefilepath)));

            if (domaindb.get_num_rows() == 0) // se nao existir o registro do arquivo do db
            {
                domaindb.set("INSERT INTO thunder (domain, file, size, modified, downloaded, requested, last_request) VALUES ('" + r.domain + "', '" + domaindb.sqlconv(r.file) + "', 0, '1980-01-01 00:00:00',now(),0,now());");
                if (LL > 0) LogFile::AccessMessage("MISS DB: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
            } else if (
                    ((now() - file_getmodif(completefilepath)) < 10)
                    &&
                    (
                    (filesize > file_size(completefilepath)) || (filesize == 0)
                    )
                    ) {
                if (Params::GetConfigBool(UpperCase(r.domain) + "_NODOWN")) {
					if (LL > 1) LogFile::AccessMessage("Concorrencia cancelada: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
					r.match = false; // pega direto do site
                } else {
                    hit = true;
                    downloading = true;
                    if (LL > 0) LogFile::AccessMessage("HIT DOWN: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
                    msghit = "HIT";
                }
            } else if (filesize == file_size(completefilepath)) {
                hit = true;
                downloading = false;
                domaindb.set("UPDATE thunder SET requested=requested+1, last_request=now() WHERE file='" + domaindb.sqlconv(r.file) + "' and domain='" + r.domain + "';");
                if (LL > 0) LogFile::AccessMessage("HIT!: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
                msghit = "HIT";
            } else if (Params::GetConfigBool(UpperCase(r.domain) + "_NORESUME") && filesize > 0) { // o arquivo foi baixado parcialmente
                if (disk_use(completepath) <= cache_limit) {
                    hit = resuming = downloading = true;
                    downloader.ClearVars();
                    downloader.SetDomainAndPort(domain, port);
                    if (LL > 0) LogFile::AccessMessage("HIT RESUME: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
                    msghit = "HIT";
                } else {
                    LogFile::ErrorMessage("Cache limit (%d/%d) %s%s\n", cache_limit, disk_use(completepath), cachedir.c_str(), r.domain.c_str());
                }
            }
            domaindb.clear();

        }
    } else if (!rewrited) { // sistema de cache de arquivos estáticos GENERAL
        if (Params::GetConfigInt(getFileExtension(getFileName(request)) + "_EXP") > 0 && request.find_first_not_of("()[]?&=;,´`'\"") != string::npos) { // se está configurada essa extensão
            r.file = getFileName(request);
            r.domain = domaintmp;
            subdir = request.substr(1, request.length() - r.file.length() - 2);
            completepath = cachedir + "general/" + r.domain.at(0) + "/" + ConvertChar(r.domain) + "/" + r.domain;
            completefilepath = completepath + "/" + subdir + "/" + r.file;
			if (string(subdir + "/" + r.file).length() <= 767) {
				r.match = general = true;
				//if (LL>1) LogFile::AccessMessage("Arquivo: %s\n",completefilepath.c_str());
				if (LL > 1) LogFile::AccessMessage("arquivo: %s subdir: %s completepath: %s\n", r.file.c_str(), subdir.c_str(), completepath.c_str());
				if (!file_exists(completefilepath)) {
					if (disk_use(completepath) <= cache_limit) {
						// cria diretorios e etc
						if (!file_exists(completepath + "/" + subdir + "/")) {
							mkdir_p(completepath + "/" + subdir + "/");
						}
						domaindb.set("INSERT INTO thunder (domain, file, size, modified, downloaded, requested, last_request, static) VALUES ('" + r.domain + "', '" + domaindb.sqlconv(subdir + "/" + r.file) + "', 0, '1980-01-01 00:00:00',now(),0,now(),1);");
						if (LL > 0) LogFile::AccessMessage("MISS: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
					} else {
						LogFile::ErrorMessage("Cache limit (%d/%d) %s%s\n", cache_limit, disk_use(completepath), cachedir.c_str(), r.domain.c_str());
					}
				} else if (file_exists(completefilepath)) {
					if (domaindb.get("SELECT size,(unix_timestamp(now())-unix_timestamp(downloaded)) as expiration FROM thunder WHERE file='" + domaindb.sqlconv(subdir + "/" + r.file) + "' and domain='" + r.domain + "';") != 0) {
						LogFile::ErrorMessage("erro select mysql: %s\n", domaindb.getError().c_str());
						r.match = general = false;
					}
					filesize = atol(domaindb.get("size", 1).c_str());
					expiration = atol(domaindb.get("expiration", 1).c_str());

					if (LL > 1) LogFile::AccessMessage("Size db: "LLD" File size: "LLD" Timeout: %d Expiration: %d\n", filesize, file_size(completefilepath), (now() - file_getmodif(completefilepath)), expiration);

					if (domaindb.get_num_rows() == 0) // se nao existir o registro do arquivo do db
					{
						domaindb.set("INSERT INTO thunder (domain, file, size, modified, downloaded, requested, last_request,static) VALUES ('" + r.domain + "', '" + domaindb.sqlconv(subdir + "/" + r.file) + "', 0, '1980-01-01 00:00:00',now(),0,now(),1);");
						if (LL > 0) LogFile::AccessMessage("MISS DB: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
					} else if (
							((now() - file_getmodif(completefilepath)) < 10)
							&&
							(
							(filesize > file_size(completefilepath)) || (filesize == 0)
							)
							) {
						hit = true;
						downloading = true;
						if (LL > 0) LogFile::AccessMessage("HIT DOWN: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
						msghit = "HIT";
					} else if (filesize == file_size(completefilepath)) {
						if (expiration <= Params::GetConfigInt(getFileExtension(getFileName(request)) + "_EXP")) {
							hit = true;
							downloading = false;
							domaindb.set("UPDATE thunder SET requested=requested+1, last_request=now() WHERE file='" + domaindb.sqlconv(subdir + "/" + r.file) + "' and domain='" + r.domain + "';");
							if (LL > 0) LogFile::AccessMessage("HIT!: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
							msghit = "HIT";
						} else {
							if (LL > 0) LogFile::AccessMessage("EXPIRED: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
							domaindb.set("UPDATE thunder SET downloaded=NOW() WHERE file='" + domaindb.sqlconv(subdir + "/" + r.file) + "' and domain='" + r.domain + "';");
						}
					}
					else if (filesize > 0) { // o arquivo foi baixado parcialmente
						if (disk_use(completepath) <= cache_limit) {
							hit = resuming = downloading = true;
							downloader.ClearVars();
							downloader.SetDomainAndPort(domain, port);
							if (LL > 0) LogFile::AccessMessage("HIT RESUME: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
							msghit = "HIT";
						} else {
							LogFile::ErrorMessage("Cache limit (%d/%d) %s%s\n", cache_limit, disk_use(completepath), cachedir.c_str(), r.domain.c_str());
						}
					}
					domaindb.clear();
				}
            }
        }
    }
}

bool ConnectionToHTTP2::SetDomainAndPort(string domainT, int portT, string requestT) {
    //if (LL>1) LogFile::AccessMessage("Passando pelo SetDomainAndPort \n");
    if (!domaindb.connected)
        if (domaindb.open(Params::GetConfigString("MYSQL_HOST"), Params::GetConfigString("MYSQL_USER"), Params::GetConfigString("MYSQL_PASS"), Params::GetConfigString("MYSQL_DB")) != 0) {
            LogFile::ErrorMessage("erro conexao mysql: %s\n", domaindb.getError().c_str());
            Close();
        }
    domain = domainT;
    request = requestT;
    port = portT;
    pluginsdir = Params::GetConfigString("PLUGINSDIR");
    cachedir = Params::GetConfigString("CACHEDIR");
    cache_limit = Params::GetConfigInt("CACHE_LIMIT");

    Cache();

    if (hit)
        return true;
    else
        return ConnectionToHTTP::SetDomainAndPort(domain, port);
}

bool ConnectionToHTTP2::ConnectToServer() {
    //if (LL>1) LogFile::AccessMessage("Passando pelo ConnectToServer \n");
    if (rewrited) return true;
    if (resuming) {
        if (!downloader.ConnectToServer()) {
            if (LL > 0) LogFile::AccessMessage("downloader nao conectou\n");
            Close();
        }
    }
    if (hit) return true;
    else return ConnectionToHTTP::ConnectToServer();
}

bool ConnectionToHTTP2::SendHeader(string header, bool ConnectionClose, string requestT) {
    //if (LL>1) LogFile::AccessMessage("Passando pelo SendHeader \n");
    if (passouheader) {
        if (cachefile.is_open()) cachefile.close();
        if (outfile.is_open()) outfile.close();
        if ((filesended < filesize) && ((r.match && !hit) || (r.match && resuming)))
            file_setmodif(completefilepath, 1);
        downloader.Close();

        if (LL > 1) LogFile::AccessMessage("Novo header, chamando Cache()\n");
        request = requestT;
        Cache();
    }
    if (!passouheader) passouheader = true;
    if (rewrited) return true;

    if (resuming) {
        vector<string> lines;
        char linha[5000];
        string headertmp = "";
        stringexplode(header, "\r\n", &lines);
        for (unsigned int i = 0; i <= lines.size() - 1; ++i) {
            if (i == lines.size() - 1) {
                sprintf(linha, "Range: bytes="LLD"-"LLD"\r\n", file_size(completefilepath), filesize);
            } else {
                sprintf(linha, "%s\r\n", lines.at(i).c_str());
            }
            headertmp.append(linha);
        }
        if (LL > 1) LogFile::AccessMessage("header req down (%s%s)\n %s \n", domain.c_str(), request.c_str(), headertmp.c_str());
        downloader.SendHeader(headertmp, true);
    }
    if (hit && (header.find("If-None-Match") != string::npos)) etag = true;
    if (hit) {
        if (!downloading && !resuming) {
            vector<string> lines;
            stringexplode(header, "\r\n", &lines);
            for (unsigned int i = 1; i <= lines.size() - 1; ++i) {
                if (lines.at(i).find("Range: bytes=") == 0) {
                    //pegar o valor do range, vou trabalhar só com o inicio do arquivo, foda-se, hehehe
                    partial = atoi(lines.at(i).substr(13, lines.at(1).length() - 13 - lines.at(i).find("-")).c_str());
                    break;
                }
            }

        }
        return true;
    } else return ConnectionToHTTP::SendHeader(header, ConnectionClose);
}

string ConnectionToHTTP2::GetIP() {
    //if (LL>1) LogFile::AccessMessage("Passando pelo GetIP \n");
    if (hit || rewrited) return "0.0.0.0";
    else return ConnectionToHTTP::GetIP();
}

bool ConnectionToHTTP2::ReadHeader(string &headerT) {
    //if (LL>1) LogFile::AccessMessage("Passando pelo ReadHeader \n");
    if (resuming) {
        string headertmp = "";
        if (downloader.GetResponse() == 0) downloader.ReadHeader(headertmp);
        downloader.AnalyseHeader(headertmp);
        if (LL > 1) LogFile::AccessMessage("header downloader (%d)(%s%s)\n%s\n", downloader.GetResponse(), domain.c_str(), request.c_str(), headertmp.c_str());
        if ((downloader.GetResponse() == 302) || (downloader.GetResponse() == 303)) {
            if (LL > 0) LogFile::AccessMessage("redirect resume: %d\n", downloader.GetResponse());
            headerT = headertmp;
            hit = resuming = downloading = false;
            rewrited = true;
            return true;
        } else if (downloader.GetResponse() != 206) {
            if (LL > 0) LogFile::AccessMessage("resume fail: %d\n", downloader.GetResponse());
            return false;
        }
    }

    if (rewrited) {
        stringstream tmp;
        tmp.str("");

        tmp << "HTTP/1.0 302 Moved\r\n";
        tmp << "Location: http://" << r.file << "\r\n";
        tmp << "Content-Length: 0\r\n";
        tmp << "Connection: close\r\n\r\n";

        headerT = tmp.str();
        return true;
    } else if (hit && etag) {
        stringstream tmp;
        tmp.str("");

        tmp << "HTTP/1.0 304 Not Modified\r\n";
        tmp << "Connection: close\r\n\r\n";

        headerT = tmp.str();
        rewrited = true;
        r.match = hit = false;
        return true;
    } else if (hit) {
        stringstream tmp;
        tmp.str("");
        if (partial == 0)
            tmp << "HTTP/1.0 200 OK\r\n";
        else
            tmp << "HTTP/1.0 206 Partial\r\n";
        //tmp << "Content-Disposition: attachment; filename=\"" + r.file + "\"\r\n";
        if (getFileExtension(r.file) == "SWF")
            tmp << "Content-Type: application/x-shockwave-flash\r\n";
        else if (getFileExtension(r.file) == "FLV")
            tmp << "Content-Type: video/x-flv\r\n";
		else
			tmp << "Content-Type: application/octet-stream\r\n";

        tmp << "Content-Length: ";
        tmp << filesize - partial;
        tmp << "\r\n";
        if (partial > 0) {
            tmp << "Accept-Ranges: bytes\r\n";
            tmp << "Content-Range: bytes " << partial << "-" << (filesize - 1) << "/" << filesize << "\r\n";
        }
        tmp << "X-Cache: HIT from Thunder\r\n";
        tmp << "Thunder: HIT FROM " << r.domain << "\r\n";
        headerT = tmp.str();
        return true;

    } else return ConnectionToHTTP::ReadHeader(headerT);

}

bool ConnectionToHTTP2::AnalyseHeader(string &linesT) {
    //if (LL>1) LogFile::AccessMessage("Passando pelo AnalyseHeader \n");
    if (hit) return true;
    else return ConnectionToHTTP::AnalyseHeader(linesT);
}

bool ConnectionToHTTP2::IsItKeepAlive() {
    //if (LL>1) LogFile::AccessMessage("Passando pelo IsItKeepAlive \n");
    if (hit) return true;
    else return ConnectionToHTTP::IsItKeepAlive();
}

int64_t ConnectionToHTTP2::GetContentLength() {
    //if (LL>1) LogFile::AccessMessage("Passando pelo GetContentLength \n");
    if (hit)
        return filesize;
    else {
        int64_t ContentLengthReference;
        ContentLengthReference = ConnectionToHTTP::GetContentLength();
        if (r.match) {
            if (ContentLengthReference > 0)
                if (general)
                    domaindb.set("UPDATE thunder SET size=" + itoa(ContentLengthReference) + " WHERE file='" + domaindb.sqlconv(subdir + "/" + r.file) + "' and domain='" + r.domain + "';");
                else
                    domaindb.set("UPDATE thunder SET size=" + itoa(ContentLengthReference) + " WHERE file='" + domaindb.sqlconv(r.file) + "' and domain='" + r.domain + "';");
            filesize = ContentLengthReference;
            if (((ContentLengthReference < Params::GetConfigInt(getFileExtension(getFileName(request)) + "_MIN") || (ContentLengthReference > Params::GetConfigInt(getFileExtension(getFileName(request)) + "_MAX") && Params::GetConfigInt(getFileExtension(getFileName(request)) + "_MAX") > 0)) && general) ||
				 (ContentLengthReference < Params::GetConfigInt(UpperCase(r.domain) + "_MIN") || (ContentLengthReference > Params::GetConfigInt(UpperCase(r.domain) + "_MAX") && Params::GetConfigInt(UpperCase(r.domain) + "_MAX") > 0))) {
                r.match = general = false;
                if (LL > 0) LogFile::AccessMessage("MAXMIN CANCEL: Domain: %s File: %s Size: "LLD"\n", r.domain.c_str(), r.file.c_str(), filesize);
            }
        }
        return ContentLengthReference;
    }
}

bool ConnectionToHTTP2::IsItChunked() {
    //if (LL>1) LogFile::AccessMessage("Passando pelo IsItChunked \n");
    if (hit) return false;
    else return ConnectionToHTTP::IsItChunked();
}

string ConnectionToHTTP2::PrepareHeaderForBrowser() {
    //if (LL>1) LogFile::AccessMessage("Passando pelo PrepareHeaderForBrowser \n");
    if (hit) {
        string header;
        ReadHeader(header);
        return header;
    } else if (r.match) {
        timerecord = time(NULL);
        stringstream tmp;
        tmp.str("");
        tmp << ConnectionToHTTP::PrepareHeaderForBrowser();
        tmp << "Thunder: MISS FROM " << r.domain << "\r\n";
        return tmp.str();

    } else return ConnectionToHTTP::PrepareHeaderForBrowser();
}

int ConnectionToHTTP2::GetResponse() {
    //if (LL>1) LogFile::AccessMessage("Passando pelo GetResponse \n");
    int retorno;
    if (hit) retorno = 200;
    else if (!hit && r.match) {
        retorno = ConnectionToHTTP::GetResponse();
        if (retorno != 200) {
            r.match = hit = resuming = false;
        }
    } else retorno = ConnectionToHTTP::GetResponse();
    return retorno;
}

bool ConnectionToHTTP2::CheckForData(int timeout) {
    //if (LL>1) LogFile::AccessMessage("Passando pelo CheckForData \n");
    if (hit) return false;
    else return ConnectionToHTTP::CheckForData(timeout);
}

ssize_t ConnectionToHTTP2::ReadBodyPart(string &bodyT, bool Chunked) {
    //if (LL>1) LogFile::AccessMessage("Passando pelo ReadBodyPart \n");
    if (rewrited) {
        bodyT.append("\r\n", 2);
        return 2;
    }
    ssize_t BodyLength = 0;
    if (resuming) {
        string bodyTmp = "";
        BodyLength = downloader.ReadBodyPart(bodyTmp, Chunked);
        if (!outfile.is_open()) {
            outfile.open(string(completefilepath).c_str(), ios::out | ios::app | ios::binary);
        }
        if (BodyLength > 0)
            outfile.write(bodyTmp.c_str(), BodyLength);
        if ((time(NULL) - timerecord2) > 1) {
            if (LL > 1) LogFile::AccessMessage("resumindo... %d \n", BodyLength);
            outfile.flush();
            file_setmodif(completefilepath);
        }
    }
    if (hit) {
        BodyLength = 0;
        int tbuffer = MAXRECV;
        char memblock[MAXRECV];
        memset(memblock, '\0', sizeof (memblock));
        bodyT = "";
        while (BodyLength == 0) {
            if (!cachefile.is_open()) {
                cachefile.open(string(completefilepath).c_str(), ios::in | ios::binary);

                if (partial > 0) cachefile.seekg(partial);
                timeout = timerecord = time(NULL);
            }
            if (downloading) {
                if ((time(NULL) - timerecord) > 1) {
                    filedownloaded = file_size(completefilepath);
                    timerecord = time(NULL);
                    if (LL > 1) LogFile::AccessMessage("atualizando... %s tbuffer: %d downloaded: "LLD" filesize: "LLD" \n", r.file.c_str(), tbuffer, filedownloaded, filesize);
                }
                if ((filesended + MAXRECV) > filedownloaded) {
                    tbuffer = filedownloaded - filesended;
                } else if (filedownloaded == filesize) {
                    downloading = false;
                    tbuffer = MAXRECV;
                    if (LL > 1) LogFile::AccessMessage("acabou o download, nao precisa ajustar tbuffer %s \n", r.file.c_str());
                } else tbuffer = MAXRECV;
            }
            if (cachefile.eof()) {
                if (LL > 0) LogFile::AccessMessage("EOF: %s\n", r.file.c_str());
                BodyLength = 0;
                cachefile.close();
                break;
            }
            if (tbuffer > 0) {
                cachefile.read(memblock, tbuffer);
                BodyLength = cachefile.gcount();
                if (BodyLength > 0) {
                    filesended += BodyLength;
                    bodyT.append(memblock, BodyLength);
                    timeout = time(NULL);
                } else usleep(rand() % 300000);
            } else usleep(rand() % 300000);
            if ((time(NULL) - timeout) >= 5) {
                if (LL > 0) LogFile::AccessMessage("TIMEOUT: %s\n", r.file.c_str());
                BodyLength = -1;
                cachefile.close();
                break;
            }
            if (filesended == filesize) {
                cachefile.close();
                break;
            }
        }
        return BodyLength;
    } else {
        ssize_t BodyLength;
        BodyLength = ConnectionToHTTP::ReadBodyPart(bodyT, Chunked);
        if (r.match) {
            if (!cachefile.is_open()) {
                cachefile.open(string(completefilepath).c_str(), ios::out | ios::binary);
            }
            if (BodyLength > 0) {
                cachefile.write(bodyT.c_str(), BodyLength);
                filesended += BodyLength;
                if ((time(NULL) - timerecord) > 1) {
                    cachefile.flush();
                    file_setmodif(completefilepath);
                    timerecord = time(NULL);
                }
            }
        }
        return BodyLength;
    }
}

void ConnectionToHTTP2::Close() {
    //if (LL>1) LogFile::AccessMessage("Passando pelo Close\n");
    domaindb.close();
    if (cachefile.is_open()) cachefile.close();
    if (outfile.is_open()) outfile.close();
    if ((filesended < filesize) && ((r.match && !hit) || (r.match && resuming)))
        file_setmodif(completefilepath, 1);

    downloader.Close();
    ConnectionToHTTP::Close();
}

