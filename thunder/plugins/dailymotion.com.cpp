/* 
 * (c) Copyright 2009 Joaquim Pedro França Simão (osmano807) <osmano807@gmail.com>. Some Rights Reserved. 
 * @autor Joaquim Pedro França Simão (osmano807) <osmano807@gmail.com> 
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"


// use this line to compile
// g++ -I. -fPIC -shared -g -o dailymotion.com.so dailymotion.com.cpp

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
	
	
	if ( ( (regex_match("proxy[a-z0-9\\-][a-z0-9][a-z0-9][a-z0-9]?\\.dailymotion\\.com", url) != "") or 
			(url.find("vid.akm.dailymotion.com") != string::npos) or (url.find(".cdn.dailymotion.com") != string::npos) ) and
		( (url.find(".flv") != string::npos) or (url.find(".h264") != string::npos) or (url.find(".mp4") != string::npos) or
			(url.find(".on2") != string::npos) )
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "dailymotion";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
	
}

