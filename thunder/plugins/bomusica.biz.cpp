/*
 * (c) Copyright 2013 Erick Colindres <firecoldangelus@gmail.com>
 * Some Rights Reserved.
 *
 * @autor Erick Colindres <firecoldangelus@gmail.com>
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o bomusica.biz.so bomusica.biz.cpp
// regex
// http.*bomusica\.biz.*(\.swf|\.mp3)

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
 
        if ( (url.find("bomusica.biz") != string::npos)
        ) {
                
            r.file = get_filename(url);
                if (!r.file.empty()) {
                        r.match = true;
                        r.domain = "bomusica.biz";
                } else {
                        r.match = false;
                }
        } else {
                r.match = false;
        }
        return r;
}
