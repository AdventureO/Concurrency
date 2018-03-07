# Inserts into Experiment database experiment parameters
def insert_experiment(list_of_arg, cursor, db):
    sql = """INSERT INTO Experiment (type_of_concurrency, file_size, threads_number, block_size, compiler)
    VALUES ('%s', '%f', '%d', '%d', '%s')""" %  (list_of_arg[0], float(list_of_arg[1]), int(list_of_arg[2]), int(list_of_arg[3]), list_of_arg[4])
    try:
        cursor.execute(sql)
        db.commit()
    except:
        # Rollback in case there is any error
        db.rollback()


# Select all experiments from Experiment table
def select_all_experiment(cursor):
    sql = "SELECT * FROM Experiment"
    experiments = []

    try:
        cursor.execute(sql)
        results = cursor.fetchall()

        for row in results:
            experiments.append(row)
            id = row[0]
            concurrency_method = row[1]
            file_size = row[2]
            threads_number = row[3]
            block_size = row[4]
            compiler = row[5]
            print(id, concurrency_method, file_size, threads_number, block_size, compiler)
    except:
      print("Error: unable to fetch data")

    return experiments


# Select all experiments from AnalisysResult table
def select_all_analysis_result(cursor):
    sql = "SELECT * FROM AnalisysResult"

    try:
        cursor.execute(sql)
        results = cursor.fetchall()

        for row in results:
            id = row[0]
            experiment_id = row[1]
            wall_time = row[2]
            process_total_time = row[3]
            process_total_page_faults = row[4]
            process_total_context_switches = row[5]
            print(id, experiment_id, wall_time, process_total_time, process_total_page_faults, process_total_context_switches)
    except:
        print("Error: unable to fetch data")


# Inserts result of program (analysis) into database
def insert_analysis_result(list_of_arg, cursor, db):
    sql = """SELECT * FROM Experiment WHERE id=(SELECT MAX(id) FROM Experiment);"""
    cursor.execute(sql)
    id = [row[0] for row in  cursor.fetchall()]
    print(id)
    for i in list_of_arg:
        sql = """INSERT INTO AnalisysResult (experiment_id, wall_time, process_total_time, process_total_page_faults,
               process_total_context_switches) VALUES ('%d', '%d', '%d', '%d', '%d')""" \
               % (int(id[0]), int(i[0]), int(i[1]), int(i[2]), int(i[3]))

        cursor.execute(sql)
        db.commit()


# Return the average and minimum value of experiment
def get_average_min(experiment_id, cursor):
    sql = "SELECT * FROM AnalisysResult WHERE experiment_id = " + str(experiment_id)

    wall_time = []
    process_total_time = []
    process_total_page_faults = []
    process_total_context_switches = []

    try:
        cursor.execute(sql)
        results = cursor.fetchall()

        for row in results:
            wall_time.append(row[2])
            process_total_time.append(row[3])
            process_total_page_faults.append(row[4])
            process_total_context_switches.append(row[5])
    except:
        print("Error: unable to fetch data")

    wall_time = [min(wall_time), sum(wall_time)/len(wall_time) ]
    process_total_time = [min(process_total_time), sum(process_total_time)/len(process_total_time) ]
    process_total_page_faults = [min(process_total_page_faults),
                                 sum(process_total_page_faults)/len(process_total_page_faults) ]
    process_total_context_switches = [min(process_total_context_switches),
                                      sum(process_total_context_switches)/len(process_total_context_switches)]

    print("Wall time min: {0}, average: {1}".format(wall_time[0], wall_time[1]))
    print("Process total time min: {0}, average: {1}".format(process_total_time[0], process_total_time[1]))
    print("Process total page faults min: {0}, average: {1}".format(process_total_page_faults[0],
                                                                    process_total_page_faults[1]))
    print("Process total context switches min: {0}, average: {1}".format(process_total_context_switches[0],
                                                                         process_total_context_switches[1]))
    print("------------------------------------------------------------------------------------------------------")

    return wall_time, process_total_time, process_total_page_faults, process_total_context_switches


# Get result of experiment by id
def get_results_experiment(experiment_id, cursor):
    sql = "SELECT * FROM AnalisysResult WHERE experiment_id = " + str(experiment_id)
    wall_time = []

    try:
        cursor.execute(sql)
        results = cursor.fetchall()

        for row in results:
            wall_time.append(row[2])
    except:
        print("Error: unable to fetch data")

    return wall_time
