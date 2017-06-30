from time import time
import os
from multiprocessing import Process, Queue
import sys

"""
multiprocessing with Queue
"""
def read_file(file_name):
    words_list = []
    for line in open(file_name, 'r'):
        for word in line.split():
            words_list.append(word)

    return words_list


def write_file(word_counter, file_name):
    with open(file_name, 'w') as file:
        for (word, occurance) in word_counter.items():
            file.write('{:15} {:3}\n'.format(word, occurance))



class WordsCount(Process):
    def __init__(self, words_list, result_queue):
        super().__init__()
        self.words_list = words_list
        self.result_queue = result_queue


    def run(self):

        #lock = multiprocessing.Lock()
        local_dict = {}
        i = 0
        for word in self.words_list:
            word = "".join(i for i in word if i.isalpha()).lower()
            if word != "":
                if word not in local_dict:
                    local_dict[word] = 1
                else:
                    local_dict[word] += 1


        self.result_queue.put(local_dict)
        #print("Added: ", os.getpid())



with open("input_data") as f:
    content = f.readlines()

content = [x.strip().split("=")[1] for x in content]
for i in range(len(content)-1):
    content[i] = content[i][1:-1]

processes_number = int(sys.argv[1])
if __name__ == '__main__':
    word_counter = {}
    result_queue = Queue()
    processes = []
    input_list = read_file(content[0])
    avg = len(input_list) / processes_number
    last = 0

    while last < len(input_list):
        processes.append(WordsCount(input_list[int(last):int(last + avg)], result_queue))
        last += avg

    start_time = time()

    for process in processes:
        process.start()

    while True:
        running_processes = any(p.is_alive() for p in processes)
        while not result_queue.empty():
            local_dict = result_queue.get()
            for i in local_dict.keys():
                if i in word_counter.keys():
                    word_counter[i] += local_dict[i]
                else:
                    word_counter[i] = local_dict[i]
        if not running_processes:
            break

    for process in processes:
        process.join()


    a = round((time() - start_time) * 1000000)
    print(a)
    with open('result.txt', 'w') as file:
        file.write('{}\n'.format(a))
        file.write('{}\n'.format(100))
