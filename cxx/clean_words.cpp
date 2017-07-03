// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "clean_words.hpp"

#include <cctype>
#include <algorithm>

void cleanWord(std::string &word)
{
#ifdef _MSC_VER
	// isXXX function have assertions in MSVC...
    word.erase(remove_if(word.begin(), word.end(), [](unsigned char a) { return !isalnum(a); }), word.end());
	transform(word.begin(), word.end(), word.begin(), [](unsigned char a) { return tolower(a); });

#else
    word.erase( remove_if(word.begin(), word.end(), [](char x){ return !isalnum(x);}), word.end() );
    transform(word.begin(), word.end(), word.begin(), ::tolower);
#endif 
}
