#pragma once


#ifndef _ROTATEDRS
#define _ROTATEDRS

#ifdef __cplusplus
extern "C" {
#endif

extern void rotatedrs_encode(int k, int m, int w, char** data_ptrs, char** coding_ptrs, int size);
extern int  rotatedrs_decode(int k, int m, int w, int* erasures, char **data_ptrs, char** coding_ptrs, int size);

extern int* rotatedrs_coding_matrix(int k, int m, int w);

extern void rotatedrs_coding_decode(int k, int m, int w, int erased, char** data_ptrs, char** coding_ptrs, int size);
extern void rotatedrs_data_decode(int k, int m, int w, int *matrix, int erased, char** data_ptrs, char** coding_ptrs, int size);

#ifdef __cplusplus
}
#endif
#endif
