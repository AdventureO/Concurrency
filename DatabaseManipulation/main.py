import MySQLdb
import os
from tempfile import mkstemp
from shutil import move
from os import fdopen, remove
from database_manipulations import insertExperiment, insertAnalisysResult, \
    selectAllExperiment, selectAllAnalisysResult
from plot_graphic import plot_graphics

result_conf = []

def addSize(file):
    statinfo = os.stat(file)
    result_conf.append(statinfo.st_size)

def readFile(filename):
    with open(filename) as f:
        content = f.readlines()
    return content


def config():
    for a in readFile("data_input_conc.txt"):
        if "threads" in a or "blockSize" in a:
            a = a.replace('\n', "").split("=")
            result_conf.append(a[1])
    return result_conf


def results(path):
    r = 0
    result_arr = []
    while r != 5:
        tmp = []
        os.system(path)

        for i in readFile("result.txt"):
            i = i.replace(" ", "").replace('\n', "").split(":")
            if "TypeofConcurrency" in i:
                if len(result_conf) == 1:
                    continue
                else:
                    result_conf.append(i[1])
            else:
                tmp.append(i[1])
            open("result.txt", 'w').close()
        result_arr.append(tmp)
        r += 1
    return result_arr

def replace_config(file_path, file_name):
    #Create temp file
    fh, abs_path = mkstemp()
    with fdopen(fh,'w') as new_file:
        with open(file_path) as old_file:
            i = 0
            for line in old_file:
                if i == 1:
                    new_file.write('infile = "data_files/' + file_name + '"\n')
                else:
                    new_file.write(line)

                i += 1

    #Remove original file
    remove(file_path)
    #Move new file
    move(abs_path, file_path)


def run_program(execute_file, cursor, db):

    file_names = ['data_1MB.txt', 'data_2MB.txt', 'data_4MB.txt', 'data_8MB.txt', 'data_16MB.txt', 'data_32MB.txt',
                  'data_64MB.txt', 'data_128MB.txt', 'data_256MB.txt', 'data_512MB.txt']
    for file in file_names:

        replace_config('data_input_conc.txt', file)
        result = results(execute_file)
        addSize('data_files/' + file)
        insertExperiment(config(), cursor, db)
        insertAnalisysResult(result, cursor, db)

def main():
    # Open database connection
    db = MySQLdb.connect("localhost","root","oles0033","ConcurrencyResearch" )

    # prepare a cursor object using cursor() method
    cursor = db.cursor()

    path = "/home/oleksandr/CLIon/CLionProjects/cxx/gcc5/"


    #run_program(path + 'Async', cursor, db)


    selectAllExperiment(cursor)

    # selectAllAnalisysResult(cursor)
    plot_graphics([i for i in range(234, 244)],
                 [i for i in range(342, 352)],
                 [i for i in range(352, 362)],
                 cursor)

    db.close()

main()

