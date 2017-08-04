import threading
import multiprocessing
from time import time
from string import punctuation

def read_config(filename):
    res = {}
    with open(filename, 'r', encoding='utf-8') as conf_file:
        for line in conf_file:
            line = line.split('#',1)[0].strip() # Remove comments
            if not line:
                continue
            name,val = (x.strip() for x in line.split('=', 1))
            res[name] = val
        return res

def read_file(file_name):
    words_list = []
    for line in open(file_name, 'r', encoding='utf-8'):
        for word in line.translate(line.maketrans("", "", punctuation)).lower().split():
            words_list.append(word)

    return words_list

#    return [word for line in open(file_name, 'r') for word in line.replace(',','').replace('\'','').\
#        replace('.','').replace(';','').replace(':','').lower().split()]


def write_file(word_counter, file_name):
    with open(file_name, 'w') as file:

        for (word, occurance) in word_counter.items():
            file.write('{:15}{:3}\n'.format(word, occurance))

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
else:
    raise Exception('Neither threads no processes concurrency selected')

number_of_threads = threads_n


class WordsCount(parent_class):

    word_counter = {}
    lock = lock_type

    # lock = multiprocessing.Lock()
    # lock = threading.Lock()

    def __init__(self, words_list):
        super().__init__()
        self.words_list = words_list

    def run(self):

        local_dict = {}
        for word in self.words_list:
            if word not in local_dict:
                local_dict[word] = 1
            else:
                local_dict[word] += 1

        with WordsCount.lock:
            for i in local_dict.keys():
                if i in WordsCount.word_counter.keys():
                    WordsCount.word_counter[i] += local_dict[i]

                else:
                    WordsCount.word_counter[i] = local_dict[i]

if __name__ == '__main__':

    input_list = read_file('text1.txt')
    threads = []

    avg = len(input_list) / number_of_threads
    last = 0

    while last < len(input_list):
        threads.append(WordsCount(input_list[int(last):int(last + avg)]))
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

    print(WordsCount.word_counter)
    write_file(WordsCount.word_counter, 'result.txt')

