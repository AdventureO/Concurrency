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
    punctuation = ['(', ')', '?', ':', ';', ',', '.', '!', '/', '"', "'"]
    for row in range(rows):
        sentence = random.sample(all_words, words_in_row)
        punct_dist = 0
        for word in sentence:
            block = block + " " + word
            if punct_dist % 4 == 0:
                block += random.choice(punctuation)
            punct_dist += 1
        block += "\n"

    return block

def generate_file():
    words = get_all_words("data_files/english_words_10K.txt")
    #files = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048]
    files = [0.5, 3, 5, 10, 20, 30, 40, 50, 70, 80, 90, 100]
    for i in range(len(files)):
        new_name = 'data_' + str(files[i]) + 'MB.txt'
        with open( new_name, 'w') as file:
            while os.stat(new_name).st_size <= files[i]*1048576:
                sentence = generate_block(25, 20, words)
                file.write(sentence)

if __name__ == "__main__":
    generate_file()