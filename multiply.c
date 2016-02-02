#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>


void fillRow(int *A,char* nline){
	int c = 0;
	A[c++] = atoi(strtok(nline, " \n") );
	while( (nline = strtok(NULL, " \n")) != NULL ){
		A[c++] = atoi(nline);
	}

}

// args[0]	 args[1]  args[2] args[3] args[4] args[5]  6    7
//"multiply", strid,  rsize, shrdCsize, aR,    bC,  , ci , cj NULL};

int main(int argc, char* argv[]){
	int shmid = atoi(argv[1]);
	int rsize = atoi(argv[2]);
	int numCols = atoi(argv[3]);
	int ci = atoi(argv[6]);
	int cj = atoi(argv[7]);
	int (*arr)[numCols];
	int ar[numCols]; //input aRow
	int bc[numCols]; //input bCol

	fillRow(ar, argv[4]);
	fillRow(bc, argv[5]);

	arr = shmat(shmid, NULL, 0);


	int tmp;
	int i;
	tmp = 0;
	for(i = 0; i < rsize; i++){
		tmp += ar[i]*bc[i];
	}
	
	arr[ci][cj] = tmp;
	
	shmdt(arr);

	return 0;
}
