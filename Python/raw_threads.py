import threading
import multiprocessing
from time import time

def cleanWord(word):
    word = ''.join(x.lower() for x in word if x.isalnum())
    return word;
# ===============================
def read_config(filename):
    res = {}
    with open(filename, 'r', encoding='utf-8') as conf_file:
        for line in conf_file:
            line = line.split('#',1)[0].strip() # Remove comments
            if not line:
                continue
            name,val = (x.strip().strip('\"') for x in line.split('=', 1))
            res[name] = val
        return res

def write_sorted_by_key(word_counter, file_name):
    with open(file_name, 'w', encoding='utf-8') as file:
        for (word, occurance) in sorted(word_counter.items()):
            if word:
                file.write('{:15}:{:3}\n'.format(word, occurance))

import operator
def write_sorted_by_value(word_counter, file_name):
    with open(file_name, 'w', encoding='utf-8') as file:
        for (word, occurance) in sorted(word_counter.items(), key=operator.itemgetter(1)):  # key=lambda x: x[1]
            if word:
                file.write('{:15}:{:3}\n'.format(word, occurance))

import itertools
def compareFiles(filename1, filename2):
    with open(filename1, 'r') as file1,  open(filename2, 'r') as file2:
        data1 = file1.readlines()
        data2 = file2.readlines()
    data1 = [ ''.join(c for c in x if not c.isspace() ) for x in data1 ]
    data2 = [ ''.join(c for c in x if not c.isspace() ) for x in data2 ]
    for n, (l1, l2) in enumerate(itertools.zip_longest(data1, data2)):
        if l1 != l2:
            print("Difference at line", n)
            print("\t First  file: |" + str(l1) + "|")
            print("\t Second file: |" + str(l2) + "|")
            return False

    return True
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

    input_list = readData(infile)
    threads = []

    avg = len(input_list) / number_of_threads
    last = 0

    while last < len(input_list):
        threads.append(WordsCount(input_list[int(last):int(last + avg)], word_counter, lock_type))
        last += avg

    start_time = time()

    for thread in threads:
        thread.start()

    for thread in threads:
        thread.join()

    work_time = time() - start_time
    if 't' in python_flags:
        print('Got {} threads in {} seconds'.format(len(threads), work_time))
    else:
        print('Got {} processes in {} seconds'.format(len(threads), work_time))

    write_sorted_by_key(word_counter,   out_by_a)
    write_sorted_by_value(word_counter, out_by_n)

    are_correct = True
    if etalon_a_file:
        are_correct = compareFiles(out_by_a, etalon_a_file);
    exit(not are_correct)
