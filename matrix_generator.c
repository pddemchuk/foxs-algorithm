#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main(int argc, char* argv[]) {

    int size = atoi(argv[1]);
    FILE *A, *B;
    int a_ij, b_ij;
    A = fopen("matrix_A.txt", "w");
    B = fopen("matrix_B.txt", "w");

    srand(time(NULL));

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            a_ij = rand() % 10;
            fprintf(A, "%d ", a_ij);
            b_ij = rand() % 10;
            fprintf(B, "%d ", b_ij);
        }
        fprintf(A, "\n");
        fprintf(B, "\n");
    }

    fclose(A);
    fclose(B);
    
    return 0;
}