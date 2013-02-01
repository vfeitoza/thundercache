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
// g++ -I. -fPIC -shared -g -o fbcdn.net.so fbcdn.net.cpp
// Regex
// http.*\.fbcdn\.net.*(\.jpg|\.mp4)
bool in_array(const string &needle, const vector< string > &haystack) {
    int max = haystack.size();

    if (max == 0) return false;

for (int iii = 0; iii < max; iii++) {
        if (regex_match(haystack[iii], needle) != "") {
            return true;
}
}
    return false;
}
string dominiotxt="facebook_img";
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
vector<string> black_list; 
black_list.push_back ("safe_image.php");
black_list.push_back ("app_full_proxy.php");
 
if ( (url.find("sphotos") != string::npos) or (url.find("photos") != string::npos) and (in_array(url, black_list) == false)
   ) {
dominiotxt="facebook_photos";
}
if ( (url.find("profile") != string::npos) and (in_array(url, black_list) == false)
   ) {
dominiotxt="facebook_profile";
}
if ( (url.find("static") != string::npos) or (url.find("platform") != string::npos) or (url.find("external") != string::npos) and (in_array(url, black_list) == false)
   ) {
dominiotxt="facebook_small";
}
if ( (url.find("video") != string::npos) or (url.find(".mp4") != string::npos)
   ) {
dominiotxt="facebook_video";
}
   if ( (url.find(".fbcdn.net/") != string::npos) and (in_array(url, black_list) == false)
   ) {
      
       r.file = get_filename(url);
      if (!r.file.empty()) {
         r.match = true;
         r.domain = dominiotxt;
      } else {
         r.match = false;
      }
   } else {
      r.match = false;
   }
   return r;
}
