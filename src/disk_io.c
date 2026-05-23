#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "filesystem_internal.h"

/*
 * Requisitos do sistema:
 * Requer espaço na memório suficiente para as váriaveis do sistema
 * e clusters necessários para a execução dos comandos.
 */
FILE*       disk;
MetaData    info;
uint8_t     fat [FAT_SIZE];
char        path[PATH_LIMIT + 1];
const char* diskname;

/* 
 * Function init_fs: STRING -> VOID 
 * Inicializar um arquivo de sistemas com o nome dado.
 * init_fs("disk0");
 */
int init_fs(const char* disk_name){
    int sucess = 0;
    disk = fopen(disk_name, "wb+");
    if(disk != NULL){
        Cluster* a = (Cluster*)malloc(sizeof(Cluster));

        info.fatSize     = FAT_SIZE;
        info.clusterSize = CLUSTER_SIZE;   
        info.fatRegion   = sizeof(MetaData); 
        info.dataRegion  = sizeof(MetaData) + sizeof(fat);

        memset(&fat, FREE_CLU, FAT_SIZE); 
        
        fat[ROOT_CLUSTER] = EOF_CLU; 
        fat[FAIL_CLU]     = FAIL_CLU;
        fat[EOF_CLU]      = FAIL_CLU;
        
        memset(&(a->u.data), 0, CLUSTER_SIZE);
        fseek (disk, 0, SEEK_SET);
        fwrite(&info, sizeof(MetaData), 1, disk);
        fwrite(&fat, FAT_SIZE, 1, disk);

        for(int i = 0; i < FAT_SIZE; i++)
            fwrite(&(a->u.data), CLUSTER_SIZE, 1, disk);
    
        fclose(disk);
        free(a);
        sucess = 1;
    }

    return sucess;
}

/* 
 * Function open_fs: STRING -> INT
 * Dado o nome de um arquivo, abrir no formato fat. O arquivo precisa existir. 
 * Retorna o resultado da operação: 1->Sucesso; 0->Fracasso 
 * open_fs("disk0");
 */
int open_fs(const char* disk_name) {
    int sucess = 0;
    
    disk = fopen(disk_name, "rb+");
    if(disk != NULL) {
        fread (&info, sizeof(MetaData), 1, disk);
        fread (&fat, sizeof(fat), 1, disk);
        strcpy(path, "/root");
        diskname = disk_name;
        sucess   = 1;
    }

    return sucess;
}

/* 
 * Function close_filesystem: VOID -> VOID
 * Fechar o arquivo do filesystem.
 * close_filesystem();
 */
void close_fs() {
    fclose(disk);
}

/* 
 * Function get_disk: VOID -> STRING
 * Retorna o nome do disco.
 * WARNING: Desta forma o usuario pode mudar o conteúdo de disco. 
 *          Devolver uma cópia é melhor. 
 * get_disco();
 */
const char* get_diskname(){
    return diskname;
}


/* 
 * Function get_path: VOID -> STRING
 * Retorna o Current Path.
 * WARNING: Desta forma o usuario pode mudar o conteúdo de path. 
 *          Devolver uma cópia é melhor. 
 * get_path();
 */
char* get_path(){
    return path;
}

/* 
 * Function update_fat: VOID -> VOID
 * Atualiza a tabela fat
 * update_fat();
 */
void update_fat(){
    fseek (disk, info.fatRegion, SEEK_SET);
    fwrite(&fat, FAT_SIZE, 1, disk);
}

/* 
 * Function next: BYTE -> BYTE
 * Retorna o valor do Ponteiro de um cluster.
 * EOF -> 0xFF
 */
uint8_t point_to(uint8_t index) {
    return fat[index];
}

/* 
 * Function get_cluster: CLUSTER BYTE  -> INT
 * Dado o valor de um cluster e o index da fat, 
 * carregar o cluster em memória.
 * Retorna o resultado da operação: 1->Sucesso; 0->Fracasso 
 * sucess = get_cluster(cluster, 2);
 */
int get_cluster(Cluster* a, uint8_t index) {
    int sucess = 0;
    a->fat = FAIL_CLU;

    if(index != FAIL_CLU && index != EOF_CLU){
        a->fat = index;
        a->next = point_to(index);
        fseek(disk, info.dataRegion + (CLUSTER_SIZE * index), SEEK_SET);
        fread(&(a->u.data), CLUSTER_SIZE, 1, disk);
        sucess = 1;
    }

    return sucess;
}

/* 
 * Function update_cluster: CLUSTER BYTE -> INT
 * dado o index da fat e um cluster, descarregar em disco
 * o cluster no valor referente ao index
 * Retorna o resultado da operação: 1->Sucesso; 0->Fracasso 
 * sucess = update_cluster(2);
 */
int update_cluster(Cluster* a) {
    int sucess = 0;
    
    if(a->fat != FAIL_CLU && a->fat != EOF_CLU){
        fseek(disk, info.dataRegion + (CLUSTER_SIZE * a->fat), SEEK_SET);
        fwrite(&(a->u.data), CLUSTER_SIZE, 1, disk);
        fat[a->fat] = a->next;
        update_fat();
        return 1;
    }
    
    return sucess;
}