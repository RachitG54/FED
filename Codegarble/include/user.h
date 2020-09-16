#ifndef USER_H_
#define USER_H_
int checkfn(int *a, int *outputs, int n);
int checkadd(int *a, int *outputs, int n);
int checkandB_k(int *a, int *outputs, int n, int k);
int checkxor(int *a, int *outputs, int n);
int checkshift(int *a, int *outputs, int n);
int checkshift_k(int *a, int *outputs, int n, int k);
int checknot(int *a, int *outputs, int n);
int checkinc(int *a, int *outputs, int n);
int checksub(int *a, int *outputs, int n);
int checksetzero(int bit, int* outputs, int n);
int checkaddN1(int* a, int *outputs, int n, int bit);
void print(int*outputs, int n);
int checkham2(int *a, int *outputs, int n);
int checkham(int *a, int *outputs, int n);
#endif /* USER_H_ */
