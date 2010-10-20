/* 
 * (c) Copyright 2009 Joaquim Pedro França Simão (osmano807) <osmano807@gmail.com>. Some Rights Reserved. 
 * @autor Joaquim Pedro França Simão (osmano807) <osmano807@gmail.com> 
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o redtube.com.so redtube.com.cpp

string get_filename(string url) {
		vector<string> resultado;
		if (url.find("?") != string::npos) {
			stringexplode(url, "?", &resultado);
			stringexplode(resultado.at(resultado.size()-2), "/", &resultado);
			if (url.find("/_videos") != string::npos) {
				return resultado.at(resultado.size()-1);
			} else {
				return resultado.at(resultado.size()-3);
			}
			return resultado.at(resultado.size()-3);           
		} else {
			stringexplode(url, "/", &resultado);
			if (url.find("/_videos") != string::npos) {
				return resultado.at(resultado.size()-1);
			} else {
				return resultado.at(resultado.size()-3);
			}
		}
}
extern "C" resposta getmatch(const string url) {
    resposta r;	
    	
	if ( (url.find("redtube.com/") != string::npos)  and
		 (url.find(".flv") != string::npos) and (url.find("?start=0") != string::npos) //start=0 para nao fazer cache de file incompleto
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "redtube";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}

