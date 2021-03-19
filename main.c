#include <stdio.h>
#include <string.h>
#include <stdlib.h>


char *substr(char *str, int start, int end){
	char *substr = (char *)malloc(strlen(str)+1);
	strncpy(substr, str + start, end-start);
	substr[end-start] = '\0';
	return substr;
}

int check_hex(char *str) {
	if (strlen(str) >= 3 && strcmp(substr(str, 0, 2), "0x") == 0) return 1;
	else return 0;
}

char *int_to_binary(int n) {
	char *b = (char *)malloc(32 + 1);
	int i, d;
	for (i = 0; i < 32; i++){
		d = n >> (31 - i);
		if (d & 1) *(b + i) = '1';
		else *(b + i) = '0';
	}
	*(b + 32) = '\0'; 
	return b;
}

char *short_to_binary(short n) {
	char *b = (char *)malloc(16 + 1);
	int i, d;
	for (i = 0; i < 16; i++){
		d = n >> (15 - i);
		if (d & 1) *(b + i) = '1';
		else *(b + i) = '0';
	}
	*(b + 16) = '\0'; 
	return b;
}

int find(char *array[], char *e, int len) {
	int i;
	for (i = 0; i < len; i++) {
		if (!strcmp(array[i], e)) return i;
	}
	return -1;
}

int find1(char *str) {
	int i;
	for (i = 0; i < strlen(str); i++) {
		if (str[i] == '(') return i;
	}
	return -1;
}

int find2(char *str) {
	int i;
	for (i = 0; i < strlen(str); i++) {
		if (str[i] == ')') return i;
	}
	return -1;
}

int main(int argc, char* argv[]){

	if(argc != 2){
		printf("Usage: ./runfile <assembly file>\n"); //Example) ./runfile /sample_input/example1.s
		printf("Example) ./runfile ./sample_input/example1.s\n");
		exit(0);
	}
	else
	{

		// To help you handle the file IO, the deafult code is provided.
		// If we use freopen, we don't need to use fscanf, fprint,..etc. 
		// You can just use scanf or printf function 
		// ** You don't need to modify this part **
		// If you are not famailiar with freopen,  you can see the following reference
		// http://www.cplusplus.com/reference/cstdio/freopen/

		//For input file read (sample_input/example*.s)

		char *file=(char *)malloc(strlen(argv[1])+3);
		strncpy(file,argv[1],strlen(argv[1]));

		FILE *fp;
		if((fp = freopen(file, "r",stdin))==0){
			printf("File open Error!\n");
			exit(1);
		}

		//From now on, if you want to read string from input file, you can just use scanf function.
		int text_address = 0x400000;
		int data_address = 0x10000000;
		
		//int text_size = 0;
		//int data_size = 0;
		
		char *str = (char *)malloc(50); // string from scanf
		char *answer = (char *)malloc(2000); // final output
		char *inst = (char *)malloc(1000); // instruction binary format
		char *val = (char *)malloc(1000); // value binary format

		char *label_array[10];
		int address_array[10];
		//int data_array[10];
		int label_index = 0; // index for label array and address array
		int data_index = 0; // used for calculating data address
		int text_index = 0; // used for calculating text address
		int isData;

		while (1) {
			scanf("%s", str);
			if (feof(fp)) break;

			if (!strcmp(str, ".data")) {
				isData = 1;
			}
			else if (!strcmp(str, ".text")) {
				isData = 0;
			}

			else if (str[strlen(str)-1] == ':') {
				char *label = substr(str, 0, strlen(str)-1);
				
				label_array[label_index] = label;
				if (isData) address_array[label_index] = data_address + 4*data_index;
				else address_array[label_index] = text_address + 4*text_index;
				label_index++;
			}

			else if (!strcmp(str, ".word")) {
				scanf("%s", str);
				
				int data;
				if (check_hex(str)) data = (int)strtol(str, NULL, 0);
				else data = atoi(str);
				//data_array[data_index] = data;
				strcat(val, int_to_binary(data));
				data_index++;
				//data_size += 4;
			}

			else {
				int opcode;
				int rs;
				int rt;
				int rd;
				int sh;
				int func;
				int imm;
				int addr;
				
				if (!strcmp(str, "addiu")) {
					scanf("%s", str);
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "addu")) {
					scanf("%s", str);
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "and")) {
					scanf("%s", str);
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "andi")) {
					scanf("%s", str);
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "beq")) {
					scanf("%s", str);
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "bne")) {
					scanf("%s", str);
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "j")) {
					scanf("%s", str);
				}
				else if (!strcmp(str, "jal")) {
					scanf("%s", str);
				}
				else if (!strcmp(str, "jr")) {
					scanf("%s", str);
				}
				else if (!strcmp(str, "lui")) {
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "lw")) {
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "la")) {
					scanf("%s", str);
					scanf("%s", str);
					int idx = find(label_array, str, label_index);
					int add = address_array[idx];
					int lower = add & 0xffff;
					
					if (lower != 0) {
						text_index++;
					}
				}
				else if (!strcmp(str, "nor")) {
					scanf("%s", str);
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "or")) {
					scanf("%s", str);
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "ori")) {
					scanf("%s", str);
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "sltiu")) {
					scanf("%s", str);
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "sltu")) {
					scanf("%s", str);
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "sll")) {
					scanf("%s", str);
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "srl")) {
					scanf("%s", str);
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "sw")) {
					scanf("%s", str);
					scanf("%s", str);
				}
				else if (!strcmp(str, "subu")) {
					scanf("%s", str);
					scanf("%s", str);
					scanf("%s", str);
				}
				text_index++;
				//text_size += 4;
			}

		}

		if((fp = freopen(file, "r",stdin))==0){
			printf("File open Error!\n");
			exit(1);
		}

		
		text_index = 0;
		while (1) {
			scanf("%s", str);
			if (feof(fp)) break;

			
			int opcode;
			int rs;
			int rt;
			int rd;
			int sh;
			int func;
			int imm;
			int addr;
			
			if (!strcmp(str, "addiu")) {
				opcode = 0x9;
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rs = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				if (check_hex(str)) imm = (int)strtol(str, NULL, 0);
				else imm = atoi(str);
				strcat(inst, int_to_binary((opcode << 26) + (rs << 21) + (rt << 16) + imm));
			}
			else if (!strcmp(str, "addu")) {
				opcode = 0;
				func = 0x21;
				sh = 0;
				scanf("%s", str);
				rd = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rs = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)));
				strcat(inst, int_to_binary((opcode << 26) + (rs << 21) + (rt << 16) + (rd << 11) 
						+ (sh << 6) + func));
			}
			else if (!strcmp(str, "and")) {
				opcode = 0;
				func = 0x24;
				sh = 0;
				scanf("%s", str);
				rd = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rs = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)));
				strcat(inst, int_to_binary((opcode << 26) + (rs << 21) + (rt << 16) + (rd << 11) 
						+ (sh << 6) + func));
			}
			else if (!strcmp(str, "andi")) {
				opcode = 0xc;
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rs = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				if (check_hex(str)) imm = (int)strtol(str, NULL, 0);
				else imm = atoi(str);
				strcat(inst, int_to_binary((opcode << 26) + (rs << 21) + (rt << 16) + imm));
			}
			else if (!strcmp(str, "beq")) {
				opcode = 0x4;
				scanf("%s", str);
				rs = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				int idx = find(label_array, str, label_index);
				int add = address_array[idx];
				imm = (add - (text_address + 4*text_index + 4)) >> 2;
				strcat(inst, short_to_binary((opcode << 10) + (rs << 5) + rt));
				strcat(inst, short_to_binary((short)imm));
			}
			else if (!strcmp(str, "bne")) {
				opcode = 0x5;
				scanf("%s", str);
				rs = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				int idx = find(label_array, str, label_index);
				int add = address_array[idx];
				imm = (add - (text_address + 4*text_index + 4)) >> 2;
				strcat(inst, short_to_binary((opcode << 10) + (rs << 5) + rt));
				strcat(inst, short_to_binary((short)imm));
			}
			else if (!strcmp(str, "j")) {
				opcode = 0x2;
				scanf("%s", str);
				int idx = find(label_array, str, label_index);
				int add = address_array[idx];
				addr = add >> 2;
				strcat(inst, int_to_binary((opcode << 26) + addr));
			}
			else if (!strcmp(str, "jal")) {
				opcode = 0x3;
				scanf("%s", str);
				int idx = find(label_array, str, label_index);
				int add = address_array[idx];
				addr = add >> 2;
				strcat(inst, int_to_binary((opcode << 26) + addr));
			}
			else if (!strcmp(str, "jr")) {
				opcode = 0;
				func = 0x08;
				sh = 0;
				scanf("%s", str);
				rs = atoi(substr(str, 1, strlen(str)));
				rt = 0;
				rd = 0;
				strcat(inst, int_to_binary((opcode << 26) + (rs << 21) + (rt << 16) + (rd << 11) 
						+ (sh << 6) + func));
			}
			else if (!strcmp(str, "lui")) {
				opcode = 0xf;
				rs = 0;
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				if (check_hex(str)) imm = (int)strtol(str, NULL, 0);
				else imm = atoi(str);
				strcat(inst, short_to_binary((opcode << 10) + (rs << 5) + rt));
				strcat(inst, short_to_binary((short)imm));
			}
			else if (!strcmp(str, "lw")) {
				opcode = 0x23;
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				imm = atoi(substr(str, 0, find1(str)));
				rs = atoi(substr(str, find1(str)+2, find2(str)));
				strcat(inst, short_to_binary((opcode << 10) + (rs << 5) + rt));
				strcat(inst, short_to_binary((short)imm));
			}
			else if (!strcmp(str, "la")) {
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				int idx = find(label_array, str, label_index);
				int add = address_array[idx];
				int upper = (add >> 16) & 0xffff;
				int lower = add & 0xffff;
				
				opcode = 0xf;
				imm = upper;
				rs = 0;
				strcat(inst, short_to_binary((opcode << 10) + (rs << 5) + rt));
				strcat(inst, short_to_binary((short)imm));
				
				if (lower != 0) {
					opcode = 0xd;
					rs = rt;
					imm = lower;
					strcat(inst, short_to_binary((opcode << 10) + (rs << 5) + rt));
					strcat(inst, short_to_binary((short)imm));
					text_index++;
				}
			}
			else if (!strcmp(str, "nor")) {
				opcode = 0;
				func = 0x27;
				sh = 0;
				scanf("%s", str);
				rd = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rs = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)));
				strcat(inst, int_to_binary((opcode << 26) + (rs << 21) + (rt << 16) + (rd << 11) 
						+ (sh << 6) + func));
			}
			else if (!strcmp(str, "or")) {
				opcode = 0;
				func = 0x25;
				sh = 0;
				scanf("%s", str);
				rd = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rs = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)));
				strcat(inst, int_to_binary((opcode << 26) + (rs << 21) + (rt << 16) + (rd << 11) 
						+ (sh << 6) + func));
			}
			else if (!strcmp(str, "ori")) {
				opcode = 0xd;
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rs = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				if (check_hex(str)) imm = (int)strtol(str, NULL, 0);
				else imm = atoi(str);
				strcat(inst, short_to_binary((opcode << 10) + (rs << 5) + rt));
				strcat(inst, short_to_binary((short)imm));
			}
			else if (!strcmp(str, "sltiu")) {
				opcode = 0xb;
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rs = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				if (check_hex(str)) imm = (int)strtol(str, NULL, 0);
				else imm = atoi(str);
				strcat(inst, int_to_binary((opcode << 26) + (rs << 21) + (rt << 16) + imm));
			}
			else if (!strcmp(str, "sltu")) {
				opcode = 0;
				func = 0x2b;
				sh = 0;
				scanf("%s", str);
				rd = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rs = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)));
				strcat(inst, int_to_binary((opcode << 26) + (rs << 21) + (rt << 16) + (rd << 11) 
						+ (sh << 6) + func));
			}
			else if (!strcmp(str, "sll")) {
				opcode = 0;
				func = 0x00;
				scanf("%s", str);
				rd = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				sh = atoi(str);
				rs = 0;
				strcat(inst, int_to_binary((opcode << 26) + (rs << 21) + (rt << 16) + (rd << 11) 
						+ (sh << 6) + func));
			}
			else if (!strcmp(str, "srl")) {
				opcode = 0;
				func = 0x02;
				scanf("%s", str);
				rd = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				sh = atoi(str);
				rs = 0;
				strcat(inst, int_to_binary((opcode << 26) + (rs << 21) + (rt << 16) + (rd << 11) 
						+ (sh << 6) + func));
			}
			else if (!strcmp(str, "sw")) {
				opcode = 0x2b;
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				imm = atoi(substr(str, 0, find1(str)));
				rs = atoi(substr(str, find1(str)+2, find2(str)));
				strcat(inst, short_to_binary((opcode << 10) + (rs << 5) + rt));
				strcat(inst, short_to_binary((short)imm));
			}
			else if (!strcmp(str, "subu")) {
				opcode = 0;
				func = 0x23;
				sh = 0;
				scanf("%s", str);
				rd = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rs = atoi(substr(str, 1, strlen(str)-1));
				scanf("%s", str);
				rt = atoi(substr(str, 1, strlen(str)));
				strcat(inst, int_to_binary((opcode << 26) + (rs << 21) + (rt << 16) + (rd << 11) 
						+ (sh << 6) + func));
			}
			else {
				continue;
			}
			text_index++;
			//text_size += 4;
			

		}

		// For output file write 
		// You can see your code's output in the sample_input/example#.o 
		// So you can check what is the difference between your output and the answer directly if you see that file
		// make test command will compare your output with the answer
		file[strlen(file)-1] ='o';
		freopen(file,"w",stdout);

		//If you use printf from now on, the result will be written to the output file.

		strcat(answer, int_to_binary(text_index * 4));
		strcat(answer, int_to_binary(data_index * 4));
		strcat(answer, inst);
		strcat(answer, val);
		printf("%s", answer);
	}
	return 0;
}

