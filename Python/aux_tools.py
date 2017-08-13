import operator
import itertools

def read_config(filename):
    res = {}
    with open(filename, 'r', encoding='utf-8') as conf_file:
        for line in conf_file:
            line = line.split('#',1)[0].strip() # Remove comments
            if not line:
                continue
            name,val = (x.strip().strip('\"') for x in line.split('=', 1))
            res[name] = val
        return res

def write_sorted_by_key(word_counter, file_name):
    with open(file_name, 'w', encoding='utf-8') as file:
        for (word, occurance) in sorted(word_counter.items()):
            if word:
                file.write('{:15}:{:3}\n'.format(word, occurance))

def write_sorted_by_value(word_counter, file_name):
    with open(file_name, 'w', encoding='utf-8') as file:
        for (word, occurance) in sorted(word_counter.items(), key=operator.itemgetter(1)):  # key=lambda x: x[1]
            if word:
                file.write('{:15}:{:3}\n'.format(word, occurance))

def compareFiles(filename1, filename2):
    with open(filename1, 'r') as file1,  open(filename2, 'r') as file2:
        data1 = file1.readlines()
        data2 = file2.readlines()
    data1 = [ ''.join(c for c in x if not c.isspace() ) for x in data1 ]
    data2 = [ ''.join(c for c in x if not c.isspace() ) for x in data2 ]
    for n, (l1, l2) in enumerate(itertools.zip_longest(data1, data2)):
        if l1 != l2:
            print("Difference at line", n)
            print("\t First  file: |" + str(l1) + "|")
            print("\t Second file: |" + str(l2) + "|")
            return False

    return True

if __name__ == '__main__':
	print('Auxiliary tools for (samle) concurrent word counters')
