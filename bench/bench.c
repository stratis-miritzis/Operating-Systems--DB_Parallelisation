#include "bench.h"

#define DATAS ("testdb")  /*to metaferame apo to arxeio kiwi.c gia na ekteloume to db_open, db_close mia fora xwris ta threads*/

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

/*----OUR CODE----*/

void _print_help(){					/*e3hgei thn xrhsh twn orismatwn sto command line*/
		printf(LINE1);
		printf("\nIn readwrite mode the minimum threads are 2.\nWhen in readwrite mode the 4th argument defines the percentage \nat which the count is split up to read and writes.\n100 means 100%c of the count number is written.\n\n",37);
		printf(LINE1);
}

int main(int argc,char** argv)
{
	int i;				/*counter gia tis for*/
	long int count;	
	pthread_t *threads;		/*pinakas me antikeimena pthread_t*/
	int threadcount;		/*orisma apo grammh entolwn gia to posa threads tha xrhsimopoih8oun*/
	args *arg;			/*pinakas me antikeimena args(struct ston bench.h) gia ta orismata twn threads*/
	res* ret;			/*pinakas me antikeimena res(struct ston bench.h) gia ta apotelesmata twn threads*/
	ret = (res*)malloc(sizeof(res));		
	int found = 0;
	double cost;
	long long int start,end;

	DB* db;				/*to metaferame apo to arxeio kiwi.c gia na ekteloume to db_open, db_close mia fora xwris ta threads*/

	srand(time(NULL));
	if (strcmp(argv[1], "help") == 0){
		_print_help();
	}
	if (argc < 4) {
		fprintf(stderr,"Usage: db-bench <write | read | readwrite> <count> <threads> <readwrite %c >\n",37);
		exit(1);
	}

	if (strcmp(argv[1], "write") == 0) {						//write
		int r = 0;
        
        count = atoi(argv[2]);
		threadcount = atoi(argv[3]);						/*to 3o argument apo to command line gia ton ari8mo twn threads*/
        
		arg = (args*)malloc(threadcount*sizeof(args));				/*pinakas me antikeimena args(struct ston bench.h) gia ta orismata twn threads*/
		threads  = (pthread_t*)malloc((threadcount)*sizeof(pthread_t));		/*pinakas me antikeimena pthread_t*/

		_print_header(count);
		_print_environment();
		if (argc == 5)
			r = 1;
		start = get_ustime_sec();

				/*----------DIKOS MAS KWDIKAS-----------*/

		int work = count/threadcount;						/*ypologizoume ton forto(posa writes) tou ka8e thread*/
		db = db_open(DATAS);							/*anoigoume-arxikopoioume thn vash dedomenwn*/
		
/*arxizoume kai gemizoume ton pinaka me ta orismata poy dinoume sta threads.
Otan kaloume thn _write_test sto kiwi.c ths dinoume san orisma thn vash
dedomenwn, apo poio kleidi ews poio 8a grapsei ka8e thread (curkey-count),
kai an 8a grapsei tyxaia kleidia.*/

		for(i = 0;i < threadcount;i++){
			arg[i].db = db;
			arg[i].curKey = i*work;
			arg[i].count = (i+1)*work;
			arg[i].r = r;
		}
		arg[threadcount-1].count = count;

/*apo8hkeuoume ta threads ston pinaka antikeimenwn pthread_t kai dinoume orismata apo ton pinaka arg pou ta kanoume cast se typo void*/

		for(i = 0;i < threadcount;i++){
			pthread_create(&threads[i],NULL,_write_test,(void*)&arg[i]);	
		}

/*ta kanoume join wste na teleiwsoun ola mazi */

     	for(i = 0;i < threadcount;i++){
            pthread_join(threads[i],NULL);
		}

		db_close(db);				/*kleinoume to db*/
							
		end = get_ustime_sec();
		cost = (double)(end - start)/1000000;
		
		printf(LINE);				/*print olika stats(ta metaferame apo to kiwi.c) gia na mhn kaleitai apo to ka8e thread*/
		printf("|Random-Write	(done:%ld): %.6f sec/op; %.1f writes/sec(estimated); cost:%f(sec);\n"
			,count, (double)(cost / count)
			,(double)(count / cost)
			,cost);	


	} else if (strcmp(argv[1], "read") == 0) {					//read

		int r = 0;
        
        count = atoi(argv[2]);
		threadcount = atoi(argv[3]);						/*orisma apo grammh entolwn gia to posa threads tha xrhsimopoih8oun*/
        
		arg = (args*)malloc(threadcount*sizeof(args));				/*pinakas me antikeimena args(struct ston bench.h) gia ta orismata twn threads*/
		threads  = (pthread_t*)malloc(threadcount*sizeof(pthread_t));		/*pinakas me antikeimena pthread_t*/

		_print_header(count);
		_print_environment();
		if (argc == 5)
			r = 1;

		int work = count/threadcount;						/*ypologizoume ton forto(posa reads) tou ka8e thread*/
		db = db_open(DATAS);							/*anoigoume-arxikopoioume thn vash dedomenwn*/

		start = get_ustime_sec();

/*arxizoume kai gemizoume ton pinaka me ta orismata poy dinoume sta threads.
Otan kaloume thn _read_test sto kiwi.c ths dinoume san orisma thn vash
dedomenwn, apo poio kleidi ews poio 8a diavasei ka8e thread (curKey-count),
kai an 8a diavasei tyxaia kleidia.*/

		for(i = 0;i < threadcount;i++){
			arg[i].db = db;
			arg[i].curKey = i*work;
			arg[i].count = (i+1)*work;
			arg[i].r = r;
		}
		arg[threadcount-1].count = count;

/*apo8hkeuoume ta threads ston pinaka antikeimenwn pthread_t kai dinoume orismata apo ton pinaka arg pou ta kanoume cast se typo void*/

		for(i = 0;i < threadcount;i++){
			pthread_create(&threads[i],NULL,_read_test,(void*)&arg[i]);
		}

/*ta kanoume join wste na teleiwsoun ola mazi kai au3anoume ton counter found analoga me ta kleidia pou vrethikan*/

     	for(i = 0;i < threadcount;i++){
            pthread_join(threads[i],(void*)&ret);
            found += ret->found;
		}

		db_close(db);								/*kleinoume thn vash dedomenwn*/
        
		end = get_ustime_sec();
		cost = (double)(end - start)/1000000;
        
		printf(LINE);								/*print olika stats(ta metaferame apo to kiwi.c) gia na mhn kaleitai apo to ka8e thread*/
		printf("|Random-Read	(done:%ld, found:%d): %.6f sec/op; %.1f reads /sec(estimated); cost:%f(sec)\n",
			count, found,
			(double)(cost / count),
			(double)(count / cost),
			cost);
	

	}else if (strcmp(argv[1], "readwrite") == 0) {					//readwrite
		if(atoi(argv[3]) < 2){
			_print_help();
			exit(1);
		}
		int r = 0;
        
		count = atoi(argv[2]);
        threadcount = atoi(argv[3]);                                /*orisma apo grammh entolwn gia to posa threads tha xrhsimopoih8oun*/
        int perc = atoi(argv[4]);                                    /*pososto egrafwn/anagnwsewn. 100 = mono egrafes, 0 = mono anagnwseis*/
        
		if (argc == 6)
			r = 1;

		arg = (args*)malloc(threadcount*sizeof(args));				/*pinakas me antikeimena args(struct ston bench.h) gia ta orismata twn threads*/
		threads  = (pthread_t*)malloc(threadcount*sizeof(pthread_t));/*pinakas me antikeimena pthread_t*/
		_print_header(count);
		_print_environment();
		int workr = (count*(100-perc)/100)/(threadcount-1);			/*o fortos twn anagnwstwn(count se ka8e thread)*/
		int writen = count*perc/100;								/*o fortos tou egrafea*/


		start = get_ustime_sec();

		db = db_open(DATAS);										/*anoigoume-arxikopoioume thn vash dedomenwn*/


/*ston pinaka me ta arguments h 8esh 0 einai gia ton egrafea kai oi alles gia tous anagnwstes*/
/*oi anagnwstes einai sthn 8esh 1-threadcount*/

		arg[0].db = db;
		arg[0].curKey = 0;
		arg[0].count = writen;
		arg[0].r = r;

		for(i = 1;i < threadcount;i++){
			arg[i].db = db;
			arg[i].curKey = workr*(i-1);
			arg[i].count = workr*(i-1)+workr;
			arg[i].r = r;
		}
		arg[threadcount-1].count = count-arg[0].count;
        
/*dhmiourgoume ta threads*/
		for(i = 0;i < threadcount;i++){
			if(i == 0){
				pthread_create(&threads[i],NULL,_write_test,(void*)&arg[i]);
			}else{
				pthread_create(&threads[i],NULL,_read_test,(void*)&arg[i]);
			}
		}
        
/*join gia na teleiwsoun ola mazi*/
     	for(i = 0;i < threadcount;i++){
            pthread_join(threads[i],(void*)&ret);
			found += ret->found;
		}

		db_close(db);								/*kleinoume thn vash dedomenwn*/
        
		end = get_ustime_sec();
		cost = (double)(end -start)/1000000;
        
		printf(LINE);								/*print olika stats poses egrafes-anagnwseis eginan,egrafes mono,posa vre8hkan*/
		printf("|Random-ReadWrite	(done:%ld, writen:%d, found:%d): %.6f sec/op; %.1f reads /sec(estimated); cost:%f(sec)\n",
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
