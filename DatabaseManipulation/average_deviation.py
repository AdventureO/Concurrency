from database_manipulations import get_results_experiment

# Extract data from data base - all experiments result
def extract_data(result, cursor):
    byte = 1048576
    return [i / byte for i in get_results_experiment(result, cursor)]

# Calculate standart deviation for point (experiment)
def calculate_standart_deviation(result, cursor):
    data = extract_data(result, cursor)
    mean = sum(data)/len(data)
    diff_mean = [(i - mean)**2 for i in data]
    aver_dev = sum(diff_mean)/len(diff_mean)
    return aver_dev**0.5

# Calculate standart deviation for all points (experiments)
def calculate_all_st_deviation(result, cursor):
    return [calculate_standart_deviation(i, cursor) for i in result]