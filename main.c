#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define BYTES_PER_WORD 4

char** str_split(char *a_str, const char a_delim){
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp){
	if (a_delim == *tmp){
	    count++;
	    last_comma = tmp;
	}
	tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
     *        knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result){
	size_t idx  = 0;
	char* token = strtok(a_str, delim);

	while (token){
	    assert(idx < count);
	    *(result + idx++) = strdup(token);
	    token = strtok(0, delim);
	}
	assert(idx == count - 1);
	*(result + idx) = 0;
    }

    return result;
}

/***************************************************************/
/*                                                             */
/* Procedure : cdump                                           */
/*                                                             */
/* Purpose   : Dump cache configuration                        */   
/*                                                             */
/***************************************************************/
void cdump(int capacity, int assoc, int blocksize){

	printf("Cache Configuration:\n");
    	printf("-------------------------------------\n");
	printf("Capacity: %dB\n", capacity);
	printf("Associativity: %dway\n", assoc);
	printf("Block Size: %dB\n", blocksize);
	printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : sdump                                           */
/*                                                             */
/* Purpose   : Dump cache stat		                       */   
/*                                                             */
/***************************************************************/
void sdump(int total_reads, int total_writes, int write_backs,
	int reads_hits, int write_hits, int reads_misses, int write_misses) {
	printf("Cache Stat:\n");
    	printf("-------------------------------------\n");
	printf("Total reads: %d\n", total_reads);
	printf("Total writes: %d\n", total_writes);
	printf("Write-backs: %d\n", write_backs);
	printf("Read hits: %d\n", reads_hits);
	printf("Write hits: %d\n", write_hits);
	printf("Read misses: %d\n", reads_misses);
	printf("Write misses: %d\n", write_misses);
	printf("\n");
}


/***************************************************************/
/*                                                             */
/* Procedure : xdump                                           */
/*                                                             */
/* Purpose   : Dump current cache state                        */ 
/* 							       */
/* Cache Design						       */
/*  							       */
/* 	    cache[set][assoc][word per block]		       */
/*      						       */
/*      						       */
/*       ----------------------------------------	       */
/*       I        I  way0  I  way1  I  way2  I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set0  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set1  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*      						       */
/*                                                             */
/***************************************************************/
void xdump(int set, int way, uint32_t** cache)
{
	int i,j,k = 0;

	printf("Cache Content:\n");
    	printf("-------------------------------------\n");
	for(i = 0; i < way;i++)
	{
		if(i == 0)
		{
			printf("    ");
		}
		printf("      WAY[%d]",i);
	}
	printf("\n");

	for(i = 0 ; i < set;i++)
	{
		printf("SET[%d]:   ",i);
		for(j = 0; j < way;j++)
		{
			if(k != 0 && j == 0)
			{
				printf("          ");
			}
			printf("0x%08x  ", cache[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void update_lru(unsigned char* set, unsigned char index, int way) {
	int i;
	unsigned char cur = set[index];
	if (cur == 0) {
		for (i = 0; i < way; i++) {
			if (i == index) {
				set[i] = 1;
				continue;
			}
			if (set[i] == 0) continue;
			set[i]++;
		}
	}

	else {
		for (i = 0; i < way; i++) {
			if (i == index) {
				set[i] = 1;
				continue;
			}
			if (set[i] == 0) continue;
			if (set[i] < cur) set[i]++;
		}
	}
}

unsigned char get_victim(unsigned char* set, int way) {
	int i;
	unsigned char victim_idx = -1;
	for (i = 0; i < way; i++) {
		if (set[i] == 8) {
			victim_idx = i;
			break;
		}
	}
	assert(victim_idx != -1);
	return victim_idx;
}

int main(int argc, char *argv[]) {                              
	char *file_name = argv[argc-1];
	char **cache_config = str_split(argv[2], ':');

	uint32_t** cache;
	unsigned char** dirty;
	unsigned char** lru;
	unsigned char* last_idx;
	int i, j, k;	
	int capacity = atoi(cache_config[0]);
	int way = atoi(cache_config[1]);
	int blocksize = atoi(cache_config[2]);
	int set = capacity/way/blocksize;
	int words = blocksize / BYTES_PER_WORD;

	int total_reads = 0;
	int total_writes = 0;
	int write_backs = 0;
	int read_hits = 0;
	int write_hits = 0;
	int read_misses = 0;
	int write_misses = 0;

	//printf("%s: capacity %d, way %d, blocksize %d\n", file_name, capacity, way, blocksize);

	FILE *prog;
	if ((prog = fopen(file_name, "r")) == NULL) {
		printf("Error: cannot open file %s\n", file_name);
		exit(-1);
	}

	// allocate
	cache = (uint32_t**) malloc (sizeof(uint32_t*) * set);
	dirty = (unsigned char**) malloc(sizeof(unsigned char*) * set);
	lru = (unsigned char**) malloc(sizeof(unsigned char*) * set);
	last_idx = (unsigned char*) malloc(sizeof(unsigned char) * set); // less than way
	for(i = 0; i < set; i++) {
		cache[i] = (uint32_t*) malloc(sizeof(uint32_t) * way);
		dirty[i] = (unsigned char*) malloc(sizeof(unsigned char) * way);
		lru[i] = (unsigned char*) malloc(sizeof(unsigned char) * way);
	}
	for(i = 0; i < set; i++) {
		last_idx[i] = 0;
		for(j = 0; j < way; j ++) {
			cache[i][j] = 0x0;
			dirty[i][j] = 0;
			lru[i][j] = 0;
		}
	}

	char buffer[14];
	char rw[2], addr[11] = {0};
	uint32_t address;
	unsigned char index1, index2, hit;
	uint32_t *cur_set;
	while (fgets(buffer, 14, prog) != NULL) {
		//printf("%s\n", buffer);
		sscanf(buffer, "%1s %10s", rw, addr);
		
		if (strcmp(rw, "R") == 0) total_reads++;
		else if(strcmp(rw, "W") == 0) total_writes++;
		else {printf("neither R nor W\n"); exit(-1);}

		address = strtol(addr, NULL, 16);
		address = address - address % blocksize;
		index1 = (address/blocksize) % set;
		cur_set = cache[index1];
		//printf("rw: %s, address: %x, index: %d\n", rw, address, index1);

		//for loop - check hit
		hit = 0;
		for (i = 0; i < way; i++) {
			if (address == cur_set[i]) {
				hit = 1;
				index2 = i;
				update_lru(lru[index1], index2, way);

				if (strcmp(rw, "R") == 0) read_hits++;
				else {
					write_hits++;
					dirty[index1][index2] = 1;
				}
				break;
			}
		}
		if (hit == 1) continue;

		// miss
		if (last_idx[index1] == way) {
			//evict and put block
			index2 = get_victim(lru[index1], way);
			if (dirty[index1][index2] == 1) {
				write_backs++;
				dirty[index1][index2] = 0;
			}

			cur_set[index2] = address;
			update_lru(lru[index1], index2, way);

			if (strcmp(rw, "R") == 0) read_misses++;
			else {
				write_misses++;
				dirty[index1][index2] = 1;
			}
		}
		else if (last_idx[index1] < way && last_idx[index1] >= 0) {
			//put block
			index2 = last_idx[index1];
			cur_set[index2] = address;
			update_lru(lru[index1], index2, way);
			last_idx[index1]++;

			if (strcmp(rw, "R") == 0) read_misses++;
			else {
				write_misses++;
				dirty[index1][index2] = 1;
			}
		}
		else {
			printf("error: wrong index\n");
			exit(-1);
		}
	}

	// test example
	cdump(capacity, way, blocksize);
	sdump(total_reads, total_writes, write_backs, read_hits, write_hits, read_misses, write_misses); 
	xdump(set, way, cache);

	return 0;
}
