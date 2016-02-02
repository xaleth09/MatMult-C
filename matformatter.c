#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define NROWS 0
#define NCOLS 1
#define LNGEST_NUM 10
#define LNGEST_ROWSIZE 50
#define MAXROWS 200
#define MAXCOLS 200

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


void matOps(int nsizes[2], char** A){

	char *line = NULL; 
	int ind = NROWS;
	int isfirstline = 1;
	int r = 0;
	size_t s = 0;
	ssize_t read = 0;
    while( (read = getline(&line, &s, stdin)) != -1 ){
			
		A[r++] = strdup(line);

			if( isfirstline ){
				isfirstline = 0;
				nsizes[ind + 1] = getrowsize(line);
			}
				nsizes[ind] = nsizes[ind]++;
					
	}	

	free(line);

}


int main(int argc, char* argv[]){
	int i,j;	
	int sizes[2] = {0,0};
	
	char *tmp;
	char *matRows[MAXROWS];
	char *matCols[MAXCOLS];
	matOps(sizes, matRows);
	
	for(i = 0; i < sizes[NCOLS]; i++){
		matCols[i] = malloc(sizeof(char)*LNGEST_NUM*LNGEST_ROWSIZE);
	}

	//printf("NROWS = %d, NCOLS = %d\n", sizes[NROWS], sizes[NCOLS]);
	
	//Transpose Mat
	tmp = strdup(matRows[0]);
	for(j = 0; j < sizes[NCOLS]; j++){
			if(j == 0)
				matCols[j] = strcat(matCols[j], strdup(strtok(tmp, " \n")));
			else{
				matCols[j] = strcat(matCols[j], strdup(strtok(NULL, " \n")));
			}
	}

	for(i = 1; i < sizes[NROWS]; i++){
		tmp = strdup(matRows[i]);
		for(j = 0; j < sizes[NCOLS]; j++){
			if(j == 0){
				matCols[j] = strcat(matCols[j], " ");
				matCols[j] = strcat(matCols[j], strdup(strtok(tmp, " \n")));
			}else{
				matCols[j] = strcat(matCols[j], " ");
				matCols[j] = strcat(matCols[j], strdup(strtok(NULL, " \n")));
			}		
		}
	}

		
	for(i = 0; i < sizes[NCOLS]; i++){
		printf("%s\n", matCols[i]);
	}

	return 0;
}






