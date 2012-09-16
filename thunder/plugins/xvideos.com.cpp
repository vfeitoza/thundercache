#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o xvideos.com.so xvideos.com.cpp

string get_filename(string url) {
	vector<string> resultado;
	string::size_type pos;
	string tmp;
	
	stringexplode(url,"/",&resultado);
	url = resultado.at(resultado.size() - 1);
	resultado.clear();
	
	if (url.find("?") != string::npos) {
		stringexplode(url, "?", &resultado);
		url = resultado.at(0);
		if( (pos = url.find(";")) != string::npos )
			url.erase(pos);
		return url;
	} else {
		if( (pos = url.find(";")) != string::npos )
			url.erase(pos);
		return url;
	}
}

extern "C" resposta getmatch(const string url) {
	resposta r;	
	r.file = get_filename(url);
	if ( !r.file.empty() ) {
		r.match = true;
		r.domain = "xvideos";
	} else {
		r.match = false;
	}
	return r;
}

