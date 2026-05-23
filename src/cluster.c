#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "filesystem_internal.h"

/* 
 * Function first_free_cluster: CLUSTER -> BYTE
 * Retornar o valor do primeiro cluster livre.
 * Caso não encontre um cluster livre, retorne 0xFE.
 * free = first_free_cluster(cluster);
 */
uint8_t frst_free_cluster() {
    for(int i = 1; i < FAT_SIZE; i++)
        if(fat[i] == FREE_CLU)
            return i;
    return FAIL_CLU;
}

/* 
 * Function clear_chain: CLUSTER -> VOID
 * Limpa uma chain na fat
 * clear_chain(cluster);
 */
void clear_chain(Cluster *a) {
    uint8_t init = a->fat;
    uint8_t next;
    if(init == FAIL_CLU || init == EOF_CLU)
        return;

    memset(&(a->u.data), 0, CLUSTER_SIZE);
    do {
        next = a->next;
        a->next = FREE_CLU;
        update_cluster(a);
        get_cluster(a, next);
    } while(next != EOF_CLU);

    a->fat = init;
}

/* 
 * Function first_free_cluster: CLUSTER -> BYTE
 * Retornar o valor do primeiro cluster livre.
 * Caso não encontre um cluster livre, retorne 0xFE.
 * free = first_free_cluster(cluster);
 */
void swap_clusters(uint8_t dest, uint8_t orig) {
    Cluster* destCLU = (Cluster*)malloc(sizeof(Cluster));
    Cluster* origCLU = (Cluster*)malloc(sizeof(Cluster));
    Cluster aux;
    uint8_t fatAux, fatAux2;
    get_cluster(destCLU, dest);
    get_cluster(origCLU, orig);
    fatAux = origCLU->fat;
    fatAux2 = destCLU->fat;
    memcpy(&aux, destCLU, sizeof(Cluster));
    memcpy(destCLU, origCLU, sizeof(Cluster));
    memcpy(origCLU, &aux, sizeof(Cluster));
    origCLU->fat = fatAux;
    destCLU->fat = fatAux2;
    update_cluster(destCLU);
    update_cluster(origCLU);
    free(destCLU);
    free(origCLU);
}

/* 
 * Function first_free_cluster: CLUSTER -> BYTE
 * Retornar o valor do primeiro cluster livre.
 * Caso não encontre um cluster livre, retorne 0xFE.
 * free = first_free_cluster(cluster);
 */
bool is_valid_cluster(Cluster* a) {
    return (a->fat != FAIL_CLU) && (a->fat != EOF_CLU);
}

/* 
 * Function dir: VOID -> VOID
 * dado um sistema de arquivos, imprimir lista com os arquivos e diretórios
 * que estão dentro do diretório atual.
 * dir();
 */
uint32_t logical_size(Cluster* folder){
    uint32_t sizeofFolder = 0;
    uint8_t current;
    if(!is_empty_folder(folder)){
        for(int i=0; (i<ENTRY_IN_CLUSTER) && (!is_empty_entry(&folder->u.entry[i])); i++){
            if(folder->u.entry[i].attribute == DIR_TYPE){
                current = folder->fat;
                get_cluster(folder, folder->u.entry[i].frstCluster);
                sizeofFolder += logical_size(folder);
                get_cluster(folder, current);
            }
            else
                sizeofFolder += folder->u.entry[i].size;
        }
    }
    return sizeofFolder;    
}

/* 
 * Function dir: VOID -> VOID
 * dado um sistema de arquivos, imprimir lista com os arquivos e diretórios
 * que estão dentro do diretório atual.
 * dir();
 */
uint32_t physical_size(Cluster* folder){
    uint32_t sizeofFolder = CLUSTER_SIZE;
    uint32_t ceil;
    uint8_t current;
    if(!is_empty_folder(folder)){
        for(int i=0; (i<ENTRY_IN_CLUSTER) && (!is_empty_entry(&folder->u.entry[i])); i++){
            if(folder->u.entry[i].attribute == DIR_TYPE){
                current = folder->fat;
                get_cluster(folder, folder->u.entry[i].frstCluster);
                sizeofFolder += physical_size(folder);
                get_cluster(folder, current);
            }
            else{
                ceil = (folder->u.entry[i].size + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
                sizeofFolder += (ceil == 0 ? CLUSTER_SIZE : ceil * CLUSTER_SIZE);
            }
        }
    }
    return sizeofFolder;    
}


/* 
 * Function dir: VOID -> VOID
 * dado um sistema de arquivos, imprimir lista com os arquivos e diretórios
 * que estão dentro do diretório atual.
 * dir();
 */
uint8_t swap_cluster_in_chain(uint8_t x, int* moved){
    uint8_t frstFree = frst_free_cluster();
    if(x == EOF_CLU)
        return EOF_CLU;
    else if(frstFree < x){
         swap_clusters(frstFree, x);
         (*moved)++;
         fat[frstFree] = swap_cluster_in_chain(point_to(frstFree), moved);
         update_fat();
         return frstFree;
    }
    else{
        fat[x] = swap_cluster_in_chain(point_to(x), moved);
        update_fat();
        return x;
    }
}
