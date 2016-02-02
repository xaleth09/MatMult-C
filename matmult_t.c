#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ucontext.h>

#define MAXSTACKSIZE 2048

#define ANROWS 0
#define ANCOLS 1
#define BNROWS 2
#define BNCOLS 3

#define LNGEST_NUM 10
#define LNGEST_ROWSIZE 50

static ucontext_t uc[LNGEST_ROWSIZE][LNGEST_ROWSIZE];

int getrowsize(char *line){
	int rsize = 1;	
	char *cpy;
	cpy = strdup(line);
	
	strtok(cpy, " \n");

	while( strtok(NULL, " \n") ){
		rsize++;
	}


	free(cpy);
	return rsize;
}


void getMatSizes(int nsizes[4]){

	char *line = NULL; 
	int ind = ANROWS;
	int lastRowSize = 0;
	int isfirstline = 1;
	size_t s = 0;
	ssize_t read = 0;
    while( (read = getline(&line, &s, stdin)) != -1 ){
		if(strcmp(line,"\n") == 0){
			ind = BNROWS;
			isfirstline = 1;
		}else{
			if( isfirstline ){
				isfirstline = 0;
				lastRowSize = getrowsize(line);
				nsizes[ind + 1] = lastRowSize;
			}else{
				nsizes[ind+1] = getrowsize(line);
				if(nsizes[ind+1] != lastRowSize){
					printf("inconsistent row sizes\n");				
					exit(-1);
				}
			}
			nsizes[ind] = nsizes[ind]++;
		}
	}

	free(line);
}

void matMalloc(int **A, int nrows, int ncols){
	int i;

	for(i = 0; i < nrows; i++){
		A[i] = malloc(ncols*sizeof(int));
	}


}

void fillRow(int *A,char* nline){
	int c = 0;
	A[c++] = atoi(strtok(nline, " \n") );
	while( (nline = strtok(NULL, " \n")) != NULL ){
		A[c++] = atoi(nline);
	}

}

void storeMats(int** A, int **B){
	
	char *line = NULL;
	size_t s = 0;
	int r = 0;
	int firstMat = 1;
    while( getline(&line, &s, stdin) != -1 ){
		if(strcmp(line,"\n") == 0){
			r = 0;
			firstMat = 0;
		}else{
	
			if(firstMat){
				fillRow(A[r], line);
			}else{
				fillRow(B[r], line);
			}
			r++;
		}
	}
	free(line);

}

int multiply(int *A, int *B, int rsize){
int tmp;
	int i;
	tmp = 0;
	for(i = 0; i < rsize; i++){
		tmp += A[i]*B[i];
	}

	return tmp;
}

static void f(int *A, int *B, int **C, int nrows, int rsize, int i, int j){
	
	int tmp;
	//tmp = multiply(A,B,rsize);
	//C[i][j-1] = tmp;
	//printf("%d\n", rsize);
}


int main(int argc, char *argv[]){
	
	int **A = NULL;
	int **B = NULL;
	int **C = NULL;
	int i, j;
	int sizes[4] = {0,0,0,0}; //ANROWS, ANCOLS, BNROWS, BNCOLS
	//Determine sizes of input matrices	
	getMatSizes(sizes);
	
	if(sizes[ANCOLS] != sizes[BNROWS]){
		printf("matrices can't be multiplied\n");
		return 0;
	}	
	

	char *stacks[(sizes[ANROWS]+1)*(sizes[BNCOLS]+1)];
	for(i = 0; i < (sizes[ANROWS]+1)*(sizes[BNCOLS]+1); i++){
		stacks[i] = malloc(sizeof(char)*MAXSTACKSIZE);
	}
	A = malloc(sizes[ANROWS]*sizeof(int*));
	B = malloc(sizes[BNROWS]*sizeof(int*));	
	C = malloc(sizes[ANROWS]*sizeof(int*));	
	

	matMalloc(A, sizes[ANROWS], sizes[ANCOLS]);
	matMalloc(B, sizes[BNROWS], sizes[BNCOLS]);
	matMalloc(C, sizes[ANROWS], sizes[BNCOLS]);
	 
	rewind(stdin);
	//store initial A and B mats into array of strings aRows and bRows
	//where each index is a row of A and B respectively
	storeMats(A, B);
	//printf("arow = %d, acol = %d, brow = %d, bcol = %d\n", sizes[ANROWS], sizes[ANCOLS], sizes[BNROWS], sizes[BNCOLS]);

	for(i = 0; i < sizes[ANROWS]; i++){
		for(j = 1; j < sizes[BNCOLS]+1; j++){		
			if(getcontext(&uc[i][j]) == -1){
				printf("failed to get context\n");				
				return -1;
			}

			uc[i][j].uc_link = &uc[0][0];
	 	 	uc[i][j].uc_stack.ss_sp = stacks[sizes[BNCOLS]*i+j];
	  		uc[i][j].uc_stack.ss_size = sizeof(stacks[i]);
			printf("%d,%d ",i,j);
	  		makecontext (&uc[i][j], (void (*) (void)) f, 7, A[i], B[j], C, sizes[ANROWS], sizes[BNCOLS]+1, i, j);
		}
		printf("\n");
	}	

	swapcontext(&uc[0][0], &uc[0][1]);

	
	
	return 0;
}












