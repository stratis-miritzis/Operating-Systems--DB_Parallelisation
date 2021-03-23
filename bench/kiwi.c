#include <string.h>
#include "../engine/db.h"
#include "../engine/variant.h"
#include "bench.h"

#define DATAS ("testdb")


void* _write_test(void* arg)
{
	res* retu;
	retu = (res*)malloc(sizeof(res));
	args* loc = (args*)arg;
	long int count = loc->count;
	int r = loc->r;
	int curKey = loc->curKey;
	Variant sk, sv;
	DB* dbase;
	dbase = loc->db;

	char key[KSIZE + 1];
	char val[VSIZE + 1];
	char sbuf[1024];

	memset(key, 0, KSIZE + 1);
	memset(val, 0, VSIZE + 1);
	memset(sbuf, 0, 1024);
	
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

		db_add(dbase, &sk, &sv);
	}

	retu->found = 0;
	return retu;
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
	char key[KSIZE + 1];
	int curKey = loc->curKey;
	DB* dbase;
	dbase = loc->db;

	for (;curKey < count; curKey++) {
		memset(key, 0, KSIZE + 1);

		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", curKey);
		fprintf(stderr, "%d searching %s\n", curKey, key);
		sk.length = KSIZE;
		sk.mem = key;
		ret = db_get(dbase, &sk, &sv);
		if (ret) {
			//db_free_data(sv.mem);
			found++;
		} else {
			INFO("not found key#%s", 
					sk.mem);
    		}
	}

	retu->count = count;
	retu->found = found;
	return retu;
	free(retu);
}











