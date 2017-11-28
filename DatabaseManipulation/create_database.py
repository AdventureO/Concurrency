import MySQLdb

def create_database(name_od_database):
    """
    Create a database
    :param name_of_database: name for your database
    :return: nothing
    """
    db = MySQLdb.connect("localhost", "root", "oles0033")
    cursor = db.cursor()
    cursor.execute("CREATE DATABASE IF NOT EXISTS %s" %  name_od_database)
    db.close()
    print("DB created successfully")


def create_tables(name_of_database):
    """
    Create tables Experiment and Analisys in the database
    :param name_of_database: name of database
    :return: nothing
    """
    db = MySQLdb.connect("localhost", "root", "oles0033", name_of_database)
    cursor = db.cursor()

    experiment_table = """create table if not exists Experiment (
      id int unsigned not null auto_increment,
      type_of_concurrency varchar(100),
      file_size float,
      threads_number int,
      block_size int,
      compiler varchar(100),
      primary key(id)
    );"""

    analisys_result_table = """create table if not exists AnalisysResult (
      id int unsigned not null auto_increment,
      experiment_id int unsigned not null,
      wall_time int,
      process_total_time int,
      process_total_page_faults int,
      process_total_context_switches int,
      index experiment_index(experiment_id),
      foreign key (experiment_id) references Experiment(id) on delete cascade,
      primary key(id)
    );"""

    cursor.execute(experiment_table)
    cursor.execute(analisys_result_table)
    db.close()
    print("Tables created successfully")


def delete_all_data(name_of_database):
    """
    Delete all data from tables Experiment and Analisys
    :param name_of_database: name for your database
    :return: nothing
    """
    db = MySQLdb.connect("localhost", "root", "oles0033", name_of_database)
    cursor = db.cursor()

    sql = "DELETE FROM AnalisysResult"
    cursor.execute(sql)
    db.commit()

    sql = "DELETE FROM Experiment"
    cursor.execute(sql)
    db.commit()
    db.close()

    print("All data deleted successfully")

