import time

from aux_tools import *
from clean_words import *


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

start_time = (time.perf_counter(), time.process_time())

words_list = readData(infile)

start_anal_time = (time.perf_counter(), time.process_time())

word_counter = {}
for word in words_list:
    word = cleanWord(word)
    if word not in word_counter:
        word_counter[word] = 1
    else:
        word_counter[word] += 1

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
