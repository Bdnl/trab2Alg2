#ifndef __DATABASE_H__
#define __DATABASE_H__

#define DEBUG

#define DBFILENAME "db.dat"
#define IDXFILENAME "idx.dat"
#define IDADEFILENAME "idade.dat"
#define IDADELISTFILENAME "idade_list.dat"
#define GENEROSFILENAME "generos.dat"
#define GENEROSLISTFILENAME "generos_list.dat"
#define GENEROSTABLEFILENAME "generos_table.dat"

// tamanho que um registro nao pode exceder
#define REGSIZE  100
#define NOMESIZE 50
#define GENSIZE  256

typedef uint     id_type;
typedef uint8_t  idade_t;
typedef char     sexo_t;
typedef uint8_t  genero_t; // é repesentado por um COD
typedef uint8_t  tu_t; // tipo de usuário
typedef long int offset_t;

typedef struct {
	id_type id;
	offset_t offset;
} idx_id_t;

typedef struct {
	int cod; // codigo do registro secundario
	int last_pos; // ultima posicao do id
} secundary_node_t;

typedef struct {
	id_type id;
	int next_pos;
} pos_list_t;

typedef struct {
	secundary_node_t *nodes;
	pos_list_t *pos_list;
	uint num_node; // quantidade de nós
	uint num_list; // tamanho da lista
} secundary_t;

typedef struct {
	char str[GENSIZE];
	genero_t cod;
} genero_node_t;

typedef struct {
	genero_node_t *nodes;
	uint num_node;
} genero_table_t;

typedef struct {
	FILE *file_db; // arquivo do banco de dados
	FILE *file_id; // arquivo de índices com ID
	FILE *file_idade; // arquivo de índices com idades
	FILE *file_generos; // arquivo de índices com generos
	FILE *file_generos_table;

	idx_id_t *idx_id; // lista com todos os id e seus offset
	uint num_id; // quantidade de ids
	secundary_t idx_idade;
	secundary_t idx_genero;

	genero_table_t genero_table;
} database_t;

typedef struct {
	id_type id;
	char nome[NOMESIZE];
	idade_t idade;
	sexo_t sexo;
	// generos é um vetor, o último elemento é sempre 0 indicando o final do vetor
	// portanto, ele age como uma string
	genero_t generos[GENSIZE];
	tu_t tu;
} registro_t;

#include "registro.h"

void initDB(database_t *db);
FILE *abrirArquivoDB(database_t *db, char *mode);
FILE *abrirArquivoIdx(database_t *db, char *mode);
FILE *abrirArquivoIdade(database_t *db, char *mode);
FILE *abrirArquivoGeneros(database_t *db, char *mode);
FILE *abrirArquivoGenerosTable(database_t *db, char *mode);
void fecharArquivoDB(database_t *db);
void fecharArquivoIdx(database_t *db);
void fecharArquivoIdade(database_t *db);
void fecharArquivoGeneros(database_t *db);
void fecharArquivoGenerosTable(database_t *db);
int getNodePos(secundary_t *secundary, int cod);
void zerarSecundaryNode(secundary_node_t *node);
void newSecundaryIdx(database_t *db, secundary_t *secundary, int cod, id_type id, char *file_principal, char *file_list);
void loadSecundaryIdx(database_t *db, secundary_t *secundary, char *file_principal, char *file_list);
void closeDB(database_t *db);
#endif