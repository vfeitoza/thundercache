#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o plugin.so plugin.cpp

extern "C" resposta getmatch(const string url) {
    resposta r;
        
        vector<string> resultado;
        string tmp;
        //cout << url << endl;
        if (
                (url.find("orkut.com") != string::npos) and
                (url.find("images/small/") != string::npos)
        ) {
                r.match = true;
                r.domain = "orkut/small";
                resultado.clear();
                stringexplode(url, "/", &resultado);
                r.file = resultado.at(resultado.size()-2)+"_"+resultado.at(resultado.size()-1);
        } else if (
		         (url.find("orkut.com") != string::npos) and
                (url.find("chhota") != string::npos)
        ) {
                r.match = true;
                r.domain = "orkut/chhota";
                resultado.clear();
                stringexplode(url, "/", &resultado);
                r.file = resultado.at(resultado.size()-3)+"_"+resultado.at(resultado.size()-2)+"_"+resultado.at(resultado.size()-1);
        } else if (
                (url.find("orkut.com") != string::npos) and
                (url.find("images/klein/") != string::npos)
        ) {
                r.match = true;
                r.domain = "orkut/klein";
                resultado.clear();
                stringexplode(url, "/", &resultado);
                r.file = resultado.at(resultado.size()-3)+"_"+resultado.at(resultado.size()-2)+"_"+resultado.at(resultado.size()-1);
        } else if (
                (url.find("orkut.com") != string::npos) and
                (url.find("images/mittel/") != string::npos)
        ) {
                r.match = true;
                r.domain = "orkut/mittel";
                resultado.clear();
                stringexplode(url, "/", &resultado);
                r.file = resultado.at(resultado.size()-3)+"_"+resultado.at(resultado.size()-2)+"_"+resultado.at(resultado.size()-1);
        } else if (
                (url.find("orkut.com") != string::npos) and
                (url.find("images/milieu/") != string::npos)
        ) {
                r.match = true;
                r.domain = "orkut/milieu";
                resultado.clear();
                stringexplode(url, ".jpg", &resultado);
                tmp = resultado.at(resultado.size()-2);
                resultado.clear();
                stringexplode(url, "/", &resultado);
                r.file = resultado.at(resultado.size()-5)+"_"+resultado.at(resultado.size()-4)+"_"+resultado.at(resultado.size()-3)+"_"+resultado.at(resultado.size()-2)+"_"+resultado.at(resultado.size()-1)+".jpg";
        } else if (
                (url.find("orkut.com") != string::npos) and
                (url.find("images/medium/") != string::npos)
        ) {
                r.match = true;
                r.domain = "orkut/medium";
                resultado.clear();
                stringexplode(url, "/", &resultado);
                r.file = resultado.at(resultado.size()-3)+"_"+resultado.at(resultado.size()-2)+"_"+resultado.at(resultado.size()-1);
        } else if (
                (url.find("orkut.com") != string::npos) and
                ((url.find("/albums/") != string::npos) or (url.find("/photos/") != string::npos))
        ) {
                r.match = true;
                r.domain = "orkut/albums";
                resultado.clear();
                stringexplode(url, "/", &resultado);
                r.file = resultado.at(resultado.size()-1);
        } else {
                r.match = false;
        }
        return r;
        
}


