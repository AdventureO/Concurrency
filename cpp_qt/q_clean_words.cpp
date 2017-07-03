// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "q_clean_words.hpp"

#include <utility>
#include <algorithm>
#include <functional>
#include <QLocale>

namespace {
    QLocale locale("C");
}

//! RegExp are tooo slow.
void qtCleanWord(QString &word){
    //! We need pure ASCII, so <127.
    auto pos = std::remove_if(word.begin(), word.end(),                              
                            [](QChar c){ return !c.isLetterOrNumber() || c.unicode() > 127 ;});
    word = locale.toLower( word.remove( pos - word.begin(), word.size() ) );
}
