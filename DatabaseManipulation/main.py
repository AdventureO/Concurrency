import MySQLdb
import os
from tempfile import mkstemp
from shutil import move
from os import fdopen, remove
from database_manipulations import insertExperiment, insertAnalisysResult, \
    selectAllExperiment, selectAllAnalisysResult, get_results_experiment
from plot_graphic import plot_graphics
from average_deviation import calculate_standart_deviation, extract_data
from create_database import delete_all_data


def addSize(file, rs):
    statinfo = os.stat(file)
    rs.append(statinfo.st_size)
    return rs

def readFile(filename):
    with open(filename) as f:
        content = f.readlines()
    return content


def config(compiler, result_conf):
    for a in readFile("data_input_conc.txt"):
        if "threads" in a or "blockSize" in a:
            a = a.replace('\n', "").split("=")
            result_conf.append(a[1])

    result_conf.append(compiler)
    return result_conf


def results(path):
    result_conf = []
    r = 0
    result_arr = []
    while r != 10:
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
    return result_arr, result_conf

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


def run_program(execute_file, cursor, db, compiler):

    file_names = ['data_0.5MB.txt', 'data_1MB.txt', 'data_2MB.txt', 'data_3MB.txt',
                  'data_4MB.txt', 'data_5MB.txt', 'data_8MB.txt', 'data_10MB.txt',
                  'data_16MB.txt', 'data_20MB.txt', 'data_30MB.txt', 'data_32MB.txt', 'data_40MB.txt',
                  'data_50MB.txt', 'data_64MB.txt', 'data_70MB.txt', 'data_80MB.txt', 'data_90MB.txt',
                  'data_100MB.txt', 'data_128MB.txt', 'data_256MB.txt', 'data_512MB.txt']

    for file in file_names:
        replace_config('data_input_conc.txt', file)
        result, rs = results(execute_file)
        rs = addSize('data_files/' + file, rs)
        insertExperiment(config(compiler, rs), cursor, db)
        insertAnalisysResult(result, cursor, db)

def main():
    # Open database connection
    db = MySQLdb.connect("localhost","root","oles0033","MainDB" )

    # prepare a cursor object using cursor() method
    cursor = db.cursor()

    path = "/home/oleksandr/CLIon/CLionProjects/cxx/"

    # types = ["ConditionalQueue"]
    # folders = ['gcc5_1', 'gcc5_2', 'gcc5_3', 'gcc6_1', 'gcc6_2', 'gcc6_3', 'clang39_1', 'clang39_2', 'clang39_3']
    #
    # for compiler in folders:
    #     for type in types:
    #         run_program(path + compiler + '/' + type, cursor, db, compiler)
    # selectAllExperiment(cursor)
    # selectAllAnalisysResult(cursor)


    plot_graphics([i for i in range(1109, 1131)],
                 [i for i in range(1175, 1197)],
                 [i for i in range(1241, 1263)],
                 cursor)

    # plot_graphics([i for i in range(362, 372)], cursor)
    db.close()

main()


