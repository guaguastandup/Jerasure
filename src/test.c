#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <gf_rand.h>
#include <unistd.h>
#include "jerasure.h"
#include "reed_sol.h"
#include "cauchy.h"
#include "liberation.h"
#include "raid6.h"

int blocksize = 16777216;   // 16MB

int file_size(char *filename) {
    FILE *fp = fopen(filename, "rb"); if(fp == NULL) { return -1; }
    fseek(fp, 0, SEEK_END); int size = ftell(fp); fclose(fp);
    return size;
}

char *fname[10], *cfname[10];
char *data[10], *coding[10];
FILE *fp[10], *cfp[10];

void init(int k, int m, int filesize) {
    fname[0] = "/root/zxy/storage/jerasure/Examples/Coding/T_k1";
    fname[1] = "/root/zxy/storage/jerasure/Examples/Coding/T_k2";
    fname[2] = "/root/zxy/storage/jerasure/Examples/Coding/T_k3";
    fname[3] = "/root/zxy/storage/jerasure/Examples/Coding/T_k4";
    fname[4] = "/root/zxy/storage/jerasure/Examples/Coding/T_k5";
    fname[5] = "/root/zxy/storage/jerasure/Examples/Coding/T_k6";
    fname[k] = "/root/zxy/storage/jerasure/Examples/Coding/T_decoded";
    cfname[0] = "/root/zxy/storage/jerasure/Examples/Coding/T_m1";
    cfname[1] = "/root/zxy/storage/jerasure/Examples/Coding/T_m2";
    cfname[2] = "/root/zxy/storage/jerasure/Examples/Coding/T_m3";
    cfname[m] = "/root/zxy/storage/jerasure/Examples/Coding/T_erased_m";
    char * fname_erased = "/root/zxy/storage/jerasure/Examples/Coding/T_erased";

    for(int i=0;i<=k;i++) {
        fp[i] = fopen(fname[i], "rb");
        if(fp[i] == NULL) {
            printf("no? %s\n", fname[i]);
            fp[i] = fopen(fname_erased, "rb");
        }
        if(i == k) {
            data[i] = (char*)malloc(sizeof(char)*(k*filesize));
            fread(data[i], sizeof(char), k*filesize, fp[i]);
        }
        else {
            data[i] = (char*)malloc(sizeof(char)*filesize);
            fread(data[i], sizeof(char), filesize, fp[i]);
        }
    }
    for(int i=0;i<=m;i++) {
        cfp[i] = fopen(cfname[i], "rb");
        if(cfp[i] == NULL) {
            printf("no??? %s\n", cfname[i]);
            cfp[i] = fopen(fname_erased, "rb");
        }
        if(i == m) {
            coding[i] = (char*)malloc(sizeof(char)*(m*filesize));
            fread(coding[i], sizeof(char), m*filesize, cfp[i]);
        }
        else {
            coding[i] = (char*)malloc(sizeof(char)*filesize);
            fread(coding[i], sizeof(char), filesize, cfp[i]);
        }
    }
    printf("init ok\n");
}

void check(int k, int m, int filesize) {
    for(int i=0;i<k;i++) {
        int flag = 0;
        // printf("data_decoded: \n %s\n", data[k]+i*filesize);
        // printf("data:\n %s\n", data[i]);
        for(int j=0;j<filesize;j++) {
            if(data[i][j] != data[k][j+(i*filesize)]) {
                printf("file %d not ok: %d\n", i+1, j);
                flag = 1;
                break;
            }
        }
        if(flag == 0) {
            printf("data file %d ok\n", i+1);
        }
    }

    for(int i=0;i<m;i++) {
        int flag = 0;
        // printf("code_decoded: \n %s\n", coding[m]+i*filesize);
        // printf("coding:\n %s\n", coding[i]);
        for(int j=0;j<filesize;j++) {
            if(coding[i][j] != coding[m][j+(i*filesize)]) {
                printf("m_file %d not ok: %d\n", i+1, j);
                flag = 1;
                break;
            }
        }
        if(flag == 0) {
            printf("m_file %d ok\n", i+1);
        }
    }
}

void test(int k, int m, int num) {
    int filesize = 16777216 * num;
    init(k, m, filesize);
    check(k, m, filesize);
}