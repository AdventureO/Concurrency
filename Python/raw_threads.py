import threading
import multiprocessing
import time

from aux_tools import *
from clean_words import *

# ===============================

# Лише ASCII -- тому решта символів просто ігноруємо
def readData(file_name):
    # with open(file_name, 'r', encoding='utf-8') as datafile:
    with open(file_name, 'r', encoding='ascii', errors='ignore') as datafile:
        return datafile.read().split()


#! Винесено за межі "if __name__ == '__main__':"
# Щоб могти успадкувати WordsCount у кожному із субпроцесів
# Виглядає неохайно, зате просто.
config = read_config('data_input_conc.txt')

infile    = config["infile"]
out_by_a  = config["out_by_a"]
out_by_n  = config["out_by_n"]
threads_n = int(config["threads"])

etalon_a_file = config["etalon_a_file"]
# measurement_flags = int(config["measurement_flags"])

python_flags=config["python_flags"]
if 't' in python_flags:
    parent_class = threading.Thread
    lock_type = threading.Lock()
elif 'p' in python_flags:
    parent_class = multiprocessing.Process
    lock_type = multiprocessing.Lock()
    # manager = multiprocessing.Manager()
    # word_counter = manager.dict()
else:
    raise Exception('Neither threads no processes concurrency selected')

number_of_threads = threads_n


class WordsCount(parent_class):
    # lock = multiprocessing.Lock()
    # lock = threading.Lock()

    def __init__(self, words_list, word_counter, lock):
        super().__init__()
        self.words_list = words_list
        self.word_counter = word_counter
        self.lock = lock

    def run(self):
        local_dict = {}
        for word in self.words_list:
            word = cleanWord(word)
            if word not in local_dict:
                local_dict[word] = 1
            else:
                local_dict[word] += 1

        with self.lock:
            for i in local_dict.keys():
                if i in self.word_counter.keys():
                    self.word_counter[i] += local_dict[i]
                else:
                    self.word_counter[i] = local_dict[i]

if __name__ == '__main__':

    if 't' in python_flags:
        word_counter = {}
    elif 'p' in python_flags:
        manager = multiprocessing.Manager()
        word_counter = manager.dict()
    else:
        raise Exception('Neither threads no processes concurrency selected')

    start_time = (time.perf_counter(), time.process_time())

    input_list = readData(infile)

    start_anal_time = (time.perf_counter(), time.process_time())
    
    threads = []
    avg = len(input_list) / number_of_threads
    last = 0

    while last < len(input_list):
        threads.append(WordsCount(input_list[int(last):int(last + avg)], word_counter, lock_type))
        last += avg

    for thread in threads:
        thread.start()

    for thread in threads:
        thread.join()

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
