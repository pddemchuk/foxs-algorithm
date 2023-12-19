#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <mpi.h>


int main(int argc, char* argv[]) {
    int size = atoi(argv[1]);   // размер исходных матриц
    
    MPI_Init(&argc, &argv);
    
    int proc_rank, proc_size;
    MPI_Comm_size(MPI_COMM_WORLD, &proc_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

    int block_amt = sqrt(proc_size);    // вдоль одного измерения
    int block_size;
    if (size % block_amt == 0 && block_amt * block_amt == proc_size) {
        block_size = size / block_amt;
    } else {
        printf("неверный формат данных\n");
        exit(1);
    }
    int block_A[block_size][block_size], block_B[block_size][block_size], block_C[block_size][block_size];
    double t0;
    
    MPI_Comm GridComm;
    int dims[2] = {block_amt, block_amt};
    int periods[2] = {1, 1};
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &GridComm);

    int grid_rank;
    MPI_Comm_rank(GridComm, &grid_rank);
    int coords[2];
    MPI_Cart_coords(GridComm, grid_rank, 2, coords);

    // считывание матриц из файлов
    if  (proc_rank == 0) {
        FILE *A, *B;
        int a_ij, b_ij;
        A = fopen("matrix_A.txt", "r");
        B = fopen("matrix_B.txt", "r");
        int matrix_A[size][size], matrix_B[size][size];

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
        
        t0 = MPI_Wtime();

        // раздача блоков матриц в процессы
        int temp_block_A[block_size][block_size], temp_block_B[block_size][block_size];
        for (int ip = 0; ip < block_amt; ip++) {
            for (int jp = 0; jp < block_amt; jp++) {
                if (ip == coords[0] && jp == coords[1]) {
                    for (int i = 0; i < block_size; i++) {
                        for (int j = 0; j < block_size; j++) {
                            block_A[i][j] = matrix_A[i + ip * block_size][j + jp * block_size];
                            block_B[i][j] = matrix_B[i + ip * block_size][j + jp * block_size];
                        }
                    }
                } else {
                    for (int i = 0; i < block_size; i++) {
                        for (int j = 0; j < block_size; j++) {
                            temp_block_A[i][j] = matrix_A[i + ip * block_size][j + jp * block_size];
                            temp_block_B[i][j] = matrix_B[i + ip * block_size][j + jp * block_size];
                        }
                    }
                    MPI_Send(&(temp_block_A[0][0]), block_size * block_size, MPI_INT, ip * block_amt + jp, 0, GridComm);
                    MPI_Send(&(temp_block_B[0][0]), block_size * block_size, MPI_INT, ip * block_amt + jp, 1, GridComm);
                }
            }
        }

    }
    
    // получение блоков матриц
    if  (proc_rank != 0) {
        MPI_Recv(&(block_A[0][0]), block_size * block_size, MPI_INT, MPI_ANY_SOURCE, 0, GridComm, MPI_STATUS_IGNORE);
        MPI_Recv(&(block_B[0][0]), block_size * block_size, MPI_INT, MPI_ANY_SOURCE, 1, GridComm, MPI_STATUS_IGNORE);
    }

    MPI_Comm RowComm;
    int subdims[2] = {0, 1};
    MPI_Cart_sub(GridComm, subdims, &RowComm);

    MPI_Comm ColComm;
    subdims[0] = 1;
    subdims[1] = 0;
    MPI_Cart_sub(GridComm, subdims, &ColComm);

    int row_rank, col_rank;
    MPI_Comm_rank(RowComm, &row_rank);
    MPI_Comm_rank(ColComm, &col_rank);

    //printf("im %d grid proc, col: %d, row: %d\n", grid_rank, col_rank, row_rank);

    // инициализация алгоритма
    for (int i = 0; i < block_size; i++) {
        for (int j = 0; j < block_size; j++) {
            block_C[i][j] = 0;
        }
    }

    int temp_block_A[block_size][block_size];
    for (int i = 0; i < block_size; i++) {
        for (int j = 0; j < block_size; j++) {
            temp_block_A[i][j] = block_A[i][j];
        }
    }

    int recv_block_B[block_size][block_size];

    // алгоритм
    for (int stage = 0; stage < block_amt; stage++) {
        int ip = coords[0];
        int jp = (ip + stage) % block_amt;
        int p = ip * block_amt + jp;
        MPI_Bcast(&(block_A[0][0]), block_size * block_size, MPI_INT, jp, RowComm);
        int c_ij;
        for (int i = 0; i < block_size; i++) {
            for (int j = 0; j < block_size; j++) {
                c_ij = 0;
                for (int k = 0; k < block_size; k++){
                    c_ij += block_A[i][k] * block_B[k][j];
                }
                block_C[i][j] += c_ij;
            }
        }
        for (int i = 0; i < block_size; i++) {
            for (int j = 0; j < block_size; j++) {
                block_A[i][j] = temp_block_A[i][j];
            }
        }
        int source, dst;
        MPI_Request request;
        MPI_Status rec_status;
        MPI_Cart_shift(ColComm, 0, -1, &source, &dst);
        //printf("%d %d %d %d\n", proc_rank, col_rank, source, dst);
        MPI_Sendrecv(&(block_B[0][0]), block_size * block_size, MPI_INT, dst, 0, &(recv_block_B[0][0]), block_size * block_size, MPI_INT, source, 0, ColComm, MPI_STATUS_IGNORE);
        for (int i = 0; i < block_size; i++) {
            for (int j = 0; j < block_size; j++) {
                block_B[i][j] = recv_block_B[i][j];
            }
        }
    }

    if (proc_rank != 0) {
        MPI_Gather(&(block_C[0][0]), block_size * block_size, MPI_INT, NULL, block_size * block_size, MPI_INT, 0, MPI_COMM_WORLD);
    } else {
        int matrix_C[size * size];
        MPI_Gather(&(block_C[0][0]), block_size * block_size, MPI_INT, matrix_C, block_size * block_size, MPI_INT, 0, MPI_COMM_WORLD);

        printf("fox:\n");
        int k = 0;
        int count = 0;
        while (count != size * size) {
            printf("%d ", matrix_C[k]);
            count++;
            if (count % size != 0) {
                if (count % block_size != 0) {
                    k++;
                } else {
                    k += block_size * (block_size - 1) + 1;
                }
            } 
            else {
                printf("\n");
                if (count % (size * block_size) != 0) {
                    k -= (block_amt - 1) * block_size * block_size - 1;
                } else {
                    k ++;
                }
            }
        }

        double dt = MPI_Wtime() - t0;
	    printf("%d;%f\n", proc_size, dt);
    }

    MPI_Finalize();

    return 0;
}