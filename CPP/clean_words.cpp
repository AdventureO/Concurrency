// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "clean_words.hpp"

#include <cctype>
#include <algorithm>

void cleanWord(std::string &word)
{
    word.erase( remove_if(word.begin(), word.end(), ::ispunct), word.end() );
    transform(word.begin(), word.end(), word.begin(), ::tolower);
}
