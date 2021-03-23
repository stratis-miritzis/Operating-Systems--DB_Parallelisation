#include "bench.h"
#include "../engine/db.h"
pthread_mutex_t lock;

#define DATAS ("testdb")

void _random_key(char *key,int length) {
	int i;
	char salt[36]= "abcdefghijklmnopqrstuvwxyz0123456789";

	for (i = 0; i < length; i++)
		key[i] = salt[rand() % 36];
}

void _print_header(int count)
{
	double index_size = (double)((double)(KSIZE + 8 + 1) * count) / 1048576.0;
	double data_size = (double)((double)(VSIZE + 4) * count) / 1048576.0;

	printf("Keys:\t\t%d bytes each\n", 
			KSIZE);
	printf("Values: \t%d bytes each\n", 
			VSIZE);
	printf("Entries:\t%d\n", 
			count);
	printf("IndexSize:\t%.1f MB (estimated)\n",
			index_size);
	printf("DataSize:\t%.1f MB (estimated)\n",
			data_size);

	printf(LINE1);
}

void _print_environment()
{
	time_t now = time(NULL);

	printf("Date:\t\t%s", 
			(char*)ctime(&now));

	int num_cpus = 0;
	char cpu_type[256] = {0};
	char cache_size[256] = {0};

	FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
	if (cpuinfo) {
		char line[1024] = {0};
		while (fgets(line, sizeof(line), cpuinfo) != NULL) {
			const char* sep = strchr(line, ':');
			if (sep == NULL || strlen(sep) < 10)
				continue;

			char key[1024] = {0};
			char val[1024] = {0};
			strncpy(key, line, sep-1-line);
			strncpy(val, sep+1, strlen(sep)-1);
			if (strcmp("model name", key) == 0) {
				num_cpus++;
				strcpy(cpu_type, val);
			}
			else if (strcmp("cache size", key) == 0)
				strncpy(cache_size, val + 1, strlen(val) - 1);	
		}

		fclose(cpuinfo);
		printf("CPU:\t\t%d * %s", 
				num_cpus, 
				cpu_type);

		printf("CPUCache:\t%s\n", 
				cache_size);
	}
}

int main(int argc,char** argv)
{
	int i;
	long int count;
	pthread_t *threads;
	int threadcount;
	args *arg;
	res* ret;
	ret = (res*)malloc(sizeof(res));
	long int cnt = 0;
	int found = 0;
	double cost;
	long long int start,end;

	DB* db;

	srand(time(NULL));
	if (argc < 4) {
		fprintf(stderr,"Usage: db-bench <write | read | readwrite> <count> <threads> <readwrite %c >\n",37);
		exit(1);
	}
	
	if (strcmp(argv[1], "write") == 0) {						//write
		int r = 0;
		threadcount = atoi(argv[3]);
		arg = (args*)malloc(threadcount*sizeof(args));
		threads  = (pthread_t*)malloc((threadcount)*sizeof(pthread_t));


		count = atoi(argv[2]);
		_print_header(count);
		_print_environment();
		if (argc == 5)
			r = 1;
		int work = count/threadcount;
		start = get_ustime_sec();

		db = db_open(DATAS);

		arg[0].db = db;
		arg[0].curKey = 0;
		arg[0].count = count/threadcount;
		arg[0].r = r;

		for(i = 1;i < threadcount;i++){
			arg[i].db = db;
			arg[i].curKey = i*work;
			arg[i].count = (i+1)*work;
			arg[i].r = r;
		}
		for(i = 0;i < threadcount;i++){
			pthread_create(&threads[i],NULL,_write_test,(void*)&arg[i]);
		}
     		for(i = 0;i < threadcount;i++){
            		pthread_join(threads[i],NULL);
		}

		end = get_ustime_sec();
		cost = end -start;
		
		db_close(db);

		printf(LINE);
		printf("|Random-Write	(done:%ld): %.6f sec/op; %.1f writes/sec(estimated); cost:%.3f(sec);\n"
			,count, (double)(cost / count)
			,(double)(count / cost)
			,cost);	


	} else if (strcmp(argv[1], "read") == 0) {					//read

		int r = 0;
		threadcount = atoi(argv[3]);
		arg = (args*)malloc(threadcount*sizeof(args));
		threads  = (pthread_t*)malloc(threadcount*sizeof(pthread_t));

		count = atoi(argv[2]);
		_print_header(count);
		_print_environment();
		if (argc == 5)
			r = 1;

		db = db_open(DATAS);

		int work = count/threadcount;
		start = get_ustime_sec();

		arg[0].db = db;
		arg[0].curKey = 0;
		arg[0].count = count/threadcount;
		arg[0].r = r;
	
		for(i = 1;i < threadcount;i++){
			arg[i].db = db;
			arg[i].curKey = i*work;
			arg[i].count = (i+1)*work;
			arg[i].r = r;
		}
		for(i = 0;i < threadcount;i++){
			pthread_create(&threads[i],NULL,_read_test,(void*)&arg[i]);
		}
     		for(i = 0;i < threadcount;i++){
            		pthread_join(threads[i],(void*)&ret);
            		cnt += ret->count;
            		found += ret->found;
		}
		db_close(db);
		end = get_ustime_sec();
		cost = end - start;
		printf(LINE);
		printf("|Random-Read	(done:%ld, found:%d): %.6f sec/op; %.1f reads /sec(estimated); cost:%.6f(sec)\n",
			count, found,
			(double)(cost / count),
			(double)(count / cost),
			cost);
	

	}else if (strcmp(argv[1], "readwrite") == 0) {					//readwrite
		int r = 0;
		int perc = atoi(argv[4]);
		count = atoi(argv[2]);	
		if (argc == 6)
			r = 1;
		threadcount = atoi(argv[3]);
		threads  = (pthread_t*)malloc(threadcount*sizeof(pthread_t));
		arg = (args*)malloc(threadcount*sizeof(args));
		_print_header(count);
		_print_environment();
		int workr = (count*(100-perc)/100)/(threadcount-1);
		int writen = count*perc/100;


		start = get_ustime_sec();

		db = db_open(DATAS);


		arg[0].db = db;
		arg[0].curKey = 0;
		arg[0].count = writen;
		arg[0].r = r;

		arg[1].db = db;
		arg[1].curKey = 0;
		arg[1].count = workr;
		arg[1].r = r;

		for(i = 2;i < threadcount;i++){
			arg[i].db = db;
			arg[i].curKey = workr*(i-1);
			arg[i].count = workr*(i-1)+workr;
			arg[i].r = r;
		}	


		for(i = 0;i < threadcount;i++){
			if(i == 0){
				pthread_create(&threads[i],NULL,_write_test,(void*)&arg[i]);
			}else{
				pthread_create(&threads[i],NULL,_read_test,(void*)&arg[i]);
			}
		}
     		for(i = 0;i < threadcount;i++){
            		pthread_join(threads[i],(void*)&ret);
			found += ret->found;
		}

		db_close(db);
		end = get_ustime_sec();
		cost = end - start;
		printf(LINE);
		printf("|Random-ReadWrite	(done:%ld, writen:%d, found:%d): %.6f sec/op; %.1f reads /sec(estimated); cost:%.6f(sec)\n",
			count,writen,found,
			(double)(cost / count),
			(double)(count / cost),
			cost);

	} else {
		fprintf(stderr,"Usage: db-bench <write | read | readwrite> <count> <threads> <random>\n");
		exit(1);
	}

	free(ret);
	free(arg);
	free(threads);
	return 1;
}
