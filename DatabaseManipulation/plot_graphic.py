import matplotlib.pyplot as plt
from database_manipulations import get_average_min

def plot_graphics(result1, result2, result3, cursor):

    byte = 1048576
    data_size = [1048576, 2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728, 268435456, 536870912]

    plot1 = [get_average_min(i, cursor)[0][0] / byte for i in result1]
    plot2 = [get_average_min(i, cursor)[0][0] / byte for i in result2]
    plot3 = [get_average_min(i, cursor)[0][0] / byte for i in result3]

    plt.plot(data_size, plot1, 'ro', label="-O3")
    plt.plot(data_size, plot2, 'r', label="-O2")
    plt.plot(data_size, plot3, 'g', label="-O1")

    # Place a legend to the right of this smaller subplot.
    plt.legend()
    plt.ylabel('time (s)/ 1 byte')
    plt.xlabel('Data size (Byte)')
    plt.title('Wall time')

    plt.savefig('FuturePromiseGCC5Optimization.svg', format='svg', dpi=1200)

    plt.show()
