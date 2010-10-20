#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o plugin.so plugin.cpp

string get_videoid(string url){
	vector<string> resultado,valor;
	string retorna = "";
	SearchReplace(url,"?","&");
	stringexplode(url, "/", &resultado);
	if (resultado.size() > 1){
	    url = resultado.at(1);
	    resultado.clear();
	    stringexplode(url, "&", &resultado);
	    for (int i=0; i <= resultado.size()-1;i++){
		    valor.clear(); 
		    stringexplode(resultado.at(i), "=", &valor);
		    if (valor.at(0) == "id" || valor.at(0) == "video_id") {
			    retorna  = valor.at(1);
			    break;
		    }

	    }
    }	
	return retorna;
		
}
// o regex retorna a parte do texto encontrada na linha
//regex_match(regex,texto);

extern "C" resposta getmatch(const string url) {
    resposta r;

	r.file = get_videoid(url);
	if (	!r.file.empty() and
		((url.find(".googlevideo.com") != string::npos) or (url.find(".youtube.com") != string::npos) or
		 (regex_match("74\\.125\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)", url) != "")) and
		(url.find("videoplayback") != string::npos) and
		(url.find("begin=") == string::npos)
	) {
		r.match = true;
		r.domain = "youtube";
		r.file += ".flv";
	} else {
		r.match = false;
	}
	return r;
}

