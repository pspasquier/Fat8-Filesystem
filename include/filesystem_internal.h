#ifndef FILESYSTEM_INTERNAL_H_INCLUDED
#define FILESYSTEM_INTERNAL_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* ====================== TERMINAL ====================== */

#define RESET   "\033[0m"
#define GREEN   "\033[0;42m"

/* ====================== TAMANHOS ====================== */

#define FAT_SIZE        256
#define CLUSTER_SIZE    32768
#define PATH_LIMIT      100
#define NAME_LIMIT      18

/* ====================== VALORES ESPECIAIS DA FAT ====================== */
/*
 * Ponteiros normais ocupam o intervalo 0x01–0xFD.
 * 0xFE e 0xFF são reservados para FAIL e EOF respectivamente.
 */
#define FREE_CLU        0x00   /* cluster livre                  */
#define FAIL_CLU        0xFE   /* índice inválido / sem espaço   */
#define EOF_CLU         0xFF   /* fim de cadeia                  */
#define ROOT_CLUSTER    0x00   /* índice do diretório raiz       */

/* ====================== TIPOS DE ENTRADA ====================== */

#define DIR_TYPE    0
#define FILE_TYPE   1

/* Número máximo de entradas que cabem em um cluster de diretório */
#define ENTRY_IN_CLUSTER (CLUSTER_SIZE / sizeof(Entry))

/* ====================== STRUCTS ====================== */

typedef struct MetaData MetaData;
typedef struct Entry    Entry;
typedef struct Cluster  Cluster;

/*
 * MetaData — cabeçalho do disco, gravado no offset 0.
 * Descreve o layout das regiões FAT e de dados.
 */
struct MetaData {
    uint16_t fatSize;     /* número de entradas da FAT (= FAT_SIZE)           */
    uint16_t clusterSize; /* tamanho em bytes de cada cluster (= CLUSTER_SIZE)*/
    uint16_t fatRegion;   /* offset em bytes onde a FAT começa                */
    uint16_t dataRegion;  /* offset em bytes onde os clusters de dados começam*/
} __attribute__((packed));

/*
 * Entry — entrada de diretório de 32 bytes.
 * Representa um arquivo ou subdiretório.
 */
struct Entry {
    char     name[NAME_LIMIT + 1]; /* nome com terminador nulo (19 bytes)       */
    uint8_t  attribute;            /* DIR_TYPE (0) ou FILE_TYPE (1)             */
    uint8_t  frstCluster;          /* índice FAT do primeiro cluster do conteúdo*/
    uint32_t size;                 /* tamanho lógico em bytes (arquivos)        */
    uint8_t  reserved[7];          /* padding para alinhar em 32 bytes          */
} __attribute__((packed));

/*
 * Cluster — unidade de armazenamento carregada em memória.
 *
 * Campos de controle:
 *   fat  — índice FAT deste cluster (sua própria posição no disco)
 *   next — valor que a FAT aponta para o próximo cluster da cadeia
 *           (EOF_CLU se for o último, FREE_CLU se liberado)
 *
 * Union u:
 *   entry[] — interpretação como diretório (array de Entry)
 *   data [] — interpretação como arquivo   (bytes brutos)
 *
 */
struct Cluster {
    uint8_t fat;
    uint8_t next;
    union {
        Entry   entry[CLUSTER_SIZE / sizeof(Entry)];
        uint8_t data [CLUSTER_SIZE];
    } u;
} __attribute__((packed));

/* ====================== ESTADO GLOBAL (definido em disk_io.c) ====================== */

extern FILE*       disk;                 /* arquivo de disco aberto */
extern MetaData    info;                 /* metadados lidos no open_fs */
extern uint8_t     fat [FAT_SIZE];       /* cópia em memória da FAT */
extern char        path[PATH_LIMIT + 1]; /* diretório atual (ex: "/root/foo") */
extern const char* diskname;             /* nome do arquivo de disco */

/* ====================== PROTÓTIPOS INTERNOS ====================== */

/* -- disk_io.c -- */
void    update_fat     (void);
uint8_t point_to       (uint8_t index);
int     get_cluster    (Cluster* a, uint8_t index);
int     update_cluster (Cluster* a);

/* -- cluster.c -- */
uint8_t  frst_free_cluster    (void);
void     clear_chain          (Cluster* a);
void     swap_clusters        (uint8_t dest, uint8_t orig);
bool     is_valid_cluster     (Cluster* a);
uint32_t logical_size         (Cluster* folder);
uint32_t physical_size        (Cluster* folder);
uint8_t  swap_cluster_in_chain(uint8_t x, int* moved);

/* -- directory.c -- */
bool   is_empty_folder  (Cluster* folder);
bool   is_empty_entry   (Entry* file);
bool   has_entry        (Cluster* folder, char* filename);
Entry* get_entry        (Cluster* folder, char* filename);
int    insert_entry     (Cluster* folder, Entry newEntry);
int    delete_entry     (Cluster* folder, char* filename);
void   walk_through_path(Cluster* folder, char* currpath);
int    parse_path       (char* fullpath, char* currpath, char* filename);
bool   is_valid_name    (char* filename);

#endif /* FILESYSTEM_INTERNAL_H_INCLUDED */
