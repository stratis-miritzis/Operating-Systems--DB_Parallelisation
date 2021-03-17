#include <string.h>
#include "../engine/db.h"
#include "../engine/variant.h"
#include "bench.h"

#define DATAS ("testdb")


void* _write_test(void* arg)
{
    	pthread_mutex_lock(&mtx);
	args* loc = (args*)arg;
	long int count = loc->count;
	int r = loc->r;
	int curKey = loc->curKey;
	Variant sk, sv;
	DB* db;

	char key[KSIZE + 1];
	char val[VSIZE + 1];
	char sbuf[1024];

	memset(key, 0, KSIZE + 1);
	memset(val, 0, VSIZE + 1);
	memset(sbuf, 0, 1024);

	db = db_open(DATAS);
	
	for(;curKey < count; curKey++) {
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", curKey);
		fprintf(stderr, "%d adding %s\n", curKey, key);
		snprintf(val, VSIZE, "val-%d", curKey);

		sk.length = KSIZE;
		sk.mem = key;
		sv.length = VSIZE;
		sv.mem = val;

		db_add(db, &sk, &sv);
		if ((curKey % 10000) == 0) {
			fprintf(stderr,"random write finished %d ops%30s\r", 
					curKey, 
					"");

			fflush(stderr);
		}
	}
	db_close(db);

	pthread_mutex_unlock(&mtx);
	return NULL;
}

void* _read_test(void* arg)
{
	res* retu;
	retu = (res*)malloc(sizeof(res));
	args* loc = (args*)arg;
	long int count = loc->count;
	int r = loc->r;
	int ret;
	int found = 0;
	Variant sk;
	Variant sv;
	DB* db;
	char key[KSIZE + 1];
	int curKey = loc->curKey;

	db = db_open(DATAS);
	for (;curKey < count; curKey++) {
		memset(key, 0, KSIZE + 1);

		/* if you want to test random write, use the following */
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", curKey);
		fprintf(stderr, "%d searching %s\n", curKey, key);
		sk.length = KSIZE;
		sk.mem = key;
		ret = db_get(db, &sk, &sv);
		if (ret) {
			//db_free_data(sv.mem);
			found++;
		} else {
			INFO("not found key#%s", 
					sk.mem);
    	}

		if ((curKey % 10000) == 0) {
			fprintf(stderr,"random read finished %d ops%30s\r", 
					curKey, 
					"");

			fflush(stderr);
		}
	}

	db_close(db);

	retu->count = count;
	retu->found = found;
	return retu;
	free(retu);
}











