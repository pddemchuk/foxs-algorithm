#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main(int argc, char* argv[]) {

    FILE *A, *B;
    int size = atoi(argv[1]);
    int a_ij, b_ij, c_ij;
    A = fopen("matrix_A.txt", "r");
    B = fopen("matrix_B.txt", "r");
    int matrix_A[size][size], matrix_B[size][size], matrix_C[size][size];

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fscanf(A, "%d", &a_ij);
            fscanf(B, "%d", &b_ij);
            matrix_A[i][j] = a_ij;
            matrix_B[i][j] = b_ij;
        }
    }

    fclose(A);
    fclose(B);

    double t0 = 0.0;

    // умножение матриц
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            c_ij = 0;
            for (int k = 0; k < size; k++) {
                c_ij += matrix_A[i][k] * matrix_B[k][j];
            }
            matrix_C[i][j] = c_ij;
        }
    }

    printf("sequential:\n");
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%d ", matrix_C[i][j]);
        }
        printf("\n");
    }

    
    double dt = (double)( clock() - t0) / CLOCKS_PER_SEC;
	printf("%f\n", dt);

    return 0;
}