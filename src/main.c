#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "filesystem.h"
#define clear() printf("\033[H\033[J");
#define INPUT_MAX 2009
#define	CD 5862188
#define DIR 193453540
#define DISK 6383966928
#define EDIT 6383997099
#define EXIT 6384018879
#define CLEAR 210669815020
#define MKDIR 210681637468
#define MKFILE 6952494108189
#define MOVE 6384296988
#define RENAME 6952682948445
#define RM 5862692
#define RF 5862685
#define DEFRAG 6952134783566

char* toUpper(char* s){
    for(char *p=s; *p; p++) *p=toupper(*p);
    return s;
}

char* toLower(char* s){
	for(char *p=s; *p; p++) *p=tolower(*p);
    return s;
}

/*
** Objetive: given a string, return a hash of 4 bytes. 
*/
unsigned long hash(const char *str) {
    unsigned long hash = 5381;  
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

void terminal(){
    clear();
	char input[INPUT_MAX + 1];
	char *command;
	char *arg1;
	char *arg2;
	do{
		printf("\033[1;32m%s:\033[1;36m%s\033[0m$ ", get_diskname(), get_path());
		fflush(stdin);
    	fgets(input, INPUT_MAX, stdin);
        input[strcspn(input, "\n")] = 0;
		command = toUpper(strtok(input, " "));
		arg1    = strtok(NULL, " ");
		switch(hash(command)) {
			case CD:
				if(arg1 != NULL){
					arg1 = toLower(arg1);
					cd(arg1);
				}
			 	break;
			case DIR:
				dir();
				break;
			case DISK:
			 	size();
			 	break;
			case RM:
				if(arg1 != NULL){
					arg1 = toLower(arg1);
					rm(arg1);
				}
			  	break;
			case MKDIR:
				if(arg1 != NULL){
					arg1 = toLower(arg1);
					mkdir(arg1);
				}
			 	break;
			case MKFILE:
				if(arg1 != NULL){
					arg1 = toLower(arg1);
					mkfile(arg1);
				}
			  	break;
			case EDIT:
				arg2 = strtok(NULL, "\"");
				if(arg1 != NULL && arg2 != NULL){
					arg1 = toLower(arg1);
					edit(arg1, arg2);
				}
			 	break;
			case MOVE:
				arg2 = strtok(NULL, " ");
				if(arg1 != NULL && arg2 != NULL){
					arg1 = toLower(arg1);
					arg2 = toLower(arg2);
					move(arg1, arg2);
				}
			  	break;
			case RENAME:
				arg2 = strtok(NULL, " ");
				if(arg1 != NULL && arg2 != NULL){
					arg1 = toLower(arg1);
					arg2 = toLower(arg2);
					fs_rename(arg1, arg2);
				}	
				break;
			case RF:
				if(arg1 != NULL){
					arg1 = toLower(arg1);
					rf(arg1);
				}
				break;
			case DEFRAG:
				defrag();
				break;
			case CLEAR:
				clear();
				break;
			case EXIT:
				printf("Logout\n");
				break;
			default:
				printf("[ERROR] '%s' is not a valid command.\n", command);
			}
	}while(hash(command) != EXIT);
	return;
}


int main(int argc, char const *argv[]) {
    if((argc == 3) && (!strcmp("init\0", argv[1]))) {
        init_fs(argv[2]);
        printf("Disco inicializado.\n");
    } else if((argc == 3) && (!strcmp("boot\0", argv[1])) && (open_fs(argv[2]))) {
        terminal();
        close_fs();
    } else {
        printf("Um seguintes comandos são necessarios: \n");
        printf("-init [diskname]\n");
        printf("-boot [diskname]\n");  
	}
    return 0;
}