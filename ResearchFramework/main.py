import MySQLdb
from database_manipulations import insert_experiment, insert_analysis_result, \
    select_all_experiment, select_all_analysis_result, get_results_experiment, get_average_min
from plot_graphic import plot_graphics
from standard_deviation import calculate_standard_deviation, extract_data
from create_database import delete_all_data
from configuration import replace_config, results, add_file_size, config


def run_program(execute_file, cursor, db, compiler):

    file_names = ['data_0.5MB.txt', 'data_1MB.txt', 'data_2MB.txt', 'data_3MB.txt',
                  'data_4MB.txt', 'data_5MB.txt', 'data_8MB.txt', 'data_10MB.txt',
                  'data_16MB.txt', 'data_20MB.txt', 'data_30MB.txt', 'data_32MB.txt', 'data_40MB.txt',
                  'data_50MB.txt', 'data_64MB.txt', 'data_70MB.txt', 'data_80MB.txt', 'data_90MB.txt',
                  'data_100MB.txt', 'data_128MB.txt', 'data_256MB.txt', 'data_512MB.txt']

    for file in file_names:
        replace_config('data_input_conc.txt', file)
        result, rs = results(execute_file)
        rs = add_file_size('data_files/' + file, rs)
        insert_experiment(config(compiler, rs), cursor, db)
        insert_analysis_result(result, cursor, db)


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


    # plot_graphics([i for i in range(1109, 1131)],
    #              [i for i in range(1175, 1197)],
    #              [i for i in range(1241, 1263)],
    #              cursor)

    get_average_min(345, cursor)
    # plot_graphics([i for i in range(362, 372)], cursor)
    db.close()

main()


