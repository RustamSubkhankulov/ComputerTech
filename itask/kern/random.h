#ifndef JOS_KERN_RANDOM_H
#define JOS_KERN_RANDOM_H

#define RAND_MAX 0x7FFFFFFF

extern int rand(void);
extern void srand(unsigned int seed);
extern void rand_init(unsigned int num);
extern unsigned char _dev_urandom[];
extern unsigned int _dev_urandom_len;

#endif