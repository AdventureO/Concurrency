import os
from tempfile import mkstemp
from shutil import move
from os import fdopen, remove


def add_file_size(file, rs):
    stat_info = os.stat(file)
    rs.append(stat_info.st_size)
    return rs


def read_file(filename):
    with open(filename) as f:
        content = f.readlines()
    return content


def config(compiler, result_conf):
    for a in read_file("data_input_conc.txt"):
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

        for i in read_file("result.txt"):
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
    # Create temporary file
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

    # Remove original file
    remove(file_path)

    # Move new file
    move(abs_path, file_path)
