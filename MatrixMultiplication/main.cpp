#include <iostream>
using namespace std;

void multiply_matrix(int X[][3], int Y[][4]) {

    size_t columns_number = sizeof(Y[0])/sizeof(Y[0][0]);
    size_t rows_number = sizeof(X)/sizeof(*X[0]);
    size_t Y_rows = sizeof(Y)/sizeof(*Y[0]);
    int result[rows_number][columns_number];

    cout << columns_number << endl;
    cout << rows_number << endl;
    cout << Y_rows << endl;


    // Initializing elements of result matrix
    for(int i = 0; i < rows_number; ++i)
        for(int j = 0; j < columns_number; ++j) {
            result[i][j]=0;
        }

    // Multiplying matrix X and Y and store result data to result matrix
    for(int i = 0; i < rows_number; ++i)
        for(int j = 0; j < columns_number; ++j)
            for(int k = 0; k < Y_rows; ++k) {
                result[i][j] += X[i][k] * Y[k][j];
            }

    // Displaying the multiplication of two matrix.
    cout << endl << "Output Matrix: " << endl;
    for(int i = 0; i < rows_number; ++i)
        for(int j = 0; j < columns_number; ++j) {
            cout << " " << result[i][j];
            if(j == columns_number-1)
                cout << endl;
        }

}

int main() {
    int X[3][3] = {{12, 7, 3},
                   {4, 5, 6},
                   {7, 8, 9}};

    int Y[3][4] = {{5, 8, 1, 2},
                   {6, 7, 3, 0},
                   {4, 5, 9, 1}};

    multiply_matrix(X, Y);
    return 0;
}

