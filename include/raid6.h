#pragma once


#ifndef _RAID6
#define _RAID6

#ifdef __cplusplus
extern "C" {
#endif

extern void raid6_encode(int k, int m, int w, int* matrix, char** data_ptrs, char** coding_ptrs, int size);
extern int  raid6_decode(int k, int m, int w, int* matrix, int* erasures, char **data_ptrs, char** coding_ptrs, int size);

extern void raid6_data_encode(int k, int m, int w, int r, int erased, char **data_ptrs, char** coding_ptrs, int size);
extern void raid6_p_coding_encode(int k, int m, int w, int r, char **data_ptrs, char **coding_ptrs, int size);
extern void raid6_q_coding_encode(int k, int m, int w, int r, int* matrix, char **data_ptrs, char **coding_ptrs, int size);
#ifdef __cplusplus
}
#endif
#endif
