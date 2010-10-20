/* 
 * (c) Copyright 2009 Joaquim Pedro França Simão (osmano807) <osmano807@gmail.com>. Some Rights Reserved. 
 * @autor Joaquim Pedro França Simão (osmano807) <osmano807@gmail.com> 
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o rapidshare.com.so rapidshare.com.cpp

string get_filename(string url) {
		vector<string> resultado;
		stringexplode(url, "/", &resultado);
		return resultado.at(resultado.size()-3) + "_" + resultado.at(resultado.size()-1);           
}

extern "C" resposta getmatch(const string url) {
    resposta r;	

	if (	(url.find(".rapidshare.com/files") != string::npos) and
			(regex_match("files/[0-9]+/[0-9]+/.*\\.", url) != "")
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "rapidshare";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}
