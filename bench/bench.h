#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "../engine/db.h"

#define KSIZE (16)
#define VSIZE (1000)

#define LINE "+-----------------------------+----------------+------------------------------+-------------------+\n"
#define LINE1 "---------------------------------------------------------------------------------------------------\n"

long long get_ustime_sec(void);
void _random_key(char *key,int length);

typedef struct ARGS{
	long int count;
	int r;
	int curKey;
	DB* db;
}args;

typedef struct RES{
    long int count,found;
}res;

void* _write_test(void*);
void* _read_test(void*);