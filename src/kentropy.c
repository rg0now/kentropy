/* 
 * K-order entropy calculator
 * 
 * Written by Andras Majdan
 * License: GNU General Public License Version 3
 * 
 * Report bugs to <majdan.andras@gmail.com>
 */

#define VERSION "1.0.0"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <float.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <arpa/inet.h>

#include "kentropy.h"

/* log2 is only available from C99 and C++11 */
#if __STDC_VERSION__ >= 199901L || __cplusplus > 199711L
#	define LOG2(A) log2(A)
#else
#   define LOG2(A) log(A)/log(2)
#endif

/** Calculates 0-order entropy
 *
 * @param m    Pointer to data
 * @param len  Length of data
 *
 * @return 0-order entropy of data
 */
double kentropy0(symbol_t *m, unsigned int len)
{
    unsigned int num[KENTROPY_SYMBOL_TABLE_SIZE] = {0};
    unsigned int i=0;
    double p;
    double ent = 0.0f;
    symbol_t j;

    while(i < len)
        num[(symbol_t)(ntohs(m[i++]))]++;

    for(j=0; j<KENTROPY_SYMBOL_TABLE_SIZE; j++)
        if(num[j] > 0)
            printf("Symbol 0x%04x: %d\n", j, num[j]);

    for(j=0; j<KENTROPY_SYMBOL_TABLE_SIZE; j++)
	{
            p = (double)num[j] / len;
            if(p > 0)
                ent = ent - p * LOG2(p);
	}

    return ent;
}

/* /\** Calculates k-order entropy */
/*  * */
/*  * @param m    Pointer to data */
/*  * @param len  Length of data */
/*  * @param k    Order of entropy */
/*  * */
/*  * @return k-order entropy of data */
/*  *\/ */
/* double kentropy(char *m, off_t len, unsigned long int k) */
/* { */
/* 	char *ptr = m; */
/* 	char *end; */
/* 	unsigned long int lensubk = (len-k <= 0) ? 0 : len-k; */

/* 	if(lensubk == 0) return 0.0f; */
/* 	end = m+lensubk; */

/* 	srand(time(NULL)); */

/* 	mdht *ht = mdhtCreate(1024, 10, murmur3, myrand); */
/* 	MDTH_GETORADD_RESULT res; */

/* 	/\*store *storepool = createStorePool(512); */
/* 	store *storeret; */
/* 	int numInPool = 0;*\/ */

/* 	mdht_entry *entry; */

/* 	chll *cl = (chll*)malloc( sizeof(chll) ); */
/* 	cl->num = 1; */
/* 	cl->next = NULL; */
/* 	chll *clret; */

/* 	while(ptr != end) */
/* 	{ */
/* 		cl->c = *(ptr+k); */
/* 		entry = mdhtGetOrAdd(ht, (void*)ptr, k, (void*)cl, */
/* 					(void*)&clret, &res); */

/* 		if(res == MDHT_GET_SUCCESS) */
/* 		{ */
/* 			while(clret != NULL) */
/* 			{ */
/* 				if(clret->c ==  *(ptr+k)) */
/* 				{ */
/* 					clret->num++; */
/* 					break; */
/* 				} */
/* 				else */
/* 				{ */
/* 					if(clret->next == NULL) */
/* 					{ */
/* 						clret->next = (chll*)malloc( sizeof(chll) ); */
/* 						clret->next->c = *(ptr+k); */
/* 						clret->next->num = 1; */
/* 						clret->next->next = NULL; */
/* 						break; */
/* 					} */
/* 					clret = clret->next; */
/* 				} */
/* 			} */
/* 		} */
/* 		else */
/* 		{ */
/* 			cl = (chll*)malloc( sizeof(chll) ); */
/* 			cl->num = 1; */
/* 			cl->next = NULL; */
/* 		} */

/* 		/\*storepool[numInPool].num = 1; */

/* 		entry = mdhtGetOrAdd(ht, (void*)ptr, k, (void*)&storepool[numInPool], */
/* 					(void*)&storeret, &res); */
/* 		if(res == MDHT_GET_SUCCESS) */
/* 		{ */
/* 			// Already in */
/* 			cnhtSetOrInc(storeret->tbl, *(ptr+k), 1); */
/* 		} */
/* 		else */
/* 		{ */
/* 			cnhtSetOrInc(storepool[numInPool].tbl, *(ptr+k), 1); */

/* 			if(++numInPool >= 512) */
/* 			{ */
/* 				storepool = createStorePool(512); */
/* 				numInPool = 0; */
/* 			} */
/* 		}*\/ */

/* 		ptr++; */
/* 	} */

/* 	//mdhtPrint(ht); */

/* 	/\*for(uint32_t i=numInPool; i < 512; i++) */
/* 		storeDestroy((void*)&storepool[i]); */

/* 	//free(storepool); */

/* 	mdhtDestroy(ht, storeDestroy);*\/ */

/* 	return 0.0 - calce(ht->table, len, 0.0); */
/* } */

/** Calculates the k-order entropy of a file
 *
 * @param filename  Name of the file
 * @param k         Order of entropy
 *
 * @return k-order entropy of file
 */
double kentropy_file(char *filename, unsigned long int k)
{
    int fd;
    struct stat s;
    symbol_t *m;
    double kent;

    /* Open and map the file to memory */
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return DBL_MAX;
    }
    fstat(fd, &s);

    m = (symbol_t*)mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(m == MAP_FAILED)
	{
            perror("mmap");
            return DBL_MAX;
	}

    /* Calculate the k-order entropy */
    if (k == 0)
        kent = kentropy0(m, s.st_size / sizeof(symbol_t));
    else {
        /* kent = kentropy(m, s.st_size, k); */
        fprintf(stderr, "Higher order entropy calculation is disabled for now\n");
        return DBL_MAX;

    }

    /* Unmap and close the file */
    munmap(m, s.st_size);
    close(fd);

    return kent;
}

int main(int argc, char *argv[])
{
    int c;
    char *endptr;
    int fixrange = 0;

    unsigned long int k = 0;

    while(1)
	{
            static struct option long_options[] =
		{
                    {"k",        required_argument, 0, 'k'},
                    {"fixrange", no_argument,       0, 'f'},
                    {"help",     no_argument,       0, 'h'},
                    {"version",  no_argument,       0, 'v'},
                    {0, 0, 0, 0}
		};

            int option_index = 0;
            c = getopt_long(argc, argv, "k:fhv", long_options, &option_index);

            /* Detect the end of the options. */
            if (c == -1)
                break;

            switch (c)
		{
                case 'k':
                    k = strtoul(optarg, &endptr, 10);

                    if(*endptr!='\0' || errno!=0)
                        {
                            fprintf(stderr, "Wrong argument for k: %s\n", optarg);
                            exit(EXIT_FAILURE);
                        }
                    break;

                case 'f':
                    fixrange = 1;
                    break;

                case 'h':
                    printf(
                           "Usage: kentropy [options] [file1] [file2].. \n\n"

                           "Calculates the k-order entropy of files.\n\n"

                           "Options:\n"
                           "  --k NUM    Order of the entropy (default: 0)\n"
                           "  --fixrange Scale entropy into [0,1] range\n"
                           "  --help     Usage information\n"
                           "  --version  Print version\n\n"

                           "Example 1: kentropy input.txt\n"
                           "Example 2: kentropy --k 4 input.dat\n"
                           "Example 3: kentropy input1.txt input2.txt input3.txt\n\n"

                           "Written by Andras Majdan\n"
                           "License: GNU General Public License Version 3\n"
                           "Report bugs to <majdan.andras@gmail.com>\n");
                    exit (EXIT_SUCCESS);
                    break;

                case 'v':
                    printf("Version: %s (Compiled on %s)\n", VERSION, __DATE__);
                    exit (EXIT_SUCCESS);
                    break;
                case '?':
                    /* getopt_long already printed an error message. */
                    exit(EXIT_FAILURE);
                    break;
		}
	}

    /* No error */
    int errnum = 0;

    /* Process additional arguments */
    if (optind < argc)
	{
            double kent;
            char *filename;


            while (optind < argc)
		{
                    filename = argv[optind++];

                    kent = kentropy_file(filename, k);

                    if(fixrange)
                        kent = kent / ((k+1)*8);

                    if(kent != DBL_MAX)
                        printf("%s %lf\n", filename, kent);
                    else
			{
                            fprintf(stderr, "FAILED: %s\n", filename);
                            errnum++;
			}
		}
	}
    else
	{
            fprintf(stderr, "Requires at least one file argument!\n");
            errnum = 1;
	}

    return errnum;
}

