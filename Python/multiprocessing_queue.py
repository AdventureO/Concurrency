from time import time
from string import punctuation
from multiprocessing import Process, Queue

"""
Версія multiprocessing з Queue

"""
def read_file(file_name):
    words_list = []
    for line in open(file_name, 'r'):
        for word in line.translate(line.maketrans("", "", punctuation)).lower().split():
            words_list.append(word)

    return words_list


def write_file(word_counter, file_name):
    with open(file_name, 'w') as file:
        for (word, occurance) in word_counter.items():
            file.write('{:15} {:3}\n'.format(word, occurance))



class WordsCount(Process):
    #lock = multiprocessing.Lock()

    def __init__(self, words_list, result_queue):
        super().__init__()
        self.words_list = words_list
        self.result_queue = result_queue


    def run(self):

        local_dict = {}
        for word in self.words_list:
            if word not in local_dict:
                local_dict[word] = 1
            else:
                local_dict[word] += 1

        self.result_queue.put(local_dict)




processes_number = 10
if __name__ == '__main__':
    word_counter = {}
    result_queue = Queue()
    processes = []
    input_list = read_file('text1.txt')
    avg = len(input_list) / processes_number
    last = 0


    while last < len(input_list):
        processes.append(WordsCount(input_list[int(last):int(last + avg)], result_queue))
        last += avg

    start_time = time()

    for process in processes:
        process.start()

    for process in processes:
        process.join()

    while not result_queue.empty():
        local_dict = result_queue.get()
        for i in local_dict.keys():
            if i in word_counter.keys():
                word_counter[i] += local_dict[i]

            else:
                word_counter[i] = local_dict[i]

    result_queue.close()

    print(word_counter)
    print('Got {} threads in {} seconds'.format(len(processes), time() - start_time))
    write_file(word_counter, 'result.txt')





