import matplotlib.pyplot as plt
from database_manipulations import get_average_min
from average_deviation import calculate_all_st_deviation

def plot_graphics(result1, result2, result3 , cursor):

    byte = 1048576
    data_size = [0.5, 1, 2, 3, 4, 5, 8, 10, 16, 20, 30, 32, 40, 50, 64, 70, 80, 90, 100, 128, 256, 512]

    plot1 = [get_average_min(i, cursor)[0][0] / byte for i in result1]
    plot2 = [get_average_min(i, cursor)[0][0] / byte for i in result2]
    plot3 = [get_average_min(i, cursor)[0][0] / byte for i in result3]
    # plot4 = [get_average_min(i, cursor)[0][0] / byte for i in result4]

    error1 = calculate_all_st_deviation(result1, cursor)
    error2 = calculate_all_st_deviation(result2, cursor)
    error3 = calculate_all_st_deviation(result3, cursor)
    # error4 = calculate_all_st_deviation(result4, cursor)

    print(len(data_size))
    print(len(plot1))
    print(len(error1))
    plt.errorbar(data_size, plot1, error1, marker='^', label="Gcc5" )
    plt.errorbar(data_size, plot2, error2, marker='^', label="Gcc6" )
    plt.errorbar(data_size, plot3, error3, marker='^', label="Clang39" )
    # plt.errorbar(data_size, plot4, error4, marker='^', label="-O0" )

    # Place a legend to the right of this smaller subplot.
    plt.legend()
    plt.ylabel('time (s)/ 1 byte')
    plt.xlabel('Data size (Byte)')
    plt.title('Wall time')

    plt.savefig('test.svg', format='svg', dpi=1200)
    plt.show()
