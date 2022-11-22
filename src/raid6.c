#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <galois.h>
#include <gf_complete.h>
#include "raid6.h"

// (P+Q), m = 2
void raid6_encode(int k, int m, int w, int* matrix, char **data_ptrs, char **coding_ptrs, int size) {
    int blocksize = size/k; // size of one single block   

    char ** temp = (char **)malloc(sizeof(char*) * (m+k));
    for(int i=0;i<(m+k);i++) {
        temp[i] = (char *)malloc(sizeof(char) * blocksize);
    }
    for(int r=0; r<k; r++) {
        char *p_coding = temp[k - r];
        char *q_coding = temp[k - r + 1];
        char *data = (char *)malloc(sizeof(char)*blocksize);

        for(int i=0;i<k;i++) { // i表示文件序号
            int data_index = i < k - r ? i : i + m;
            memcpy(data, data_ptrs[i] + blocksize * r, blocksize);
            memcpy(temp[data_index], data, blocksize); // 保存data block数据, temp[data_index]保持不变
            if(i == 0) { // 初始赋值
                // p = d0
                memcpy(p_coding, data, blocksize);
                // q = k0 * d0
                galois_w08_region_multiply(data, matrix[k+i], blocksize, data, 0); 
                memcpy(q_coding, data, blocksize);
                continue;
            } 
            // p = d0 ^ d1 ^ d2 ^ d3
            galois_region_xor(data, p_coding, blocksize); 
            // q = (k0*d0) ^ (k1*d1) ^ (k2*d2) ^ (k3*d3)
            galois_w08_region_multiply(data, matrix[k+i], blocksize, data, 0); // add表示目标块是否已初始化
            galois_region_xor (data, q_coding, blocksize); 
        }
        for(int i=0; i<k; i++) {
            memcpy(data_ptrs[i] + r * blocksize, temp[i], blocksize);
        }
        for(int i=k; i<m+k; i++) {
            memcpy(coding_ptrs[i - k] + r * blocksize, temp[i], blocksize);
        }
    }
}



int raid6_decode(int k, int m, int w, int* matrix, int* erasures, char **data_ptrs, char** coding_ptrs, int size) {
    int erased = -1;
    for(int i = 0; i<k+m; i++) {
        if(erasures[i] == -1) {
            break;
        }
        erased = erasures[i];
    }
    if(erased == -1) {
        return 1;
    }
    for(int r = 0; r<k; r++) {
        int p_index = k - r;
        if(erased == p_index) {
            raid6_p_coding_encode(k, m, w, r, data_ptrs, coding_ptrs, size);
        } else if(erased == p_index + 1) {
            raid6_q_coding_encode(k, m, w, r, matrix, data_ptrs, coding_ptrs, size);
        } else {
            raid6_data_encode(k, m, w, r, erased, data_ptrs, coding_ptrs, size);
        }
    }
    return 1;
}

void raid6_data_encode(int k, int m, int w, int r, int erased, char **data_ptrs, char** coding_ptrs, int size) {
    int blocksize = size/k;
    char *p_coding = (char *)malloc(sizeof(char)*blocksize);
    char *data = (char *)malloc(sizeof(char)*blocksize);

    int p_index = k - r;

    if(p_index < k) {
        memcpy(p_coding, data_ptrs[p_index] + r * blocksize, blocksize);
    } else {
        memcpy(p_coding, coding_ptrs[p_index - k] + r * blocksize, blocksize);
    }

    for(int i=0; i<k+m; i++) {
        if(i == p_index || i == p_index + 1 || i == erased) {
            continue;
        }
        if(i < k) {
            memcpy(data, data_ptrs[i] + blocksize * r, blocksize);
        } else {
            memcpy(data, coding_ptrs[i-k] + blocksize * r, blocksize);
        }
        galois_region_xor(data, p_coding, blocksize); 
    }

    if(erased < k) {
        memcpy(data_ptrs[erased] + r * blocksize, p_coding, blocksize);
    } else {
        memcpy(coding_ptrs[erased - k] + r * blocksize, p_coding, blocksize);
    }
    return;
}

void raid6_p_coding_encode(int k, int m, int w, int r, char **data_ptrs, char **coding_ptrs, int size) {
    // int blocksize = size/k;
    int blocksize = size/k;
    char *p_coding = (char *)malloc(sizeof(char)*blocksize);
    char *data = (char *)malloc(sizeof(char)*blocksize);

    memcpy(data, data_ptrs[0] + blocksize * r, blocksize);
    memcpy(p_coding, data, blocksize);

    int p_index = k - r;
    for(int i=1; i<k+m; i++) {
        if(i == p_index || i == p_index + 1) {
            continue;
        }
        if(i < k) {
            memcpy(data, data_ptrs[i] + blocksize * r, blocksize);
        } else {
            memcpy(data, coding_ptrs[i-k] + blocksize * r, blocksize);
        }
        galois_region_xor(data, p_coding, blocksize); 
    }
    if(p_index < k) {
        memcpy(data_ptrs[p_index] + r * blocksize, p_coding, blocksize);
    } else {
        memcpy(coding_ptrs[p_index - k] + r * blocksize, p_coding, blocksize);
    }
    // printf("p coding, blocksize: %d\n", blocksize);
}

void raid6_q_coding_encode(int k, int m, int w, int r, int* matrix, char **data_ptrs, char **coding_ptrs, int size) {
    int blocksize = size/k;
    char *q_coding  = (char *)malloc(sizeof(char)*blocksize);
    char *data      = (char *)malloc(sizeof(char)*blocksize);

    memcpy(data, data_ptrs[0] + blocksize * r, blocksize);
    galois_w08_region_multiply(data, matrix[k+0], blocksize, data, 0);
    memcpy(q_coding, data, blocksize);

    int q_index = k - r + 1;
    for(int i=1; i<k+m; i++) {
        if(i == q_index || i == q_index - 1) {
            continue;
        }
        if(i < k) {
            memcpy(data, data_ptrs[i] + blocksize * r, blocksize);
        } else {
            memcpy(data, coding_ptrs[i-k] + blocksize * r, blocksize);
        }
        int data_index = i < q_index - 1 ? i : i - m;
        galois_w08_region_multiply(data, matrix[k + data_index], blocksize, data, 0);
        galois_region_xor(data, q_coding, blocksize);
    }
    if(q_index < k) {
        memcpy(data_ptrs[q_index] + r * blocksize, q_coding, blocksize);
    } else {
        memcpy(coding_ptrs[q_index - k] + r * blocksize, q_coding, blocksize);
    }
    // printf("q coding, blocksize: %d\n", blocksize);
}