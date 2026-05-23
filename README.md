# Fat8-Filesystem

Sistema de arquivos simulado em disco, inspirado no FAT, implementado em C como trabalho da disciplina de **Arquitetura de Computadores 2**.

O disco é um arquivo binário dividido em três regiões: metadados, tabela FAT de 8 bits e área de clusters de dados. Toda a navegação e manipulação é feita via um terminal interativo em linha de comando.

---

## Estrutura do projeto

```
Fat8-Filesystem/
├── include/
│   ├── filesystem.h           # API pública
│   └── filesystem_internal.h  # structs, constantes e protótipos internos
├── src/
│   ├── disk_io.c    # I/O com o disco (fread/fwrite/fseek)
│   ├── cluster.c    # operações sobre clusters em memória
│   ├── directory.c  # entradas de diretório (inserir, buscar, remover)
│   ├── commands.c   # comandos expostos ao terminal
│   └── main.c       # loop CLI
└── makefile
```

---

## Layout do disco

```
┌──────────────┬───────────────────┬──────────────────────────────────┐
│  MetaData    │  FAT (256 bytes)  │  Clusters de dados (256 × 32 KB) │
│  (8 bytes)   │                   │                                  │
└──────────────┴───────────────────┴──────────────────────────────────┘
  offset 0       offset 8            offset 264
```

| Parâmetro        | Valor         |
|------------------|---------------|
| Entradas FAT     | 256 (8 bits)  |
| Tamanho do cluster | 32.768 bytes |
| Tamanho máximo do disco | ~8 MB  |
| Tamanho máximo de nome  | 18 chars |
| Profundidade máxima de path | 100 chars |

Valores especiais da FAT:

| Constante    | Valor  | Significado               |
|--------------|--------|---------------------------|
| `FREE_CLU`   | `0x00` | Cluster livre             |
| `EOF_CLU`    | `0xFF` | Fim de cadeia             |
| `FAIL_CLU`   | `0xFE` | Índice inválido           |
| `ROOT_CLUSTER`| `0x00` | Raiz do sistema de arquivos|

---

## Compilar e executar

```bash
make           # compila → binário ./fs
make clean     # remove o binário
```

**Inicializar um disco novo:**
```bash
./fs init meu_disco
```

**Abrir um disco existente:**
```bash
./fs boot meu_disco
```

---

## Comandos do terminal

| Comando | Argumentos | Descrição |
|---------|------------|-----------|
| `DIR` | — | Lista o conteúdo do diretório atual |
| `DISK` | — | Exibe tamanho lógico e físico do diretório atual |
| `CD` | `<path>` | Muda o diretório atual |
| `MKDIR` | `<nome>` | Cria um diretório no path atual |
| `MKFILE` | `<nome>` | Cria um arquivo no path atual |
| `EDIT` | `<path> "<dados>"` | Substitui o conteúdo de um arquivo |
| `MOVE` | `<origem> <destino>` | Move uma entrada para outro diretório |
| `RENAME` | `<path> <novo_nome>` | Renomeia uma entrada |
| `RM` | `<path>` | Remove um arquivo ou diretório vazio |
| `RF` | `<path>` | Remove recursivamente |
| `DEFRAG` | — | Desfragmenta o disco |
| `CLEAR` | — | Limpa o terminal |
| `EXIT` | — | Encerra a sessão |

Nomes de arquivos e diretórios aceitam apenas `[a-z0-9.]`. Paths usam `/root` como raiz (ex: `/root/pasta/arquivo.txt`).

---

## Módulos internos

### `disk_io.c`
Única camada que toca o arquivo em disco. Expõe `get_cluster` e `update_cluster` para carregar e persistir clusters, e `update_fat` para sincronizar a tabela FAT.

### `cluster.c`
Manipulação de clusters em memória: `frst_free_cluster` localiza espaço livre, `clear_chain` libera uma cadeia inteira, `swap_clusters` troca dois clusters de posição (usado pelo defrag), `logical_size` e `physical_size` calculam os tamanhos recursivamente.

### `directory.c`
Opera sobre as entradas (`Entry`) dentro de um cluster de diretório: `get_entry`, `insert_entry`, `delete_entry`, `walk_through_path` (navega o path até o cluster alvo) e `parse_path` (divide um fullpath em diretório pai + nome do arquivo).

### `commands.c`
Implementa os comandos expostos ao terminal. Cada função aloca seus clusters com `malloc`, executa a operação e libera ao sair — sem estado local persistente.

---

## Melhorias planejadas

- **Cluster de tamanho variável** — atualmente fixo em 32 KB; parametrizar no `init_fs` e salvar no `MetaData` para suportar discos maiores ou mais fragmentados
- Suporte a arquivos com mais de um cluster de diretório (diretórios com muitas entradas)
- `cat` — leitura do conteúdo de arquivos via terminal
- `append` — escrita incremental sem sobrescrever
- Tratar `malloc` retornando `NULL` em todos os caminhos
- Corrigir warnings de signed/unsigned nos loops sobre `ENTRY_IN_CLUSTER`
- `parse_path` atualmente modifica o argumento in-place; isolar em cópia local

---

## Limitações conhecidas

- FAT de 8 bits limita o disco a 253 clusters utilizáveis (0x01–0xFD)
- Cluster fixo em 32 KB — arquivos pequenos desperdiçam espaço
- Sem data/hora nas entradas
- Sem suporte a links simbólicos ou permissões
