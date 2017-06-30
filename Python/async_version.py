import asyncio
import time
import sys
import datetime


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

async def words_counting(words_list):
    local_dict = {}
    for word in words_list:
        word = "".join(i for i in word if i.isalpha()).lower()
        if word != "":
            if word not in local_dict:
                local_dict[word] = 1
            else:
                local_dict[word] += 1

    return local_dict


with open("input_data") as f:
    content = f.readlines()

content = [x.strip().split("=")[1][1:-1] for x in content]


number_of_tasks = int(sys.argv[1])
input_list = read_file(content[0])
word_counter = {}
avg = len(input_list) / number_of_tasks
last = 0
tasks = []


while last < len(input_list):
    tasks.append(asyncio.ensure_future(words_counting(input_list[int(last):int(last + avg)])))
    last += avg


if __name__ == "__main__":

    start_time = time.time()
    loop = asyncio.get_event_loop()
    loop.run_until_complete(asyncio.wait(tasks))
    loop.close()

    for i in tasks:
        for word in i.result().keys():
            if word in word_counter.keys():
                word_counter[word] += i.result()[word]

            else:
                word_counter[word] = i.result()[word]

    a = round((time.time() - start_time) * 1000000)
    print(a)
    with open('result.txt', 'w') as file:
        file.write('{}\n'.format(a))
        file.write('{}\n'.format(100))


 #   with open(content[1], 'a') as file:
 #       file.write(str((finish_time - start_threads).microseconds) + '\n')
  #      file.write(str((finish_time - start_time).microseconds) + '\n')


    #write_file(word_counter, 'result.txt')
