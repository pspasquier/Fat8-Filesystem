#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "filesystem.h"

#define clear()   printf("\033[H\033[J")
#define INPUT_MAX 200

static void str_to_upper(char *s) {
    for (; *s; s++) *s = toupper(*s);
}

static void str_to_lower(char *s) {
    for (; *s; s++) *s = tolower(*s);
}

static void terminal(void) {
    char  input[INPUT_MAX + 1];
    char *cmd, *arg1, *arg2;
	int exit = 0;
	clear();

    while (exit == 0) {
        printf("\033[1;32m%s:\033[1;36m%s\033[0m$ ", get_diskname(), get_path());
        fflush(stdout);
        if (!fgets(input, INPUT_MAX, stdin))
			break;

        input[strcspn(input, "\n")] = 0;
        cmd  = strtok(input, " ");

        if (!cmd)
			continue;
        str_to_upper(cmd);

        if (strcmp(cmd, "CD") == 0) {
			arg1 = strtok(NULL, " ");
            if (arg1) { 
				str_to_lower(arg1);
				cd(arg1); 
			}
        } else if (strcmp(cmd, "DIR") == 0) {
            dir();
        } else if (strcmp(cmd, "DISK") == 0) {
            size();
        } else if (strcmp(cmd, "MKDIR") == 0) {
			arg1 = strtok(NULL, " ");
            if (arg1) {
				str_to_lower(arg1);
				mkdir(arg1);
			}
        } else if (strcmp(cmd, "MKFILE") == 0) {
			arg1 = strtok(NULL, " ");
            if (arg1) {
				str_to_lower(arg1);
				mkfile(arg1);
			}
        } else if (strcmp(cmd, "EDIT") == 0) {
			arg1 = strtok(NULL, " ");
            arg2 = strtok(NULL, "\"");
            if (arg1 && arg2) { 
				str_to_lower(arg1);
				edit(arg1, arg2);
			}
        } else if (strcmp(cmd, "MOVE") == 0) {
			arg1 = strtok(NULL, " ");
            arg2 = strtok(NULL, " ");
            if (arg1 && arg2) {
				str_to_lower(arg1);
				str_to_lower(arg2);
				move(arg1, arg2);
			}
        } else if (strcmp(cmd, "RENAME") == 0) {
			arg1 = strtok(NULL, " ");
            arg2 = strtok(NULL, " ");
            if (arg1 && arg2) {
				str_to_lower(arg1);
				str_to_lower(arg2);
				fs_rename(arg1, arg2);
			}
        } else if (strcmp(cmd, "RM") == 0) {
			arg1 = strtok(NULL, " ");
            if (arg1) {
				str_to_lower(arg1);
				rm(arg1);
			}
        } else if (strcmp(cmd, "RF") == 0) {
			arg1 = strtok(NULL, " ");
            if (arg1) {
				str_to_lower(arg1);
				rf(arg1);
			}
        } else if (strcmp(cmd, "DEFRAG") == 0) {
            defrag();
        } else if (strcmp(cmd, "CLEAR") == 0) {
            clear();
        } else if (strcmp(cmd, "EXIT") == 0) {
            printf("Logout\n");
			exit = 1;
        } else {
            printf("[ERROR] '%s' is not a valid command.\n", cmd);
        }
    }
}

int main(int argc, char const *argv[]) {
    if (argc == 3 && strcmp(argv[1], "init") == 0) {
        init_fs(argv[2]);
        printf("Disco inicializado.\n");
    } else if (argc == 3 && strcmp(argv[1], "boot") == 0 && open_fs(argv[2])) {
        terminal();
        close_fs();
    } else {
        printf("Uso:\n");
        printf("  ./fs init <diskname>\n");
        printf("  ./fs boot <diskname>\n");
    }
    return 0;
}