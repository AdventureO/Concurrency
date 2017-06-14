from concurrent.futures import ThreadPoolExecutor,  ProcessPoolExecutor
from string import punctuation
import time


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


def words_counting(words_list, word_counter):

    local_dict = {}
    for word in words_list:
        if word not in local_dict:
            local_dict[word] = 1
        else:
            local_dict[word] += 1

    return local_dict

"""
Зміна між ThreadPoolExecutor та  ProcessPoolExecutor
"""
with open("input_data") as f:
    content = f.readlines()

content = [x.strip().split("=")[1] for x in content]
for i in range(len(content)-1):
    content[i] = content[i][1:-1]

while True:

    if content[2] == "t":
        print("Threading")
        type = ThreadPoolExecutor
        break

    elif content[2] == "m":
        print("Multiprocessing")
        type = ProcessPoolExecutor
        break


input_list = read_file(content[0])
number_of_workers = int(content[-1])

"""
ділення масиву слів на
певну кількість - number_of_workers
"""
avg = len(input_list) / number_of_workers
last = 0
word_counter = {}
results = []

start_time = time.time()

with type(max_workers=number_of_workers) as pool:
    while last < len(input_list):
        results.append(pool.submit(words_counting, input_list[int(last):int(last + avg)], word_counter))
        last += avg

    for future in results:
        for word in future.result().keys():
            if word in word_counter.keys():
                word_counter[word] += future.result()[word]

            else:
                word_counter[word] = future.result()[word]

print(time.time() - start_time)
write_file(word_counter, content[1])