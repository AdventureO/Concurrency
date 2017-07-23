#include "measurements.hpp"

#include <iomanip>

#ifdef USE_WIN32_MEASUREMENTS
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include <windows.h>
#include <psapi.h>

#include <pdh.h>
#elif defined USE_POSIX_MEASUREMENTS
#include <cstring>

#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/resource.h>
#ifndef RUSAGE_THREAD
#include <pthread.h>
#endif // RUSAGE_THREAD
#endif // USE_POSIX_MEASUREMENTS

#ifdef EXPERIMENTAL_USE_PCM
#include <cpucounters.h>
#endif

void print_last_error(const std::string& msg)
{
#ifdef USE_WIN32_MEASUREMENTS
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms680582(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms679351(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms679360(v=vs.85).aspx
    LPTSTR lpMsgBuf = NULL;
    DWORD err_code = GetLastError();
    if( FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        lpMsgBuf,
        0, NULL ) )
    {
        std::cerr << msg << "\n\t" << lpMsgBuf << std::endl;
    }else
    {
         DWORD fmt_err_code = GetLastError();
         std::cerr << msg << "\n\t"
                   << "FormatMessage Failed with err code " << fmt_err_code
                   << " while searching msg for err code " << err_code
                   << std::endl;
    }
#elif defined USE_POSIX_MEASUREMENTS
    auto errno_copy = errno;
    std::cerr << msg << "\n\t"
              << "Err : " << errno_copy << ": " << strerror(errno_copy)
              << std::endl;
#else
#error "Unknown measurements method"
#endif
}

mtime_t<uint64_t> get_process_times()
{
    mtime_t<uint64_t> res;
#ifdef USE_WIN32_MEASUREMENTS
    FILETIME creation_time, exit_time, kernel_time, user_time;
    int err_proc = GetProcessTimes(GetCurrentProcess(),
                                &creation_time, &exit_time,
                                &kernel_time, &user_time);
    if(!err_proc)
    {
        print_last_error("Error in GetProcessTimes.");
    }else{
        res.user_time = to_us(user_time);
        res.kern_time = to_us(kernel_time);
        res.total_time = res.user_time + res.kern_time;
    }
#elif defined USE_POSIX_MEASUREMENTS
    tms proc_times;
    times(&proc_times); // Process times

    auto ticks_per_second = sysconf(_SC_CLK_TCK);
    res.kern_time  = 1000000.0 * proc_times.tms_stime / ticks_per_second;
    res.user_time  = 1000000.0 * proc_times.tms_utime / ticks_per_second;
    res.total_time = res.kern_time + res.user_time;
#else // !USE_WIN32_MEASUREMENTS and ! USE_POSIX_MEASUREMENTS
#error "Unknown measurements method"
#endif // USE_WIN32_MEASUREMENTS
    return res;

}

mtime_t<uint64_t> get_thread_times()
{
    mtime_t<uint64_t> res;
#ifdef USE_WIN32_MEASUREMENTS
    FILETIME creation_time, exit_time, kernel_time, user_time;
    int err_proc = GetThreadTimes(GetCurrentThread(),
                                &creation_time, &exit_time,
                                &kernel_time, &user_time);
    if(!err_proc)
    {
        print_last_error("Error in GetThreadTimes.");
    }else{
        res.user_time = to_us(user_time);
        res.kern_time = to_us(kernel_time);
        res.total_time = res.user_time + res.kern_time;
    }
#elif defined USE_POSIX_MEASUREMENTS
#ifdef RUSAGE_THREAD
//! We have RUSAGE_THREAD -- can use getrusage()
    rusage usage_thread;
    getrusage(RUSAGE_THREAD, &usage_thread); // Thread times -- needs _GNU_SOURCE

    res.user_time = to_us(usage_thread.ru_utime);
    res.kern_time  = to_us(usage_thread.ru_stime);
    res.total_time = res.user_time + res.kern_time;
#else // !RUSAGE_THREAD
//! Should use (inferior) clock_gettime()
    clockid_t cid;
    timespec thread_times;
    int err = pthread_getcpuclockid(pthread_self(), &cid);
    if(err == -1){
        print_last_error("Error obtaining thread clock id");
    }
    err = clock_gettime(cid, &thread_times);
    if(err == -1){
        print_last_error("Error in clock_gettime");
    }
    res.total_time = to_us(thread_times); // Other times are "none"
#endif // RUSAGE_THREAD
#else // !USE_WIN32_MEASUREMENTS and ! USE_POSIX_MEASUREMENTS
#error "Unknown measurements method"
#endif // USE_WIN32_MEASUREMENTS
    return res;

}

void working_times_t::print(std::ostream& os, fmt_types fmt) const
{
    using namespace std;
    int w = 25;
    int dw = std::numeric_limits<uint64_t>::digits10+1;
    switch(fmt)
    {
    case READABLE_FMT:
        os << left << setw(w) << "Wall time: " << right << setw(dw) << wall_time << '\n';
        os << left << setw(w) << "Thread total time: " << right << setw(dw) << thread_times.total_time << '\n';
        os << left << setw(w) << "Thread kernel time: " << right << setw(dw) << thread_times.kern_time << '\n';
        os << left << setw(w) << "Thread user time: " << right << setw(dw) << thread_times.user_time << '\n';
        os << left << setw(w) << "Process total time: " << right << setw(dw) << process_times.total_time << '\n';
        os << left << setw(w) << "Process kernel time: " << right << setw(dw) << process_times.kern_time << '\n';
        os << left << setw(w) << "Process user time: " << right << setw(dw) << process_times.user_time << '\n';
        os << flush;
        break;
    case TABLE_FMT:
        w = 10;
        os << left;
        os << setw(w) << wall_time << ' ';
        os << setw(w) << thread_times.total_time << ' ';
        os << setw(w) << thread_times.kern_time << ' ';
        os << setw(w) << thread_times.user_time << ' ';
        os << setw(w) << process_times.total_time << ' ';
        os << setw(w) << process_times.kern_time << ' ';
        os << setw(w) << process_times.user_time << ' ';
        os << flush;
        break;
    default:
        assert(false && "Wrong format");
    }
}

//=======================================================================
sys_measurements_t get_thread_sys_measurements()
{
    sys_measurements_t res;
#ifdef USE_WIN32_MEASUREMENTS
    // Not yet implemented. Use perf. counters.
#elif defined USE_POSIX_MEASUREMENTS
#ifdef 	RUSAGE_THREAD
    rusage usage_thread;
    getrusage(RUSAGE_THREAD, &usage_thread); // Thread times -- needs _GNU_SOURCE
    res.soft_page_faults = usage_thread.ru_minflt;
    res.hard_page_faults = usage_thread.ru_majflt;
    res.total_page_faults = res.soft_page_faults + res.hard_page_faults;

    res.max_resident_size = usage_thread.ru_maxrss * 1024;

    res.voluntary_context_switches = usage_thread.ru_nvcsw;
    res.involuntary_context_switches = usage_thread.ru_nivcsw;
    res.total_context_switches = res.voluntary_context_switches + res.involuntary_context_switches;

#endif // RUSAGE_THREAD
#else // !USE_WIN32_MEASUREMENTS and ! USE_POSIX_MEASUREMENTS
#error "Unknown measurements method"
#endif // USE_WIN32_MEASUREMENTS
    return res;
}


sys_measurements_t get_process_sys_measurements()
{
    sys_measurements_t res;
#ifdef USE_WIN32_MEASUREMENTS
    //! Use perf. counters to obtain other data,
    //! present in POSIX section
    PROCESS_MEMORY_COUNTERS meminfo;
    GetProcessMemoryInfo( GetCurrentProcess( ), &meminfo, sizeof(meminfo) );
    res.max_resident_size = meminfo.PeakWorkingSetSize;
    res.total_page_faults = meminfo.PageFaultCount;

#elif defined USE_POSIX_MEASUREMENTS
    rusage usage_proc;
    getrusage(RUSAGE_SELF, &usage_proc);

    res.soft_page_faults = usage_proc.ru_minflt;
    res.hard_page_faults = usage_proc.ru_majflt;
    res.total_page_faults = res.soft_page_faults + res.hard_page_faults;

    res.max_resident_size = usage_proc.ru_maxrss * 1024;

    res.voluntary_context_switches = usage_proc.ru_nvcsw;
    res.involuntary_context_switches = usage_proc.ru_nivcsw;
    res.total_context_switches = res.voluntary_context_switches + res.involuntary_context_switches;
#else // !USE_WIN32_MEASUREMENTS and ! USE_POSIX_MEASUREMENTS
#error "Unknown measurements method"
#endif // USE_WIN32_MEASUREMENTS
    return res;
}

void sys_measurements_t::print(std::ostream& os, const std::string& prefix, fmt_types fmt) const
{
    using namespace std;
    int w = 40;
    int dw = std::numeric_limits<uint64_t>::digits10+1;
    switch(fmt)
    {
    case READABLE_FMT:
        os << left << setw(w) << prefix + " total page faults: " << right << setw(dw) << total_page_faults << '\n';
        os << left << setw(w) << prefix + " soft page faults: " << right << setw(dw) << soft_page_faults << '\n';
        os << left << setw(w) << prefix + " hard page faults: " << right << setw(dw) << hard_page_faults << '\n';
        os << left << setw(w) << prefix + " total context switches: " << right << setw(dw) << total_context_switches << '\n';
        os << left << setw(w) << prefix + " voluntary context switches: " << right << setw(dw) << voluntary_context_switches << '\n';
        os << left << setw(w) << prefix + " involuntary context switches: " << right << setw(dw) << involuntary_context_switches << '\n';
        os << left << setw(w) << prefix + " maximal resident size diff.: " << right << setw(dw) << max_resident_size << '\n';
        os << flush;
        break;
    case TABLE_FMT:
        w = 10;
        os << left;
        os << setw(w) << total_page_faults << ' ';
        os << setw(w) << soft_page_faults << ' ';
        os << setw(w) << hard_page_faults << ' ';
        os << setw(w) << total_context_switches << ' ';
        os << setw(w) << voluntary_context_switches << ' ';
        os << setw(w) << involuntary_context_switches << ' ';
        os << setw(w) << max_resident_size << ' ';
        os << flush;
        break;
    default:
        assert(false && "Wrong format");
    }
}


cpu_measurements_provider_t::cpu_measurements_provider_t()
{
#ifdef EXPERIMENTAL_USE_PCM
    m = PCM::getInstance();
    PCM::ErrorCode err = m->program();

    switch (err)
    {
        case PCM::Success:
            break;
        case PCM::MSRAccessDenied:
            std::cerr << "MSRAccessDenied." << std::endl;
            exit(EXIT_FAILURE);
        case PCM::PMUBusy:
            std::cerr << "PMU (Performance Monitoring Unit) busy." << std::endl;
            std::cerr << "Reset it? (y/n)" << std::endl;
            char ans;
            std::cin >> ans;
            if (tolower(ans) == 'y' )
            {
                m->resetPMU();
                std::cerr << "PMU reseted. Rerun the program." << std::endl;
            }
            exit(EXIT_FAILURE);
        default:
            std::cerr << "Unknown error." << std::endl;
            exit(EXIT_FAILURE);
    }
#elif defined EXPERIMENTAL_USE_PAPI
#else
#endif
}

cpu_measurements_provider_t::~cpu_measurements_provider_t()
{
#ifdef EXPERIMENTAL_USE_PCM
    m->cleanup();
#elif defined EXPERIMENTAL_USE_PAPI
#endif
}

cpu_measurements_provider_t::data_type cpu_measurements_provider_t::getCPUcounters()
{
#ifdef EXPERIMENTAL_USE_PCM
    return getSystemCounterState();
#elif defined EXPERIMENTAL_USE_PAPI
    return nullptr;
#else
    return nullptr;
#endif
}

void print_cpu_params(const cpu_measurements_provider_t::data_type& before_sstate,
                      const cpu_measurements_provider_t::data_type& after_sstate)
{
#ifdef EXPERIMENTAL_USE_PCM
    using std::setw;
    using std::cout;
    using std::endl;
    std::cout << std::fixed << std::left;
    int w = 35;
    cout << setw(w) << "Core cycles:" << getCycles(before_sstate,after_sstate) << std::endl;
    cout << setw(w) << "Average retired instructions:" << getExecUsage(before_sstate,after_sstate) << std::endl;
    cout << setw(w) << "Retired instructions:" << getInstructionsRetired(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "Invariant TSC:" << getInvariantTSC(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "Referen clock cycles:" << getRefCycles(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "Instructions per clock cycle:" << getIPC(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "Average core frequency:" << getAverageFrequency(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "Average relative core frequency:" << getRelativeFrequency(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "Core cycles lost due to L3 cache misses:" << getCyclesLostDueL3CacheMisses(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "Core cycles lost due to L2 cache misses:" << getCyclesLostDueL2CacheMisses(before_sstate,after_sstate) << std::endl;
    //cout << setw(w) <<  "Core cycles lost due to L1 cache misses:" << getCyclesLostDueL1CacheMisses(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "L2 cache hit ratio:" << getL2CacheHitRatio(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "L2 cache hits:" << getL2CacheHits(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "L2 cache misses:" << getL2CacheMisses(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "L3 cache hit ratio:" << getL3CacheHitRatio(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "L3 cache hits:" << getL3CacheHits(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "L3 cache misses:" << getL3CacheMisses(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "L3 cache occupancy before:" << getL3CacheOccupancy(before_sstate) << std::endl;
    cout << setw(w) <<  "L3 cache occupancy after:" << getL3CacheOccupancy(after_sstate) << std::endl;

    cout << setw(w) <<  "SMI interrupts:" << getSMICount(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "Local Memory Bandwidth:" << getLocalMemoryBW(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "Remote Memory Bandwidth:" << getRemoteMemoryBW(before_sstate,after_sstate) << std::endl;

    cout << setw(w) <<  "Bytes read from RAM:" << getBytesReadFromMC(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "Bytes written to RAM:" << getBytesWrittenToMC(before_sstate,after_sstate) << std::endl;
    cout << setw(w) <<  "I/O Bytes:" << getIORequestBytesFromMC(before_sstate,after_sstate) << std::endl;

    std::cout << setw(w) <<  "Consumed by CPU energy (Joules):" << getConsumedJoules(before_sstate,after_sstate) << std::endl;
    std::cout << setw(w) <<  "Consumed by DRAM energy (Joules):" << getDRAMConsumedJoules(before_sstate,after_sstate) << std::endl;
#elif defined EXPERIMENTAL_USE_PAPI
#else
#endif // EXPERIMENTAL_USE_PCM
}

all_measurements_t::all_measurements_t(
                    cpu_measurements_provider_t& cp,
                    int mts_):
    cpu_provider(cp), mts(mts_)
{

    if(mts & CPU_MSM)
    {
    }

}

void all_measurements_t::measure()
{
    OUR_FENCE();
    if(mts & CPU_MSM)
    {
        ct.push_back(cpu_provider.getCPUcounters());
    }

    if(mts & SYS_MSM)
    {
        st.push_back(get_process_sys_measurements());
    }

    if(mts & BASE_MSM)
    {
        wt.push_back(get_current_times_fenced());
    }
    OUR_FENCE();
}

int all_measurements_t::mark_start(const std::string& marktext)
{
#if defined _WIN32 && defined USE_WINDOWS_CONCURRENCY_VIS_FLAGS
    makrs.push_back(new span(series, 1, _T(marktext.c_str()))); //-V799
    series.write_flag(_T(marktext.c_str()));
    return makrs.size()-1;
#else
    return -1;
#endif
}

void all_measurements_t::mark_finish(int idx)
{
#if defined _WIN32 && defined USE_WINDOWS_CONCURRENCY_VIS_FLAGS
    delete makrs[idx];
#endif
}
