# Program to multiply two matrices

def multiply_matrix(X, Y):

    columns_number = len(Y[0])
    rows_number = len(X)
    Y_rows = len(Y)
    result = [[0 for column in range(columns_number)] for row in range(rows_number)]

    # iterate through rows of X
    for i in range(rows_number):
       # iterate through columns of Y
       for j in range(columns_number):
           # iterate through rows of Y
           for k in range(Y_rows):
               result[i][j] += X[i][k] * Y[k][j]

    return result

# 3x3 matrix
X = [
    [12, 7, 3],
    [4, 5, 6],
    [7, 8, 9]
    ]

# 3x4 matrix
Y = [
    [5, 8, 1, 2],
    [6, 7, 3, 0],
    [4, 5, 9, 1]
    ]

result_matrix = multiply_matrix(X, Y)
# print result matrix
for row in result_matrix:
    print(row)