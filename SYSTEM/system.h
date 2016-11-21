#ifndef __system_H
#define __system_H

#define uchar unsigned char 
#define uint unsigned int

#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif



extern void system_sleep(void);  
extern void system_start(void);  
extern void work3_sleep(void);
extern void enter_work3_process(void);
extern void exit_work3_process(void);

#endif