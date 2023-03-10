#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


#ifndef STDERR
    # define STDOUT 1
#endif
#define PATHLENGTH 2048
#define MAX_FILENAME_LENGTH 10

char paths[512][PATHLENGTH];
int path_num = 0;

int readLine(int fd, char* line){
    char read_buf;
    int i=0;
    for(i=0;(read(fd,&read_buf,1) >0) && read_buf != '\n'; i++ ){
        line[i] = read_buf;
    }
    line[i] = '\0';
    return i;
}


int countKey(char* line, const char key){
    int counter = 0;
    for(int i=0;i<strlen(line);i++){
        if(line[i] == key){
            counter++;
        }
    }
    return counter;
}

int countLevel(char* line){
    int i=0;
    for(i=0;i<strlen(line) && line[i] != '+';i++){
    }
    if(i==strlen(line))
        return -1;
    else
        return i/4;
}

/*
    Did not check the overflow, use with caution.
*/
void strcat(char* str1, char* str2){
    memmove(str1+strlen(str1),str2, strlen(str2));
    str1[strlen(str1)] = '\0';
}

char* substr(char* str1, int header, int tail){
    char* newStr = malloc(sizeof(char)*(tail-header+1));
    memcpy(newStr,str1+header,tail-header);
    newStr[strlen(newStr)] = '\0';

    memmove(str1,newStr,strlen(newStr)+1);
    memset(newStr, '\0', sizeof(newStr));  //to clear garbage
    free(newStr);

    return str1;
}


int findIndex(const char* data, char key){
    int i=0;
    for(i=0;i<strlen(data);i++){
        if(data[i]==key)
            return i;
    }
    return -1;
}

void makePaths(int fd, char arg[]){
    char prefix[5][MAX_FILENAME_LENGTH+1];
    strcpy(prefix[0],arg);
    //int level = 0;
    char line[512];
    for(int i=0;readLine(fd,line) != 0;i++){ // if it is 0, then it is EOF.
        if(i==0){  // I handle header root seperately
            continue;
        }

        if(i%2==0){
            int level = countLevel(line);
            char path[512] = {'\0'};
            for(int j=0;j<level;j++){
                strcat(path, prefix[j]);
                memset(path+strlen(path)+1,'\0',1);
                memset(path+strlen(path),'/',1);
            }
            int file_name_index = findIndex(line,'+')+4;
            substr(line,file_name_index,strlen(line));
            strcpy(prefix[level],line);
            strcat(path,line);

            strcpy(paths[path_num++],path);
        }
    }


}

void main(int argc, char *argv[]) {
    int cpid, p1[2];
    pipe(p1);
    
    if ((cpid = fork()) < 0) {
        fprintf(2, "fork error\n");
        exit(1);
    }

    if (cpid == 0) { /* child process */
        //write to p1[0]
        close(p1[0]);
        close(STDOUT);
        
        dup(p1[1]);   //replaces stdout
        char* ex[3];
        ex[0] = "count";
        ex[1] = argv[1];
        exec("count",ex);
        exit(0);

    } else {        /* parent process */
        //read from p1[0]
        close(p1[1]);
        makePaths(p1[0], argv[1]);

        // paths shall be filled
        for(int i=0;i<path_num;i++){
            printf("%s\n",paths[i]);
        }
        //countPaths();

    }
    exit(0);
}