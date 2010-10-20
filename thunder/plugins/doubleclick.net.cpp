/* 
 * (c) Copyright 2009 Rodrigo Medeiros (rodrigomanga) <rodrigomanga@yahoo.com.br>. Some Rights Reserved. 
 * @autor Rodrigo Medeiros (rodrigomanga) <rodrigomanga@yahoo.com.br> 
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o doubleclick.net.so doubleclick.net.cpp  

string rewriteurl(string url) {
	string banner = "zion2.zionlanhouse.com.br:8080/msn.htm";

	if ( url.find("doubleclick.net/pagead/ads?") != string::npos ){
		url = banner;
		return url;
	} else 
		return "";
}

extern "C" resposta getmatch(const string url) {
    resposta r;
	r.file = rewriteurl(url);
	if (!r.file.empty()) {
		r.match = true;
		r.domain = "rewrite";
	} else {
		r.match = false;
	}
	return r;
}
