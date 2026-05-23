#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "filesystem_internal.h"

/* Funciotn empty_entry: Cluster -> Boolean
 * Dado o ponteiro de um cluster, retornar se é um cluster vazio.
 * empty_entry(entry);
 */
bool is_empty_folder(Cluster* folder) {
    return folder->u.data[0] == 0;
}

/* Funciotn empty_entry: Entry -> Boolean
 * Dado o ponteiro de uma entry, retornar se é um entry vazio.
 * empty_entry(entry);
 */
bool is_empty_entry(Entry* file) {
    return file->name[0] == 0;
}


/* Funciotn has_entry: Cluster Entry -> Boolean
 * Dado o ponteiro de um folder e um arquivo, verifica se existe no diretorio
 * empty_entry(entry);
 */
bool has_entry(Cluster* folder, char *filename){
    for(int i=0; i < ENTRY_IN_CLUSTER; i++)
        if(strcmp(folder->u.entry[i].name, filename) == 0)
            return true;
        else if(is_empty_entry(&folder->u.entry[i]))
            return false;
    return false;
}

/*
 * Function find_entry: Cluster String -> Entry
 * Dado o nome de um arquivo, procura-lo no cluster. 
 * NULL -> Não encontrou
 * Any -> Valor da referencia no cluster; 
 * find_entry(dir, "arquivo.txt") = cluster de "arquivo.txt"
 */
Entry* get_entry(Cluster* folder, char *filename){
    for(int i=0; i < ENTRY_IN_CLUSTER; i++)
        if(strcmp(folder->u.entry[i].name, filename) == 0)
            return &folder->u.entry[i];
        else if(is_empty_entry(&folder->u.entry[i]))
            return NULL;
    return NULL;
}


/*
 * Function insert_entry: Cluster Entry -> int
 * Dado um folder e um entry, inseri-lo no folder;
 * Retorna 1, sucesso;
 * Retorna 0, falha; Ja existe arquivo com o mesmo nome ou não há espaço
 */
int insert_entry(Cluster* folder, Entry newEntry){
    int sucess = 0;
    uint8_t frst = folder->fat;
    for(int i=0; i < ENTRY_IN_CLUSTER; i++){
        if(is_empty_entry(&folder->u.entry[i])){
            folder->u.entry[i] = newEntry;
            sucess = 1;
            break;
        }
        else if(strcmp(folder->u.entry[i].name, newEntry.name) == 0)
            break;
    }
    if(folder->fat != frst){
        get_cluster(folder, frst);
    }
    return sucess;     
}

/*
 * Function insert_entry: Cluster string -> int
 * Dado um entry, remove-lo do diretório dado.
 * Retorna 1, sucesso;
 * Retorna 0, falha; Não encontrou arquivo no folder
 */
int delete_entry(Cluster* folder, char *filename){
    int sucess = 0;
    uint8_t frst = folder->fat;
    Entry* found = get_entry(folder, filename);
    if(found != NULL){
        long unsigned int setIndex = found - folder->u.entry;
        Entry aux;
        memset(found, 0, sizeof(Entry));
        for(int i = setIndex; i < (ENTRY_IN_CLUSTER - 1); i++){
            if(is_empty_entry(&folder->u.entry[i+1]))
                break;
            else{
                aux = folder->u.entry[i]; 
                folder->u.entry[i] = folder->u.entry[i+1];
                folder->u.entry[i+1] = aux;
            }
        }
        sucess = 1;
    } 
    if(folder->fat != frst){
        get_cluster(folder, frst);
    }
    return sucess;
}

/*
 * walk_through_path: string -> path
 * Dado um path, caminha entre os diretórios do path. 
 * Retorna o cluster do ultimo diretório no path.
 * "/root/pasta1/pasta2" -> primeiro cluster de pasta2
 * "/root/pasta1/pasta2/root/arquivo.txt" -> NULL
 */
void walk_through_path(Cluster* folder, char* currpath) {
    Entry *entry;
    char tmp[PATH_LIMIT];
    char *next;
    uint8_t current;
    strcpy(tmp, currpath);
    get_cluster(folder, FAIL_CLU);
    if(strncmp(tmp, "/root", 5) == 0) {
        next = strtok(tmp, "/");
        current = 0;
        while(get_cluster(folder, current) && (next = strtok(NULL, "/")) != NULL) {
            entry = get_entry(folder, next);
            if(entry != NULL && entry->attribute == DIR_TYPE)
                current = entry->frstCluster;
            else
                current = FAIL_CLU;
        }
    }
}

/*
 * Function parse_path: STRING STRING STRING ->  
 * Dado um fullpath, dividi-lo em path e filename.
 */
int parse_path(char *fullpath, char *currpath, char *filename){
    char *tmp = strrchr(fullpath, '/');
	if (tmp != NULL){
    	*tmp = '\0';
    	strcpy(currpath, fullpath);
    	strcpy(filename, tmp+1);
    }
    return 0; 
}

/*
 *
 *
 */
bool is_valid_name(char* filename){
    if (strlen(filename) > NAME_LIMIT) {
        printf("[ERROR] filename limit is %d\n", NAME_LIMIT);
        return false;
    }

    if (strlen(filename) + strlen(path) > PATH_LIMIT) {
        printf("[ERROR] fullpath + filename ultrapass the path limit.\n");
        return false;
    } 
    
    for(int i=0; i<strlen(filename); i++) {
        if(!(filename[i] >= 'a' && filename[i] <= 'z')
        && !(filename[i] >= '0' && filename[i] <= '9')
        && !(filename[i] == '.')) {
            printf("[ERROR] filename only aceppts alphanumeric + '.'\n");
            return false;
        }
    }

    return true;
}
