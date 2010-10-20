/* 
 * (c) Copyright 2009 Joaquim Pedro França Simão (osmano807) <osmano807@gmail.com>. Some Rights Reserved. 
 * @autor Joaquim Pedro França Simão (osmano807) <osmano807@gmail.com> 
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o symantecliveupdate.com.so symantecliveupdate.com.cpp

string get_filename(string url) {
		vector<string> resultado;
		if (url.find("?") != string::npos) {
			stringexplode(url, "?", &resultado);
			stringexplode(resultado.at(resultado.size()-2), "/", &resultado);
			return resultado.at(resultado.size()-1);           
		} else {
			stringexplode(url, "/", &resultado);
			return resultado.at(resultado.size()-1);
		}
}

extern "C" resposta getmatch(const string url) {
    resposta r;
    
	// http://liveupdate.symantecliveupdate.com/automatic$20liveupdate_3.0.0.154_portuguese_livetri.zip
	
	if ( (url.find(".symantecliveupdate.com/") != string::npos)  and
	   ( (url.find(".zip") != string::npos) or (url.find(".exe") != string::npos) )
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "symantec";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}

