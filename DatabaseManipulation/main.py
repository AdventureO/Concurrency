import MySQLdb
import os

result_conf = []

def addSize():
    statinfo = os.stat('data.txt')
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


def results():
    r = 0
    result_arr = []
    while r != 5:
        tmp = []
        os.system("/home/oleksandr/PyCharm/PycharmProjects/AILab/RawThreads")

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


# Inserts into Experiment database experiment parameters
def insertExperiment(list_of_arg, cursor, db):
    sql = """INSERT INTO Experiment (type_of_concurrency, file_size, threads_number, block_size)
    VALUES ('%s', '%f', '%d', '%d' )""" %  (list_of_arg[0], float(list_of_arg[1]), int(list_of_arg[2]), int(list_of_arg[3]))
    try:
       cursor.execute(sql)
       db.commit()
    except:
       # Rollback in case there is any error
       db.rollback()

# Select all experiments from Experiment database
def selectAllExperiment(cursor):
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
         print(id, concurrency_method, file_size, threads_number, block_size)
   except:
      print("Error: unable to fetch data")

   return experiments
# Select all experiments from AnalisysResult database
def selectAllAnalisysResult(cursor):
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

# Inserts result of program (analisys) into database
def insertAnalisysResult(list_of_arg, cursor, db):
   sql = """SELECT * FROM Experiment WHERE id=(SELECT MAX(id) FROM Experiment);"""
   cursor.execute(sql)
   id = [row[0] for row in  cursor.fetchall()]

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

    print("Wall time min: {0}, average: {1}".format(min(wall_time), sum(wall_time)/len(wall_time)))
    print("Process total time min: {0}, average: {1}".format(min(process_total_time), sum(process_total_time)/len(process_total_time)))
    print("Process total page faults min: {0}, average: {1}".format(min(process_total_page_faults), sum(process_total_page_faults)/len(process_total_page_faults)))
    print("Process total context switches min: {0}, average: {1}".format(min(process_total_context_switches), sum(process_total_context_switches)/len(process_total_context_switches)))


# def get_all_average_min(type_of_concurrency, cursor):
#
#     sql = "SELECT id FROM Experiment WHERE type_of_concurrency =" + str(type_of_concurrency)
#     experiments = selectAllExperiment(cursor)
#
#     try:
#     cursor.execute(sql)
#     results = cursor.fetchall()
#
#     #     print(results())
#     #     for row in results:
#     #         print(row)
#     #         experiments_ids.append(row[0])
#     #
#     # except:
#     #     print("Error: unable to fetch data")
# Connects to database and makes some manipulations
def main():
   # Open database connection
   db = MySQLdb.connect("localhost","root","oles0033","ConcurrencyResearch" )

   # prepare a cursor object using cursor() method
   cursor = db.cursor()

   # a = config()
   # b = results()
   # insertExperiment(a, cursor, db)
   # selectAllExperiment(cursor)
   # insertAnalisysResult(b, cursor, db)

   a = results()
   addSize()
   #
   insertExperiment(config(), cursor, db)
   insertAnalisysResult(a, cursor, db)
   # selectAllAnalisysResult(cursor)
   # #get_average_min(16, cursor)
   #
   selectAllExperiment(cursor)
   print("==========================")
   selectAllAnalisysResult(cursor)
   print("==========================")
   get_average_min(18, cursor)
   # get_all_average_min("RawThreads", cursor)
   #(cursor, db)
   # disconnect from server
   db.close()


main()


# def deleteAllData():
#    sql = "DELETE FROM AnalisysResult"
#    cursor.execute(sql)
#    db.commit()
#
#    sql = "DELETE FROM Experiment"
#    cursor.execute(sql)
#    db.commit()

# Delete tables
# sql = """Drop table if exists Experiment"""
# cursor.execute(sql)

# sql = """Drop table if exists AnalisysResult"""
# cursor.execute(sql)

# Create tables
# experiment_table = """create table if not exists Experiment (
#   id int unsigned not null auto_increment,
#   type_of_concurrency varchar(100),
#   file_size float,
#   threads_number int,
#   block_size int,
#   primary key(id)
# );"""
#
# analisys_result_table = """create table if not exists AnalisysResult (
#   id int unsigned not null auto_increment,
#   experiment_id int unsigned not null,
#   wall_time int,
#   process_total_time int,
#   process_total_page_faults int,
#   process_total_context_switches int,
#   index experiment_index(experiment_id),
#   foreign key (experiment_id) references Experiment(id) on delete cascade,
#   primary key(id)
# );"""
#
# cursor.execute(experiment_table)
# cursor.execute(analisys_result_table)

#Create Database
#cursor.execute("CREATE DATABASE IF NOT EXISTS ConcurrencyResearch")
#print("DB created successfully")