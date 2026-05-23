#ifndef FILESYSTEM_H_INCLUDED
#define FILESYSTEM_H_INCLUDED

/* ABERTURA */
int  init_fs(const char* disk_name);
int  open_fs(const char* disk_name);
void close_fs();

/* INFORMAÇÕES DO FS */
const char* get_diskname();
char*       get_path();

/* COMANDOS */
void cd       (char* fullpath);
void dir      ();
void size     ();
void edit     (char* fullpath, char* data);
void cat      (char* fullpath);
void mkdir    (char* filename);
void mkfile   (char* filename);
void move     (char* orig, char* dest);
void fs_rename(char* fullpath, char* new_name);
void rm       (char* fullpath);
void rf       (char* fullpath);
void defrag   ();

#endif 