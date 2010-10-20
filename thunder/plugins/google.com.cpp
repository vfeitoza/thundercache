/* 
 * (c) Copyright 2009 Rodrigo Medeiros (rodrigomanga) <rodrigomanga@yahoo.com.br>. Some Rights Reserved. 
 * @autor Rodrigo Medeiros (rodrigomanga) <rodrigomanga@yahoo.com.br> 
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o google.com.so google.com.cpp  

string rewriteurl(string url) {
        string meu_partner = "partner-pub-9370461587396013:l0rtjl-aokf";
//      string meu_partner = "partner-pub-9518885955684772:4wa6hx-kn0t";
	string partner ="";
	string urlt=url;
	vector<string> resultado,valor;
	if ( (urlt.find("/search?") != string::npos||urlt.find("/cse?") != string::npos)){
		SearchReplace(urlt,"?","&");
		stringexplode(urlt, "&", &resultado);
		for (int i=0; i <= resultado.size()-1;i++){
			valor.clear(); 
			stringexplode(resultado.at(i), "=", &valor);
			if (valor.at(0) == "cx") {
				partner = valor.at(1);
				break;
			}
		}
		if (!partner.empty() && partner != meu_partner){
			SearchReplace(url,partner,meu_partner);
			return url;
		} 
		if (partner != meu_partner){
			return url+"&cx="+meu_partner;
		}
	}
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
