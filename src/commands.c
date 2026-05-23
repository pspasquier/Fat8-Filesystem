#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "filesystem_internal.h"

/*
 * function cd: String -> Void 
 * Dado um fullpath, verificar se este caminho
 * existe no arquivo de sistema. Se existir, então,
 * torna-lo o path atual. 
 * cd("/root/folder1/folder2")
 */
void cd(char* fullpath) {
    Cluster* folder = (Cluster*)malloc(sizeof(Cluster));
    if(strlen(fullpath) > PATH_LIMIT)
        printf("[ERROR] path is limited to %d\n", PATH_LIMIT);
    else{
        walk_through_path(folder, fullpath);
        if(is_valid_cluster(folder)){
            strcpy(path, fullpath);
            free(folder);
            folder = NULL;
        }
        else
            printf("[ERROR] No such directory\n");
    }
    free(folder);
}

void size(){
    Cluster* folder = (Cluster*)malloc(sizeof(Cluster));
    walk_through_path(folder, path);
    uint32_t Lsize = logical_size (folder);
    uint32_t Psize = physical_size(folder);
    printf("Logical size of %s: %u B\n",  path, Lsize);
    printf("Pyshical size of %s: %u B\n", path, Psize);
    free(folder);
}

/* 
 * Function dir: VOID -> VOID
 * dado um sistema de arquivos, imprimir lista com os arquivos e diretórios
 * que estão dentro do diretório atual.
 * dir();
 */
void dir() {
    Cluster* folder = (Cluster*)malloc(sizeof(Cluster));
    walk_through_path(folder, path);
    if(is_empty_folder(folder)) {
        printf("<vazio>\n");
    } else {
        for(int i=0; (i<ENTRY_IN_CLUSTER) && (!is_empty_entry(&folder->u.entry[i])); i++){
            if(folder->u.entry[i].attribute == DIR_TYPE)
                printf("%s%s%s", GREEN, folder->u.entry[i].name, RESET);
            else
                printf("%s   %u", folder->u.entry[i].name, folder->u.entry[i].size);
            printf("\n");
        }
        printf("\n");
    }
    free(folder);
    folder = NULL;
    size();
}

/* 
 * Function edit: string string -> 
 * Dado um arquivo e dados, insere dados no cluster do arquivo.
 * 
 * 
 */
void edit(char* fullpath, char* data) {
    Cluster* folder = (Cluster*)malloc(sizeof(Cluster));
    Cluster* aux = (Cluster*)malloc(sizeof(Cluster));
    Entry* file;
    char currpath[PATH_LIMIT + 1];
    char filename[PATH_LIMIT + 1];
    uint8_t next;
    //int requiredCluster = (strlen(data) + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
    if(strlen(fullpath) > PATH_LIMIT)
        printf("[ERROR] path limit is %d", NAME_LIMIT);
    else{
        parse_path(fullpath, currpath, filename);
        walk_through_path(folder, currpath);
        file = get_entry(folder, filename);
        if(is_valid_cluster(folder) 
        && (file != NULL && file->attribute == FILE_TYPE)){
            get_cluster(aux, file->frstCluster);
            clear_chain(aux);
            aux->next = EOF_CLU;
            fat[aux->fat] = EOF_CLU;
            file->size = 0;
            for(int i=0; i<strlen(data); i++){
                aux->u.data[i % CLUSTER_SIZE] = data[i];
                file->size++;
                if(file->size % (CLUSTER_SIZE) == 0 && i != 0){
                    next = frst_free_cluster();
                    if(next != FAIL_CLU) {
                        aux->next = next;
                        update_cluster(aux);
                        get_cluster(aux, next);
                        aux->next = EOF_CLU;
                        fat[next] = EOF_CLU;
                    }
                    else {
                        if(file->size < strlen(data))
                            printf("[ERROR] cannot insert all data.\n");
                        aux->next = EOF_CLU;
                        break;
                    }
                }
            }    
            update_cluster(aux);
            update_cluster(folder);
        }
        else
            printf("[ERROR] file not found.\n");
    }
    free(folder); free(aux);
    folder = NULL; aux = NULL; file = NULL;
}


/* 
 * function mkdir: STRING -> VOID
 * Dado o nome de um diretório, criar o diretório no path atual.
 * Caso o nome do arquivo não se adeque ou não tenha espaço na memória 
 * para o cluster, então não criar.
 * mkdir()
 */
void mkdir(char* filename) {
    Cluster* currfolder = (Cluster*)malloc(sizeof(Cluster));
    Cluster* destCLU = (Cluster*)malloc(sizeof(Cluster));
    Entry newEntry;
    walk_through_path(currfolder, path);
    get_cluster(destCLU, frst_free_cluster());
    if (!is_valid_name(filename) && is_valid_cluster(destCLU)){
        // Do nothing
    } else if (is_valid_cluster(destCLU)) {
        strcpy(newEntry.name, filename);
        newEntry.attribute = DIR_TYPE;
        newEntry.frstCluster = destCLU->fat;
        newEntry.size = 0;
        destCLU->next = EOF_CLU;
        destCLU->u.data[0] = 0;
        if(insert_entry(currfolder, newEntry)){
            update_cluster(currfolder);
            update_cluster(destCLU);
        } else {
            printf("[ERROR] directory alredy exist or has no free space.\n");
        }
    } else {
        printf("[ERROR] has no free space.\n");
    }
    free(currfolder); free(destCLU);
    currfolder = NULL; destCLU = NULL;
}


/* 
 * function mkdir: STRING -> VOID
 * Dado o nome de um diretório, criar o diretório no path atual.
 * Caso o nome do arquivo não se adeque ou não tenha espaço na memória 
 * para o cluster, então não criar.
 * mkdir()
 */
void mkfile(char* filename) {
    Cluster* currfolder = (Cluster*)malloc(sizeof(Cluster));
    Cluster* destCLU = (Cluster*)malloc(sizeof(Cluster));
    Entry newEntry;
    walk_through_path(currfolder, path);
    get_cluster(destCLU, frst_free_cluster());
    if(!is_valid_name(filename)){
        // Do nothing
    } else if(is_valid_cluster(destCLU)){
        strcpy(newEntry.name, filename);
        newEntry.attribute = FILE_TYPE;
        newEntry.frstCluster = destCLU->fat;
        newEntry.size = 0;
        destCLU->next = EOF_CLU;
        destCLU->u.data[0] = 0;
        if(insert_entry(currfolder, newEntry)){
            update_cluster(currfolder);
            update_cluster(destCLU);
        } else {
            printf("[ERROR] file alredy exist or has no free space.\n");
        }
    } else {
        printf("[ERROR] has no free space.\n");
    }
    free(currfolder); free(destCLU);
    currfolder = NULL; destCLU = NULL;
}


void fs_rename(char* fullpath, char* newName){
    Cluster* folder = (Cluster*)malloc(sizeof(Cluster));
    Entry* file;
    char currpath[PATH_LIMIT + 1];
    char filename[NAME_LIMIT + 1];
    if (!is_valid_name(newName)) {
        // Do Nothing
    } else if (strcmp(fullpath, "/root") == 0) {
        printf("[ERROR] cannot rename root directory!\n");
    } else if (strncmp(fullpath, path, strlen(fullpath)) == 0) {
        printf("[ERROR] cannot remove %s. Current path is a subdirectory.\n", fullpath);
    } else {
        parse_path(fullpath, currpath, filename);
        walk_through_path(folder, currpath);
        if(is_valid_cluster(folder) && !has_entry(folder, newName)
        && (file = get_entry(folder, filename)) != NULL){
            strcpy(file->name, newName);
            update_cluster(folder);
        } else {
            printf("[ERROR] failed to rename.\n");
        }
    }
    free(folder);
    folder = NULL; file = NULL; 
}

/* 
 * function move: String String -> Void
 * Mover uma entry do path orig para o path dest
 * entry precisa existir no orig e não existir no dest
 * move("/root/pasta1/abc.txt", "/root");
 */
void move(char* orig, char* dest) {
    Cluster* folderOrig = (Cluster*)malloc(sizeof(Cluster));
    Cluster* folderDest = (Cluster*)malloc(sizeof(Cluster));
    Entry* entryOrig;
    Entry entryDest;
    char pathOrig[PATH_LIMIT + 1];
    char filename[NAME_LIMIT + 1];
    if(strlen(orig) > PATH_LIMIT && strlen(dest) > PATH_LIMIT)
        printf("[ERROR] path is limited to %d\n", PATH_LIMIT);
    else if(strcmp(orig, "/root") == 0)
        printf("[ERROR] cannot move root directory!\n");
    else if(strncmp(orig, path, strlen(orig)) == 0)
        printf("[ERROR] cannot move %s. Current path is a subdirectory.\n", orig);
    else{
        parse_path(orig, pathOrig, filename);
        walk_through_path(folderOrig, pathOrig);
        walk_through_path(folderDest, dest);
        if(is_valid_cluster(folderOrig) && is_valid_cluster(folderDest) 
        && (entryOrig = get_entry(folderOrig, filename)) != NULL
        && !has_entry(folderDest, filename)) {
            memcpy(&entryDest, entryOrig, sizeof(Entry));
            if(insert_entry(folderDest, entryDest)){
                delete_entry(folderOrig, filename);
                update_cluster(folderOrig);
                update_cluster(folderDest);
            }
        }
        else
            printf("[ERROR] failed to move.\n");
    }
    free(folderOrig); free(folderDest); 
    folderOrig = NULL; folderDest = NULL; entryOrig = NULL;
    return;
}

/*
 * function rm: String -> Void
 * remove um file de um diretório:
 *      FILE -> deleta do diretório
 *      DIR -> Deleta do diretório, somente se for um diretório vazio
 */
void rm(char* fullpath){
    Cluster* folder = (Cluster*)malloc(sizeof(Cluster));
    Cluster* aux = (Cluster*)malloc(sizeof(Cluster));
    Entry* file;
    char currpath[PATH_LIMIT + 1];
    char filename[NAME_LIMIT+ 1];
    if(strlen(fullpath) > PATH_LIMIT)
        printf("[ERROR] path limit is %d\n", PATH_LIMIT);
    else if(strcmp(fullpath, "/root") == 0)
        printf("[ERROR] cannot remove root directory!\n");
    else if(strncmp(fullpath, path, strlen(fullpath)) == 0)
        printf("[ERROR] cannot remove %s. Current path is a subdirectory.\n", fullpath);
    else{
        parse_path(fullpath, currpath, filename);
        walk_through_path(folder, currpath);
        file = get_entry(folder, filename);
        if(is_valid_cluster(folder) && file != NULL){
            get_cluster(aux, file->frstCluster);
            if((file->attribute == DIR_TYPE && !is_empty_folder(aux))){
                printf("[ERROR] directory is not empty!\n");
            }
            else{
                delete_entry(folder, file->name);
                update_cluster(folder);
                clear_chain(aux);
            }
        }
        else{
            printf("[ERROR] path %s/%s not found.\n", currpath, filename);
        }
    }
    free(folder); free(aux);
    folder = NULL;  
}

/*
 * 
 * ISFILE -> RM(FULLPATH)
 * ISDIR -> ENTRA E REPETE O PROCESSO
 * 
 */
void rf(char* fullpath){
    Cluster* folder = (Cluster*)malloc(sizeof(Cluster));
    char currpath[PATH_LIMIT + 1];
    char filename[NAME_LIMIT+ 1];
    Entry* file;
    uint8_t current;
    int len;
    if(strlen(fullpath) > PATH_LIMIT)
        printf("[ERROR] path limit is %d\n", PATH_LIMIT);
    else if(strcmp(fullpath, "/root") == 0)
        printf("[ERROR] cannot remove root directory!\n");
    else{
        parse_path(fullpath, currpath, filename);
        walk_through_path(folder, currpath);
        file = get_entry(folder, filename);
        strcat(fullpath, "/");
        strcat(fullpath, filename);
        if(file != NULL){
            if(file->attribute == FILE_TYPE)
                rm(fullpath);
            else {
                len = strlen(fullpath);
                current = folder->fat;
                get_cluster(folder, file->frstCluster);
                for(int i=0; (i<ENTRY_IN_CLUSTER) && (!is_empty_entry(&folder->u.entry[i])); i++){
                    strcat(fullpath, "/");
                    strcat(fullpath, folder->u.entry[i].name);
                    rf(fullpath);
                    fullpath[len] = 0;
                }
                get_cluster(folder, current);
                rm(fullpath);
            }
        }
    }
    free(folder);
    folder = NULL;
}

/* 
 * Function dir: VOID -> VOID
 * dado um sistema de arquivos, imprimir lista com os arquivos e diretórios
 * que estão dentro do diretório atual.
 * dir();
 */
static int defrag_helper(Cluster* folder) {
    uint8_t current = folder->fat;
    int moved = 0;
    for(int i=0; (i<ENTRY_IN_CLUSTER) && (!is_empty_entry(&folder->u.entry[i])); i++) {
        folder->u.entry[i].frstCluster = swap_cluster_in_chain(folder->u.entry[i].frstCluster, &moved);
        update_cluster(folder);
        if(folder->u.entry[i].attribute == DIR_TYPE){
            get_cluster(folder, folder->u.entry[i].frstCluster);
            moved += defrag_helper(folder);
            get_cluster(folder, current);
        }
    }
    return moved;
}

void defrag(){
    Cluster* folder = (Cluster*)malloc(sizeof(Cluster));
    get_cluster(folder, ROOT_CLUSTER);
    printf("clusters moved in defrag: %i\n", defrag_helper(folder));
    free(folder);
}