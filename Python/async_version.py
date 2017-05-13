import asyncio
import time
from string import punctuation

"""
Виправив async
Тепер працює правильно (я так думаю)
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
input_list = read_file('text1.txt')
word_counter = {}
avg = len(input_list) / number_of_tasks
last = 0
tasks = []

while last < len(input_list):
    tasks.append(asyncio.ensure_future(words_counting(input_list[int(last):int(last + avg)])))
    last += avg


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
