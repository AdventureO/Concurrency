import asyncio
import time
from string import punctuation

"""
8.05.17
Виправив async
Тепер працює правильно (я так думаю)
"""
def read_file(file_name):
    words_list = []
    for line in open(file_name, 'r'):
        for word in line.translate(line.maketrans("", "", punctuation)).lower().split():
            words_list.append(word)

    return words_list

def list_divider(list_of_words, parts):
    avg = len(list_of_words) / parts
    result = []
    last = 0

    while last < len(list_of_words):
        result.append(list_of_words[int(last):int(last + avg)])
        last += avg

    return result

def write_file(word_counter, file_name):
    with open(file_name, 'w') as file:
        for (word, occurance) in word_counter.items():
            file.write('{:15}{:3}\n'.format(word, occurance))

async def words_counting(words_list):

    local_dict = {}
    for word in words_list:
        if word not in local_dict:
            local_dict[word] = 1
        else:
            local_dict[word] += 1

    return local_dict



def print_result(word_counter):
    for (word, occurance) in word_counter.items():
        print('{:15} {:3}\n'.format(word, occurance))


number_of_tasks = 4
input_list = list_divider(read_file('text1.txt'), number_of_tasks)
word_counter = {}
tasks = [asyncio.ensure_future(words_counting(i)) for i in input_list]

if __name__ == "__main__":
    start = time.time()

    loop = asyncio.get_event_loop()
    loop.run_until_complete(asyncio.wait(tasks))
    loop.close()

    for i in tasks:
        for word in i.result().keys():
            if word in word_counter.keys():
                word_counter[word] += i.result()[word]

            else:
                word_counter[word] = i.result()[word]

    print("Got {} tasks in {} seconds".format(number_of_tasks, time.time() - start))


write_file(word_counter, "result.txt")
