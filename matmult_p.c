#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ANROWS 0
#define ANCOLS 1
#define BNROWS 2
#define BNCOLS 3

#define LNGEST_NUM 10
#define LNGEST_ROWSIZE 50

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

void storeMats(char** A, char **B){
	
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
				A[r] = strdup(line);
			}else{
				B[r] = strdup(line);
			}
			r++;
		}
	}
	free(line);

}


int getForking(int sizes[4], char** aRows, char** bCols){
	
	int i,j;
	int shmid;
	char ci[sizeof(char)*32];
	char cj[sizeof(char)*32];
	char rsize[sizeof(char)*64];
	char strid[sizeof(char)*64];
	char shrdCsize[sizeof(char)*64];	
	sprintf(rsize, "%d", sizes[ANCOLS]);
	sprintf(shrdCsize, "%d", sizes[BNCOLS]);

	shmid = shmget(IPC_PRIVATE, sizeof(int)*sizes[ANROWS]*sizes[BNCOLS], IPC_CREAT | S_IRUSR | S_IWUSR );
	if(shmid == -1){
		printf("shmget failed");
		exit(0);
	}	
	sprintf(strid, "%d", shmid);	
	
	sprintf(ci, "%d", 0);
	sprintf(cj, "%d", 0);
	char *args[9] = {"multiply", strid, rsize, shrdCsize, aRows[0], bCols[0], ci, cj, NULL};
	
	pid_t childpid;
	for(i = 0; i < sizes[ANROWS]; i++){
		args[4] = aRows[i];
		sprintf(args[6], "%d", i);
		for(j = 0; j < sizes[BNCOLS]; j++){
			if( (childpid = fork()) == -1 ){ //FAILED	
				printf("child %d,%d failed!\n",i,j);
				exit(-1);
			}else if(childpid == 0){				
				args[5] = bCols[j];
				sprintf(args[7], "%d", j);
				execve("multiply", args, NULL);
			}else{ //parent
				waitpid(childpid,NULL,0);
			}
		}//end of j loop
	}//end of i loop
	

	return shmid;
}



int main(int argc, char *argv[]){
	
	char *tmp;
	int i, j;
	int sizes[4] = {0,0,0,0}; //ANROWS, ANCOLS, BNROWS, BNCOLS
	//Determine sizes of input matrices	
	getMatSizes(sizes);
	
	//printf("arow = %d, acol = %d, brow = %d, bcol = %d\n", sizes[ANROWS], sizes[ANCOLS], sizes[BNROWS], sizes[BNCOLS]);

	char *aRows[sizes[ANROWS]];
	char *bRows[sizes[BNROWS]];

	if(sizes[ANCOLS] != sizes[BNROWS]){
		printf("matrices can't be multiplied\n");
		return 0;
	}

	rewind(stdin);
	//store initial A and B mats into array of strings aRows and bRows
	//where each index is a row of A and B respectively
	storeMats(aRows, bRows);


	//allocates each index 
	char *bCols[sizes[BNCOLS]];	
	for(i = 0; i < sizes[BNCOLS]; i++){
		bCols[i] = malloc(sizeof(char)*LNGEST_NUM*LNGEST_ROWSIZE);
	}


	//Transpose B mat strings
	tmp = strdup(bRows[0]);
	for(j = 0; j < sizes[BNCOLS]; j++){
			if(j == 0)
				bCols[j] = strcat(bCols[j], strdup(strtok(tmp, " \n")));
			else{
				bCols[j] = strcat(bCols[j], strdup(strtok(NULL, " \n")));
			}
	}

	for(i = 1; i < sizes[BNROWS]; i++){
		tmp = strdup(bRows[i]);
		for(j = 0; j < sizes[BNCOLS]; j++){
			if(j == 0){
				bCols[j] = strcat(bCols[j], " ");
				bCols[j] = strcat(bCols[j], strdup(strtok(tmp, " \n")));
			}else{
				bCols[j] = strcat(bCols[j], " ");
				bCols[j] = strcat(bCols[j], strdup(strtok(NULL, " \n")));
			}		
		}
	}

	int shmid;
	shmid = getForking(sizes, aRows, bCols);
	int (*C)[sizes[BNCOLS]];
	C = shmat(shmid, NULL, 0);

	C[sizes[ANROWS]-1][sizes[BNCOLS]] = '\0';
	
	
	for(i = 0; i < sizes[ANROWS]; i++){
		for(j = 0; j < sizes[BNCOLS]; j++){
			printf("%d ", C[i][j]);
		}
		printf("\n");
	}
	
	//free each index of bCols
	for(i = 0; i < sizes[BNCOLS]; i++){
		free(bCols[i]);
	}

	free(tmp);
	shmdt(C);
	shmctl(shmid,IPC_RMID, NULL);

	return 0;
}












