/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    /* Version 1 for 32 * 32: fail.
     * 8 is the best block size to try, since the b = 5 (32 bytes).
     * However, it produces 343 misses for 32 * 32 matrix.
     * diagonal element of A and B share same set address.
    int i, j, bi, bj, tmp;
    int block = 8;
    for (i = 0; i < N; i += block) {
        for (j = 0; j < M; j += block) {
            for (bi = i; bi < N && bi < (i+block); bi++) {
                for (bj = j; bj < M && bj < (j+block); bj++) {
                    B[bj][bi] = A[bi][bj];
                }
            }
        }
    }
    */

    /* Version 2 for 32 * 32: valid
    // 287 Miss，eliminate the diagonal miss except 
    // when next row of A come in in the block, it will evict the corresponding row of B
    int i, j, k;
    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 8) {
            for (k = i; k < i + 8; k++) { // Block size 8
                int tmp1 = A[k][j+0], tmp2 = A[k][j+1], tmp3 = A[k][j+2], tmp4 = A[k][j+3];
                int tmp5 = A[k][j+4], tmp6 = A[k][j+5], tmp7 = A[k][j+6], tmp8 = A[k][j+7];

                B[j+0][i] = tmp1;
                B[j+1][i] = tmp2;
                B[j+2][i] = tmp3;
                B[j+3][i] = tmp4;
                B[j+4][i] = tmp5;
                B[j+5][i] = tmp6;
                B[j+6][i] = tmp7;
                B[j+7][i] = tmp8;
            }
        }
    }
    */

   // Version 3 for 32 * 32: valid
   // 259 misses
    if (N == 32) {
        int i, j, bi, bj;
        int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
        int block = 8; // Can be moved
        for (i = 0; i < N; i += block) {
            for (j = 0; j < M; j += block) {
                for (bi = i, bj = j; bi < i + block; bi++, bj++) {
                    // copy A into cooresponding transpose position of B
                    tmp0 = A[bi][j];
                    tmp1 = A[bi][j+1];
                    tmp2 = A[bi][j+2];
                    tmp3 = A[bi][j+3];
                    tmp4 = A[bi][j+4];
                    tmp5 = A[bi][j+5];
                    tmp6 = A[bi][j+6];
                    tmp7 = A[bi][j+7];

                    B[bj][i] = tmp0;
                    B[bj][i+1] = tmp1;
                    B[bj][i+2] = tmp2;
                    B[bj][i+3] = tmp3;
                    B[bj][i+4] = tmp4;
                    B[bj][i+5] = tmp5;
                    B[bj][i+6] = tmp6;
                    B[bj][i+7] = tmp7;
                }
                // Transpose B only in matrix B, completely eliminate all the diagonal miss
                // If we do so, the diagonal miss occurs in last version disappears.
                for(bi = 0; bi < block; bi++) {
                    for(bj = bi + 1; bj < block; bj++){
                        tmp0 = B[bi+j][bj+i];
                        B[bi+j][bj+i] = B[bj+j][bi+i];
                        B[bj+j][bi+i] = tmp0;
                    }
                }
            }
        }
    }
   
    // Version 4 for 64 * 64: end up with 1027 misses
    // Version 3 does not work for 64 * 64, since we only have 32 sets.
    // Remember that the order we traverse 64 * 64, row from 0 to 63.
    // So 4 rows of 64 will occupy all the 32 sets(4 * 64 / 8 = 32)
    // The next 4 rows will share the same address with the last 4, that's where the misses come in.
    // For each block size of 8, we need to deal with the upper 4 rows first, and then lower 4 rows.
    // Reference: https://zhuanlan.zhihu.com/p/387662272
    if (N == 64) {
        int i, j, m, n;
        int t0, t1, t2, t3, t4, t5, t6, t7;
        for (i = 0; i < N; i += 8) {
            // deal with diagonal sub matrix
            // Do not deal with the diagonal matrix will return 1667 misses
    
            // copy lower 4 rows of A in some empty space of B
            if (i == 0) j = 8; else j = 0; // use the first access block after diagonal transpose
            for (m = 0; m < 4; m++) {
                t0 = A[i+4+m][i+0];
                t1 = A[i+4+m][i+1];
                t2 = A[i+4+m][i+2];
                t3 = A[i+4+m][i+3];
                t4 = A[i+4+m][i+4];
                t5 = A[i+4+m][i+5];
                t6 = A[i+4+m][i+6];
                t7 = A[i+4+m][i+7];

                B[i+m][j+0] = t0;
                B[i+m][j+1] = t1;
                B[i+m][j+2] = t2;
                B[i+m][j+3] = t3;
                B[i+m][j+4] = t4;
                B[i+m][j+5] = t5;
                B[i+m][j+6] = t6;
                B[i+m][j+7] = t7;
            }

            // transpose the copy block by each 4*4
            for (m = 0; m < 4; m++) {
                for (n = m + 1; n < 4; n++) {
                    t0 = B[i+m][j+n];
                    B[i+m][j+n] = B[i+n][j+m];
                    B[i+n][j+m] = t0;

                    t1 = B[i+m][j+n+4];
                    B[i+m][j+n+4] = B[i+n][j+m+4];
                    B[i+n][j+m+4] = t1;
                }
            }

            // copy upper 4 rows of A into B
            for (m = 0; m < 4; m++) {
                t0 = A[i+m][i+0];
                t1 = A[i+m][i+1];
                t2 = A[i+m][i+2];
                t3 = A[i+m][i+3];
                t4 = A[i+m][i+4];
                t5 = A[i+m][i+5];
                t6 = A[i+m][i+6];
                t7 = A[i+m][i+7];

                B[i+m][i+0] = t0;
                B[i+m][i+1] = t1;
                B[i+m][i+2] = t2;
                B[i+m][i+3] = t3;
                B[i+m][i+4] = t4;
                B[i+m][i+5] = t5;
                B[i+m][i+6] = t6;
                B[i+m][i+7] = t7;
            }

            // transpose lower 4 rows of B by each 4*4
            for (m = 0; m < 4; m++) {
                for (n = m + 1; n < 4; n++) {
                    t0 = B[i+m][i+n];
                    B[i+m][i+n] = B[i+n][i+m];
                    B[i+n][i+m] = t0;

                    t1 = B[i+m][i+n+4];
                    B[i+m][i+n+4] = B[i+n][i+m+4];
                    B[i+n][i+m+4] = t1;
                }
            }

            // swap the upper right with lower left
            for (m = 0; m < 4; m++) {
                t0 = B[i+m][i+4];
                t1 = B[i+m][i+5];
                t2 = B[i+m][i+6];
                t3 = B[i+m][i+7];
    
                B[i+m][i+4] = B[i+m][j+0];
                B[i+m][i+5] = B[i+m][j+1];
                B[i+m][i+6] = B[i+m][j+2];
                B[i+m][i+7] = B[i+m][j+3];

                B[i+m][j+0] = t0;
                B[i+m][j+1] = t1;
                B[i+m][j+2] = t2;
                B[i+m][j+3] = t3;
            }
            
            // copy lower rows into corresponding area
            for (int m = 0; m < 4; m++) {
                B[i+4+m][i+0] = B[i+m][j+0];
                B[i+4+m][i+1] = B[i+m][j+1];
                B[i+4+m][i+2] = B[i+m][j+2];
                B[i+4+m][i+3] = B[i+m][j+3];
                B[i+4+m][i+4] = B[i+m][j+4];
                B[i+4+m][i+5] = B[i+m][j+5];
                B[i+4+m][i+6] = B[i+m][j+6];
                B[i+4+m][i+7] = B[i+m][j+7];
            }

            for (j = 0; j < M; j += 8) {
                // use j as index i, [j, i]
                if (i == j) continue;
                // left and right upper corner
                for (m = 0; m < 4; m++) {
                    // copy row m of block A
                    t0 = A[j+m][i+0];
                    t1 = A[j+m][i+1];
                    t2 = A[j+m][i+2];
                    t3 = A[j+m][i+3];
                    t4 = A[j+m][i+4];
                    t5 = A[j+m][i+5];
                    t6 = A[j+m][i+6];
                    t7 = A[j+m][i+7];

                    // transpose 4 elements into column m
                    B[i+0][j+m] = t0;
                    B[i+1][j+m] = t1;
                    B[i+2][j+m] = t2;
                    B[i+3][j+m] = t3;
                    // transpose 4 elements into column m + 4
                    B[i+0][j+m+4] = t4;
                    B[i+1][j+m+4] = t5;
                    B[i+2][j+m+4] = t6;
                    B[i+3][j+m+4] = t7;
                }
                // left lower corner of A and right upper corner of B
                for (m = 0; m < 4; m++) {
                    // copy column m of A
                    t0 = A[j+4][i+m];
                    t1 = A[j+5][i+m];
                    t2 = A[j+6][i+m];
                    t3 = A[j+7][i+m];
                    // copy row m of B
                    t4 = B[i+m][j+4];
                    t5 = B[i+m][j+5];
                    t6 = B[i+m][j+6];
                    t7 = B[i+m][j+7];
                    // move copy of A into right upper corner of B
                    B[i+m][j+4] = t0;
                    B[i+m][j+5] = t1;
                    B[i+m][j+6] = t2;
                    B[i+m][j+7] = t3;
                    // copy row m from right upper corner to left lower corner of B
                    B[i+4+m][j+0] = t4;
                    B[i+4+m][j+1] = t5;
                    B[i+4+m][j+2] = t6;
                    B[i+4+m][j+3] = t7;
                    
                }
                // right lower corner
                for (m = 4; m < 8; m++) {
                    // copy row m of A
                    t0 = A[j+m][i+4];
                    t1 = A[j+m][i+5];
                    t2 = A[j+m][i+6];
                    t3 = A[j+m][i+7];

                    B[i+4][j+m] = t0;
                    B[i+5][j+m] = t1;
                    B[i+6][j+m] = t2;
                    B[i+7][j+m] = t3;
                }
            }
        }
    }

    // Version 5 for 61 * 67
    // Same method as 32 * 32, but use diff block.
    // 1847 misses.
    if (N == 67) {
        int i, j, bi, bj;
        int t0, t1, t2, t3, t4, t5, t6, t7;
        int b_row = 8, b_col = 16; // Can be moved
        for (i = 0; i < N; i += b_row) {
            for (j = 0; j < M; j += b_col) {
                if (i + b_row <= N) {
                    for (bj = 0; bj < b_col; bj++) {
                        t0 = A[i+0][bj+j];
                        t1 = A[i+1][bj+j];
                        t2 = A[i+2][bj+j];
                        t3 = A[i+3][bj+j];
                        t4 = A[i+4][bj+j];
                        t5 = A[i+5][bj+j];
                        t6 = A[i+6][bj+j];
                        t7 = A[i+7][bj+j];

                        B[bj+j][i+0] = t0;
                        B[bj+j][i+1] = t1;
                        B[bj+j][i+2] = t2;
                        B[bj+j][i+3] = t3;
                        B[bj+j][i+4] = t4;
                        B[bj+j][i+5] = t5;
                        B[bj+j][i+6] = t6;
                        B[bj+j][i+7] = t7;
                    }
                } else {
                    for (bi = i; bi < N && bi < i + b_row; bi++) {
                        for (bj = j; bj < M && bj < j + b_col; bj++) {
                            B[bj][bi] = A[bi][bj];
                        } 
                    }
                }
            }
        }
    }
    
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

