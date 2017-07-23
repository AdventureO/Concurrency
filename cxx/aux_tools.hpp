#ifndef CPP_CONC_AUX_TOOLS_HPP_INCLUDED
#define CPP_CONC_AUX_TOOLS_HPP_INCLUDED

#include <atomic>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <chrono>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <cassert>

inline std::chrono::high_resolution_clock::time_point get_current_time_fenced() {
#ifndef __CYGWIN__
    assert(std::chrono::high_resolution_clock::is_steady &&
           "Timer should be steady (monotonic).");
#endif
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = std::chrono::high_resolution_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res_time;
}

template<class D>
inline long long to_us_old(const D& d) {
    return std::chrono::duration_cast<std::chrono::microseconds>(d).count();
}

//! std::map is sorted by keys.
template<typename KeyT, typename ValueT>
void write_sorted_by_key( std::ostream& file, const std::map<KeyT, ValueT>& data )
{
    for(auto& item: data)
        if(!item.first.empty())
            file << item.first << ": " << item.second << '\n';
    // '\n' does not flushes the buffers, so is faster than endl.
}

template<typename MapT>
void write_sorted_by_key( std::ostream& file, const MapT& data )
{
    using VectorOfItemsT = std::vector<
                std::pair<typename MapT::key_type, typename MapT::mapped_type >
                                      >;
    VectorOfItemsT VectorOfItems;
    for(auto& item: data)
    {
        VectorOfItems.emplace_back(item);
    }
    std::sort(VectorOfItems.begin(), VectorOfItems.end());

    for(auto& item: VectorOfItems)
        if(!item.first.empty())
            file << item.first << ": " << item.second << '\n';
}


template<typename MapT>
void write_sorted_by_key( const std::string& filename, const MapT& data )
{
    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error( "Could not open file " + filename);
    }
    write_sorted_by_key(file, data);
}

//! Sorting any std::map here, so need just complete std::map type --
//! no need to overload over std::map/unordered_map
template<typename mapT>
void write_sorted_by_value( std::ostream& file, const mapT& data)
{
    using VectorOfItemsT = std::vector< std::pair<typename mapT::key_type,
                                                  typename mapT::mapped_type> >;
    VectorOfItemsT VectorOfItems;
    for(auto& item: data)
    {
        VectorOfItems.emplace_back(item);
    }
    sort(VectorOfItems.begin(), VectorOfItems.end(),
         [] (const typename VectorOfItemsT::value_type &a,
             const typename VectorOfItemsT::value_type &b) {
                return a.second > b.second;
          }
    );

    for(auto& item: VectorOfItems)
        if(!item.first.empty())
            file << item.first << ": " << item.second << '\n';
}

template<typename MapT>
void write_sorted_by_value( const std::string& filename, const MapT& data )
{
    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error( "Could not open file " + filename);
    }
    write_sorted_by_value(file, data);
}

template<typename T>
T str_to_val(const std::string& s)
{
    std::stringstream ss(s);
    T res;
    ss >> res;
    if(!ss){
        throw std::runtime_error("Failed converting: " + s);
    }
    return res;
}



std::map< std::string, std::string> read_config(const  std::string& filename);

//! Compares two files, ignoring spaces.
//! IT IS BAD FUNCTION. IT PRINTS...
//! As for now I'm just too lazy to create corresponing
//! comparison results structure.
//! Взагалі, ця функція видається мені потворною, але поки переписувати лінь.
//! Для спрощення прочитати обидва файли у вектори і там порівнювати?
//! Якщо посортувати -- можна буде ще й різного сортування файли порівнювати.
//! Але і мінусів при тому багато -- наприклад, губляться рядки відмінносте.
bool compareFiles(const  std::string& file1, const  std::string& file2);

#endif
