#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <galois.h>
#include <gf_complete.h>
#include <jerasure.h>
#include "rotatedrs.h"

int qpow(int a, int b) {
    int res = 1;
    while(b){
        if(b & 1)res = res * a;
        a = a * a; b /= 2;
    }
    return res;
}


// m = 3
void rotatedrs_encode(int k, int m, int w, char** data_ptrs, char** coding_ptrs, int size) {
    int r = 4; // 令r = 4
    int blocksize = size / r;
    int *p = (int *)malloc(sizeof(int)*(k+m+2));
    for(int i=0; i<=(k+m+1); i++) {
        p[i] = qpow(2, i);
    }

    char * mult_data = (char*)malloc(sizeof(char)*blocksize);
    
    for(int j=0;j<m;j++) {
        for(int b=0;b<r;b++) {
            for(int i=0;i<(k*j)/m;i++) {
                int y = (b+1)%r;
                // printf("here: %p, %d\n", &data_ptrs[i], y*blocksize);
                galois_w08_region_multiply(data_ptrs[i]+y*blocksize, p[i+j], blocksize, mult_data, 0); 
                galois_region_xor (mult_data, coding_ptrs[j]+b*blocksize, blocksize); 
            }  
            for(int i=(k*j)/m;i<k;i++) {
                int y = b;
                galois_w08_region_multiply(data_ptrs[i]+y*blocksize, p[i+j], blocksize, mult_data, 0); 
                galois_region_xor (mult_data, coding_ptrs[j]+b*blocksize, blocksize); 
            }
        }
    }
    free(mult_data);
    free(p);
}

int rotatedrs_decode(int k, int m, int w, int* erasures, char **data_ptrs, char** coding_ptrs, int size) {
    // blocksize = 16MB;
    // buffersize = k * m * 16MB
    // single filesize = m * 16MB = buffersize / k
    int erased = -1;
    for(int i=0;i<k+m;i++) {
       if(erasures[i] == -1) {
            break;
        }
        erased = erasures[i];
    }
    if(erased == -1) {
        return 1;
    }
    printf("here am I 1\n");
    int *matrix = rotatedrs_coding_matrix(k, m, w);
    if(erased < k) {
        printf("here am I 2\n");
        rotatedrs_data_decode(k, m, w, matrix, erased, data_ptrs, coding_ptrs, size);
    } else {
        printf("here am I 3\n");
        rotatedrs_coding_decode(k, m, w, erased, data_ptrs, coding_ptrs, size);
    }
    printf("here am I 4\n");
    return 1;
}

int* rotatedrs_coding_matrix(int k, int m, int w) {
    if (w != 8 && w != 16 && w != 32) return NULL;
    
    int *p = (int *)malloc(sizeof(int)*(k+m+2));
    for(int i=0; i<=(k+m+2); i++) {
        p[i] = qpow(2, i);
    }

    int* matrix = (int*)malloc(sizeof(int)*(m*k)); 
    if (matrix == NULL) return NULL;

    for(int i=0;i<m;i++) {
        for(int j=0;j<k;j++) {
            matrix[i*k+j] = p[i+j];
        }
    }
    free(p);
    return matrix;
}

void rotatedrs_data_decode(int k, int m, int w, int *matrix, int erased, char** data_ptrs, char** coding_ptrs, int size) {
    int r = 4; // 令r = 4
    int blocksize = size / r;
    char ** data = (char **)malloc(sizeof(char *)*k);
    char ** coding = (char **)malloc(sizeof(char *)*m);
    // 传入jerasure_make_decoding_matrix的参数
    int* erasures = (int*)malloc(sizeof(int)*(k+m));

    for(int i=0;i<k+m;i++) {
        if(i == erased) {
            erasures[i] = 1;
        } else {
            erasures[i] = 0;
        }
    }
    int *dm_ids = (int*)malloc(sizeof(int)*k);
    int *decoding_matrix = (int*)malloc(sizeof(int)*(k*k));
    jerasure_make_decoding_matrix(k, m, w, matrix, erasures, decoding_matrix, dm_ids);

    for(int i=0;i<r;i++) {
        for(int j=0;j<k;j++) {
            data[j] = (char*)malloc(sizeof(char)*blocksize);
            memcpy(data[j], data_ptrs[j]+i*blocksize, blocksize);
        }
        for(int j=0;j<m;j++) {
            coding[j] = (char*)malloc(sizeof(char)*blocksize);
            memcpy(coding[j], coding_ptrs[j]+i*blocksize, blocksize);
        }
        jerasure_matrix_dotprod(k, w, decoding_matrix+(erased*k), dm_ids, erased, data, coding, blocksize);
        for(int j=0;j<k;j++) {
            memcpy(data_ptrs[j]+i*blocksize, data[j], blocksize);
        }
        for(int j=0;j<m;j++) {
            memcpy(coding_ptrs[j]+i*blocksize, coding[j], blocksize);
        }
    }

    // free(erasures);
    // free(dm_ids);
    // free(decoding_matrix);
}

void rotatedrs_coding_decode(int k, int m, int w, int erased, char** data_ptrs, char** coding_ptrs, int size) {
    int r = 4; // 令r = 4
    int blocksize = size / r;

    int *p = (int *)malloc(sizeof(int)*(k+m+2));
    for(int i=0; i<(k+m+2); i++) {
        p[i] = qpow(2, i);
    }

    char * mult_data = (char*)malloc(sizeof(char)*blocksize);
    
    // 为erased coding block赋值
    int j = erased - k;
    // int j = erased;
    for(int b=0;b<r;b++) {
        for(int i=0;i<(k*j)/m;i++) {
            int y = (b+1)%r;
            galois_w08_region_multiply(data_ptrs[i]+y*blocksize, p[i+j], blocksize, mult_data, 0); 
            galois_region_xor (mult_data, coding_ptrs[j]+b*blocksize, blocksize); 
        }  
        for(int i=(k*j)/m;i<k;i++) {
            int y = b;
            galois_w08_region_multiply(data_ptrs[i]+y*blocksize, p[i+j], blocksize, mult_data, 0); 
            galois_region_xor (mult_data, coding_ptrs[j]+b*blocksize, blocksize); 
        }
    }
    free(mult_data);
    free(p);
}