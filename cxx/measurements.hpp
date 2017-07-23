#ifndef MEASUREMENTS_HPP_FILE_INCLUDED
#define MEASUREMENTS_HPP_FILE_INCLUDED

#if defined _WIN32 && !defined __CYGWIN__
#define USE_WIN32_MEASUREMENTS
#else
//! Звичайно, не всі інші системи є POSIX-системами,
//! але це розумний default, а детальнішу диференціацію
//! можна провести в реалізації
#define USE_POSIX_MEASUREMENTS
#endif

#include <atomic>
#include <chrono>
#include <cassert>
#include <cstdint>
#include <string>
//#include <functional>
#include <vector>

#include <iostream>


#ifdef  USE_WIN32_MEASUREMENTS
#ifndef NOMINMAX
#define NOMINMAX
// "Using NOMINMAX is the only not-completely-evil way to include <windows.h>.
// You should also define UNICODE and STRICT. Although the latter is defined
// by default by modern implementations."
// From: https://stackoverflow.com/questions/4913922/possible-problems-with-nominmax-on-visual-c
#endif // NOMINMAX
#include <windows.h>
#elif defined USE_POSIX_MEASUREMENTS
#include <time.h>
#endif

#ifdef EXPERIMENTAL_USE_PCM
#include <cpucounters.h>
#endif

#if defined _WIN32 && defined USE_WINDOWS_CONCURRENCY_VIS_FLAGS
#include <cvmarkersobj.h>
using namespace Concurrency::diagnostic;
#endif


const bool PRINT_NON_STEADY = true;

void print_last_error(const std::string& msg);

#define OUR_FENCE() do{std::atomic_thread_fence(std::memory_order_seq_cst);}while(false)

inline std::chrono::high_resolution_clock::time_point get_current_wall_time_fenced() {
    if(!std::chrono::high_resolution_clock::is_steady && PRINT_NON_STEADY)
        std::cerr << "Warning: std::chrono::high_resolution_clock is not steady." << std::endl;
    OUR_FENCE();
    auto res_time = std::chrono::high_resolution_clock::now();
    OUR_FENCE();
    return res_time;
}


template<class D>
inline uint64_t to_us(const D& d) {
    return std::chrono::duration_cast<std::chrono::microseconds>(d).count();
}

template<typename T>
struct ticks_t
{
    static constexpr T none = std::numeric_limits<T>::max();
    T val = none;
    template<typename Op>
    static T op_if_present(const T& a, const T& b, Op op){
        if( a!= none && b != none )
            return op(a,b);
        else
            return none;
    }
    ticks_t& operator -=(const ticks_t& rhs){
        val  = op_if_present(val,  rhs.val,  std::minus<T>());
        return *this;
    }
    ticks_t& operator +=(const ticks_t& rhs){
        val  = op_if_present(val,  rhs.val,  std::plus<T>());
        return *this;
    }

    ticks_t& operator=(const T& v) {
        val = v;
        return *this;
    }
    ticks_t(const T& v): val(v) {}
    ticks_t() = default;
};

template<typename T>
inline ticks_t<T> operator-(ticks_t<T> a, const ticks_t<T>& b){
    return a-=b;
}
template<typename T>
inline ticks_t<T> operator+(ticks_t<T> a, const ticks_t<T>& b){
    return a+=b;
}
template<typename T>
std::ostream& operator<<(std::ostream& os, const ticks_t<T>& t)
{
    if(t.val == t.none)
        os << "not_found";
    else
        os << t.val;
    return os;
}

template<typename T>
struct mtime_t
{
    ticks_t<T> user_time, kern_time, total_time;
    mtime_t& operator -=(const mtime_t& rhs){
        user_time  -= rhs.user_time;
        kern_time  -= rhs.kern_time;
        total_time -= rhs.total_time;
        return *this;
    }
    mtime_t& operator +=(const mtime_t& rhs){
        user_time  += rhs.user_time;
        kern_time  += rhs.kern_time;
        total_time += rhs.total_time;
        return *this;
    }
};

template<typename T>
inline mtime_t<T> operator-(mtime_t<T> a, const mtime_t<T>& b){
    return a-=b;
}
template<typename T>
inline mtime_t<T> operator+(mtime_t<T> a, const mtime_t<T>& b){
    return a+=b;
}

#ifdef USE_WIN32_MEASUREMENTS
inline uint64_t to_us(const FILETIME& ft){ //! Unit -- 100ns
    return ( (static_cast<uint64_t>(ft.dwHighDateTime) << 32) + ft.dwLowDateTime)/10; //-V112
}
#elif defined USE_POSIX_MEASUREMENTS
inline uint64_t to_us(const timespec & t){
    return t.tv_sec * 1000000 + t.tv_nsec / 1000;
}

inline uint64_t to_us(const timeval& t){
 return t.tv_sec * 1000000 + t.tv_usec;
}
#endif

mtime_t<uint64_t> get_process_times();
mtime_t<uint64_t> get_thread_times();


struct working_times_t
{
    mtime_t<uint64_t> process_times;
    mtime_t<uint64_t> thread_times;
    ticks_t<uint64_t> wall_time;

    working_times_t& operator -=(const working_times_t& rhs)
    {
        process_times -= rhs.process_times;
        thread_times  -= rhs.thread_times;
        wall_time     -= rhs.wall_time;
        return *this;
    }

    enum fmt_types{READABLE_FMT, TABLE_FMT};
    void print(std::ostream& os, fmt_types fmt = READABLE_FMT) const;
};

inline working_times_t operator-(working_times_t a, const working_times_t& b)
{
    return a-=b;
}


inline working_times_t get_current_times_fenced()
{
    working_times_t res;
    OUR_FENCE();
    res.process_times = get_process_times();
    res.thread_times = get_thread_times();
    res.wall_time = std::chrono::duration_cast<std::chrono::microseconds>
                            (std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    OUR_FENCE();
    return res;
}

//! Реалізація поки часткова, зокрема, варто залучити активніше
//! win32 performance counters.
struct sys_measurements_t
{
    ticks_t<uint64_t> soft_page_faults;
    ticks_t<uint64_t> hard_page_faults;
    ticks_t<uint64_t> total_page_faults;

    ticks_t<uint64_t> voluntary_context_switches;
    ticks_t<uint64_t> involuntary_context_switches;
    ticks_t<uint64_t> total_context_switches;

    ticks_t<uint64_t> max_resident_size;
    sys_measurements_t& operator -=(const sys_measurements_t& rhs)
    {
        soft_page_faults -= rhs.soft_page_faults;
        hard_page_faults -= rhs.hard_page_faults;
        total_page_faults -= rhs.total_page_faults;
        voluntary_context_switches -= rhs.voluntary_context_switches;
        involuntary_context_switches -= rhs.involuntary_context_switches;
        total_context_switches -= rhs.total_context_switches;
        max_resident_size -= rhs.max_resident_size;
        return *this;
    }

    enum fmt_types{READABLE_FMT, TABLE_FMT};
    void print(std::ostream& os, const std::string& prefix = "", fmt_types fmt = READABLE_FMT) const;
};

inline sys_measurements_t operator-(sys_measurements_t a, const sys_measurements_t& b)
{
    return a-=b;
}

sys_measurements_t get_thread_sys_measurements();
sys_measurements_t get_process_sys_measurements();

//===============================================================

//! Most platform-specific class...
struct cpu_measurements_provider_t
{
#ifdef EXPERIMENTAL_USE_PCM
    PCM * m;
    typedef SystemCounterState data_type;
#elif defined EXPERIMENTAL_USE_PAPI
    typedef void* data_type;
#else
    typedef void* data_type;
#endif // EXPERIMENTAL_USE_PCM

    data_type getCPUcounters();

    cpu_measurements_provider_t();
    ~cpu_measurements_provider_t();

};

void print_cpu_params(const cpu_measurements_provider_t::data_type& before,
                      const cpu_measurements_provider_t::data_type& after);
//===============================================================

//! Should be the one!
struct all_measurements_t
{
#if defined _WIN32 && defined USE_WINDOWS_CONCURRENCY_VIS_FLAGS
    marker_series series;
    std::vector<span*> makrs;
#endif

    std::vector<working_times_t> wt;
    std::vector<sys_measurements_t> st;
    std::vector<cpu_measurements_provider_t::data_type> ct;
    cpu_measurements_provider_t &cpu_provider;

    enum measurement_types{BASE_MSM = 1, SYS_MSM = 1<<1, CPU_MSM = 1<<2};
    int mts;

    explicit all_measurements_t(cpu_measurements_provider_t& cp,
                                int mts_ = BASE_MSM | SYS_MSM | CPU_MSM);

    void measure();
    //! Put mark for tracers/profiles/etc
    int mark_start(const std::string& marktext);
    void mark_finish(int idx);
};


#endif
