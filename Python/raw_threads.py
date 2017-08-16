import threading
import time

from aux_tools import *
from clean_words import *

# ===============================

# Лише ASCII -- тому решта символів просто ігноруємо
def readData(file_name):
    # with open(file_name, 'r', encoding='utf-8') as datafile:
    with open(file_name, 'r', encoding='ascii', errors='ignore') as datafile:
        return datafile.read().split()


def WordsCount(words_list, beg, end, word_counter, lock):
    local_dict = {}
    # for word in itertools.islice(words_list, beg, end):
    for word in words_list:
        word = cleanWord(word)
        if word not in local_dict:
            local_dict[word] = 1
        else:
            local_dict[word] += 1

    with lock:
        for i in local_dict.keys():
            if i in word_counter.keys():
                word_counter[i] += local_dict[i]
            else:
                word_counter[i] = local_dict[i]

if __name__ == '__main__':

    config = read_config('data_input_conc.txt')

    infile    = config["infile"]
    out_by_a  = config["out_by_a"]
    out_by_n  = config["out_by_n"]
    threads_n = int(config["threads"])
    etalon_a_file = config["etalon_a_file"]

    start_time = (time.perf_counter(), time.process_time())

    input_list = readData(infile)

    start_anal_time = (time.perf_counter(), time.process_time())

    word_counter = {}
    lock = threading.Lock()

    threads = []
    avg = len(input_list) / threads_n
    last = 0

    while last < len(input_list):
        threads.append( threading.Thread(target = WordsCount, args = (input_list[int(last):int(last + avg)], int(last), int(last + avg), word_counter, lock) ) )
        # threads.append( threading.Thread(target = WordsCount, args = (input_list, int(last), int(last + avg), word_counter, lock) ) )        
        last += avg

    for process in threads:
        process.start()

    for process in threads:
        process.join()

    finish_time = (time.perf_counter(), time.process_time())

    print( 'Total:' )
    print( 'Wall time: {0}'.format(finish_time[0] - start_time[0]) )
    print( 'Process total time: {0}'.format(finish_time[1] - start_time[1]) )

    print( 'Analisys:' )
    print( 'Wall time: {0}'.format(start_anal_time[0] - start_time[0]) )
    print( 'Process total time: {0}'.format(start_anal_time[1] - start_time[1]) )

    write_sorted_by_key(word_counter,   out_by_a)
    write_sorted_by_value(word_counter, out_by_n)

    are_correct = True
    if etalon_a_file:
        are_correct = compareFiles(out_by_a, etalon_a_file);
    exit(not are_correct)

