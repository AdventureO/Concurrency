import random
import os

def get_all_words(filename):
    english_words = []
    with open(filename) as file:
        for line in file:
            english_words.append(line.strip('\n'))

    return english_words

def generate_block(rows, words_in_row, all_words):
    block = ""
    for row in range(rows):
        sentence = random.sample(all_words, words_in_row)
        for word in sentence:
            block = block + " " + word
        block += "\n"

    return block

def generate_file():
    words = get_all_words("english_words_10K.txt")
    files = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096]
    for i in range(len(files)):
        new_name = 'data_' + str(files[i]) + 'MB.txt'
        with open( new_name, 'w') as file:
            while os.stat(new_name).st_size <= files[i]*1048576:
                sentence = generate_block(10, 30, words)
                file.write(sentence)

generate_file()

print(os.stat("data_1MB.txt").st_size/1048576)
print(os.stat("data_2MB.txt").st_size/1048576)
print(os.stat("data_4MB.txt").st_size/1048576)
print(os.stat("data_8MB.txt").st_size/1048576)
print(os.stat("data_16MB.txt").st_size/1048576)
print(os.stat("data_32MB.txt").st_size/1048576)
print(os.stat("data_64MB.txt").st_size/1048576)
print(os.stat("data_128MB.txt").st_size/1048576)
print(os.stat("data_256MB.txt").st_size/1048576)
print(os.stat("data_512MB.txt").st_size/1048576)
print(os.stat("data_1024MB.txt").st_size/1048576)
print(os.stat("data_2048MB.txt").st_size/1048576)
print(os.stat("data_4096MB.txt").st_size/1048576)