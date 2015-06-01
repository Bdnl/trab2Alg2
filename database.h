#ifndef __DATABASE_H__
#define __DATABASE_H__

// #define TEST // defina isto para fazer casos de teste

#define DBFILENAME "db.dat"
#define IDXFILENAME "idx.dat"
#define IDADEFILENAME "idade.dat"
#define GENEROSFILENAME "generos.dat"
#define GENEROSTABLEFILENAME "generos_table.dat"
#define TUFILENAME "tu.dat"

// tamanho que um registro nao pode exceder
#define REGSIZE  100
#define NOMESIZE 50
#define GENSIZE  100

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
	id_type id;
} secondary_node_t;

typedef struct {
	secondary_node_t *nodes;
	uint num_node; // quantidade de nós
} secondary_t;

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
	int ordenado; // diz se os arquivos de indice estão ordenados, é int pq usa fgetc e pode retornar -1
	secondary_t idx_idade;
	secondary_t idx_genero;
	secondary_t idx_tu;

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

/**
 * Inicializa database com os valores padrões
 * @param db necessariamente não inicializada
 */
void initDB(database_t *db);

/* ====================================================
   FUNÇÕES PARA ABRIR ARQUIVOS
   ==================================================== */

FILE *abrirArquivoDB(database_t *db, char *mode);

FILE *abrirArquivoIdx(database_t *db, char *mode);

FILE *abrirArquivoIdade(database_t *db, char *mode);

FILE *abrirArquivoGeneros(database_t *db, char *mode);

/* ====================================================
   FUNÇÕES PARA FECHAR ARQUIVO
   ==================================================== */

void fecharArquivoDB(database_t *db);

void fecharArquivoIdx(database_t *db);

void fecharArquivoIdade(database_t *db);

void fecharArquivoGeneros(database_t *db);

/**
 * inicializa um nó de índices secundarios
 * @param node necessariamente não inicializada
 */
void initSecondaryNode(secondary_node_t *node);

/**
 * cria um novo índice secundário
 * @param db        previamente inicializada
 * @param secondary previamente inicializada
 * @param cod       cod do novo índice
 * @param id        id que apontará o índice
 * @param file_name nome do arquivo principal de índice secundário
 */
void newSecondaryIdx(database_t *db, secondary_t *secondary, int cod, id_type id, char *file_name);

/**
 * carrega para a memória RAM um índice secundário
 * @param db        previamente inicializada
 * @param secondary não necessariamente inicializada, será alterada
 * @param file_name nome do arquivo principal de índice secundário
 */
void loadSecondaryIdx(database_t *db, secondary_t *secondary, char *file_name);

/**
 * libera um índice secundário
 * @param secondary previamente inicializado
 */
void freeSecondaryIdx(secondary_t *secondary);

/**
 * libera e encerra o db
 * @param db previamente inicializado
 */
void closeDB(database_t *db);

/* ====================================================
   FUNCOES DE ORDENACAO E PESQUISA
   ==================================================== */
int qsort_secondary(const void *p1, const void *p2);

/**
 * Busca binária em índice secundário
 * @param  id        id do índice procurado
 * @return           retorna a posição do primeiro índice na memória
 */
int bsearchSecondary(secondary_t *secondary, id_type id);

int qsort_idx(const void *p1, const void *p2);

/**
 * funcao que retorna se tem registro ou não dentro do programa
 * @param  db previamente inicializado
 * @return    verdadeiro se tem registros dentro do arquivo
 */
bool temRegistro(database_t *db);

void ordenarSecundario(database_t *db, secondary_t *secondary, FILE *fd);

void loadRegFromMemory(database_t *db, id_type id, registro_t *reg);

void removerSecondary(database_t *db, secondary_t *secondary, id_type id, char *file_name);

void ordenarDB(database_t *db);

/**
 * set na flag do arquivo idx, essa flag diz se os indices estao atualizados ou não
 * @param db   inicializado previamente
 * @param flag nova flag
 */
void setOrdenado(database_t *db, char flag);
#endif