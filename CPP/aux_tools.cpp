// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "aux_tools.hpp"

#include <algorithm>
#include <iostream>

using namespace std;

std::map<string,string> read_config(const string& filename)
{
    map<string,string> result;
    ifstream confFile(filename);
    if( !confFile.is_open() ){
        throw std::runtime_error("Failed to open file: " + filename);
    }
    string str;
    while( getline(confFile,str) )
    {
        auto cut_pos = str.find('#');
        if(cut_pos!=string::npos)
            str.erase(cut_pos);
        auto cut_iter = remove_if(str.begin(), str.end(), ::isspace);
        str.erase( cut_iter, str.end() );
        if( str.empty() )
            continue;
        auto pos = str.find('=');
        if(pos==string::npos){
            throw std::runtime_error("Wrong option: " + str + "\n in file: " + filename);
        }
        string name {str, 0, pos};
        string value{str, pos+1, string::npos};
        cut_iter = std::remove(value.begin(), value.end(), '\"');
        value.erase(cut_iter, value.end());
        result[name] = value;
    }
    return result;

}

bool compareFiles(const  std::string& file1, const  std::string& file2)
{
    std::ifstream f1(file1);
    std::ifstream f2(file2);

    if (f1.fail()) {
        throw std::runtime_error("Failed to open file for comparison: " + file1);
    }
    if (f2.fail()) {
        throw std::runtime_error("Failed to open file for comparison: " + file2);
    }
    string str1, str2;
    size_t cur_line = 0;
    while(getline(f1, str1), getline(f2, str2), f1 && f2)
    {
        str1.erase( remove_if(str1.begin(), str1.end(), ::isspace), str1.end() );
        str2.erase( remove_if(str2.begin(), str2.end(), ::isspace), str2.end() );
        if(str1 != str2)
        {
            cerr << "Difference at line " << cur_line << endl;
            cerr << "\t First  file: |" << str1 << "|" << endl;
            cerr << "\t Second file: |" << str2 << "|" << endl;
            return false;
        }
        ++cur_line;
    }
    // Remove empty lines at the end
    if( !f1.eof() )
    {
        do
        {
            str1.erase( remove_if(str1.begin(), str1.end(), ::isspace), str1.end() );
            if(!str1.empty())
            {
                cerr << "Excess line in file 1: " << str1 << endl;
                return false;
            }
        }
        while(getline(f1, str1));
    }
    if( !f2.eof() )
    {
        do
        {
            str2.erase( remove_if(str2.begin(), str2.end(), ::isspace), str2.end() );
            if(!str2.empty())
            {
                cerr << "Excess line in file 1: " << str2 << endl;
                return false;
            }
        }
        while(getline(f2, str2));
    }

    if(f1.eof() && f2.eof() )
    {
        return true;
    }
    else
    {
        if( f2.eof() )
        { // First file is not finished
            cerr << "First file is longer." << endl;
        }else
        { // Second file is not finished
            cerr << "Second file is longer." << endl;
        }
        return false;
    }
}
