/**
 * Leonardo Guarnieri de Bastiani  8910434
 * Guilherme José Acra             7150306
 * Ricardo Chagas                  8957242
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "misc.h"
#include "database.h"

// funções do misc
#define fopen _fopen
#define fclose _fclose

/**
 * Inicializa database com os valores padrões
 * @param db necessariamente não inicializada
 */
void initDB(database_t *db) {
	// zera completamente db
	memset(db, 0, sizeof(database_t));
	if(file_exists(DBFILENAME) && !file_exists(IDXFILENAME)) {
		// se o arquivo de dados existe e os arquivos de índice não
		// cria os arquvos de índice
		criarIndiceComFileDB(db);
		return ;
	}
	// abre o arquivo para garantir que ele vai existir
	fclose(fopen(DBFILENAME, "a"));
	fclose(fopen(IDXFILENAME, "a"));
	fclose(fopen(IDADEFILENAME, "a"));
	fclose(fopen(GENEROSFILENAME, "a"));
	fclose(fopen(TUFILENAME, "a"));
	fclose(fopen(GENEROSTABLEFILENAME, "a"));
	// carrega todos os idx
	// carrega idx principal
	FILE *fd = fopen(IDXFILENAME, "r+");
	// menos um porque o primeiro byte é o flag q diz se tá ordenado ou não
	db->num_id = (_file_size(fd) - 1) / sizeof(idx_id_t);
	db->ordenado = fgetc(fd);
	if(db->ordenado == -1) {
		// não foi inserido nenhum arquivo por enquanto, não há necessidade de carregar conteúdos em RAM
		// mas deve inserir a flag no arquivo
		rewind(fd);
		fputc(1, fd);
		db->ordenado = 1;
		db->num_id = 0;
		fclose(fd);
		return;
	}
	db->idx_id = malloc(db->num_id * sizeof(idx_id_t));
	fread(db->idx_id, sizeof(idx_id_t), db->num_id, fd);
	fclose(fd);
	// remove os ids = 0
	while(db->num_id > 0) {
		if(db->idx_id[db->num_id-1].id != 0) {
			// ja removeu todo mundo
			break;
		}
		db->num_id--;
	}
	// carrega os indices secundarios
	loadSecondaryIdx(db, &db->idx_idade, IDADEFILENAME);
	loadSecondaryIdx(db, &db->idx_genero, GENEROSFILENAME);
	loadSecondaryIdx(db, &db->idx_tu, TUFILENAME);
	// carrega a table de generos
	fd = fopen(GENEROSTABLEFILENAME, "r");
	db->genero_table.num_node = _file_size(fd) / sizeof(genero_node_t);
	db->genero_table.nodes = NULL;
	// leitura binária
	// fread(db->genero_table.nodes, sizeof(genero_node_t), db->genero_table.num_node, fd);
	// leitura IU
	char buffer[256];
	#ifdef DEBUG
		section("CARREGANDO GENEROS");
	#endif // DEBUG
	while(_fgets(buffer, 256, fd)) {
		// i vai de 0 até o número de generos-1
		int i = db->genero_table.num_node++;
		db->genero_table.nodes = realloc(db->genero_table.nodes, db->genero_table.num_node * sizeof(genero_node_t));
		char *igual = buffer; // procura o igual na frase
		while(*igual != '=') igual++;	
		*igual = 0;
		// copia o nome do genero para seu devido lugar
		strcpy(db->genero_table.nodes[i].str, buffer);
		igual++;
		db->genero_table.nodes[i].cod = atoi(igual);
		#ifdef DEBUG
			printf("Novo genero carregado: '%s' => %d\n", db->genero_table.nodes[i].str, db->genero_table.nodes[i].cod);
		#endif // DEBUG
	}
	fclose(fd);
	#ifdef DEBUG
		section("DATABASE CARREGADO!");
	#endif // DEBUG
}

/* ====================================================
   FUNÇÕES PARA ABRIR ARQUIVOS
   ==================================================== */

FILE *abrirArquivoDB(database_t *db, char *mode) {
	if(db->file_db != NULL) {
		return db->file_db;
	}
	db->file_db = fopen(DBFILENAME, mode);
	return db->file_db;
}

FILE *abrirArquivoIdx(database_t *db, char *mode) {
	if(db->file_id != NULL) {
		return db->file_id;
	}
	db->file_id = fopen(IDXFILENAME, mode);
	return db->file_id;
}

FILE *abrirArquivoIdade(database_t *db, char *mode) {
	if(db->file_idade != NULL) {
		return db->file_idade;
	}
	db->file_idade = fopen(IDADEFILENAME, mode);
	return db->file_idade;
}

FILE *abrirArquivoGeneros(database_t *db, char *mode) {
	if(db->file_generos != NULL) {
		return db->file_generos;
	}
	db->file_generos = fopen(GENEROSFILENAME, mode);
	return db->file_generos;
}

/* ====================================================
   FUNÇÕES PARA FECHAR ARQUIVO
   ==================================================== */

void fecharArquivoDB(database_t *db) {
	fclose(db->file_db);
	db->file_db = NULL;
}

void fecharArquivoIdx(database_t *db) {
	fclose(db->file_id);
	db->file_id = NULL;
}

void fecharArquivoIdade(database_t *db) {
	fclose(db->file_idade);
	db->file_idade = NULL;
}

void fecharArquivoGeneros(database_t *db) {
	fclose(db->file_generos);
	db->file_generos = NULL;
}

/**
 * inicializa um nó de índices secundarios
 * @param node necessariamente não inicializada
 */
void initSecondaryNode(secondary_node_t *node) {
	memset(node, 0, sizeof(secondary_node_t));
}

/**
 * cria um novo índice secundário
 * @param db        previamente inicializada
 * @param secondary previamente inicializada
 * @param cod       cod do novo índice
 * @param id        id que apontará o índice
 * @param file_name nome do arquivo principal de índice secundário
 */
void newSecondaryIdx(database_t *db, secondary_t *secondary, int cod, id_type id, char *file_name) {
	#ifdef DEBUG
		printf("Inserindo no secundario: %d => %d\n", cod, id);
	#endif // DEBUG
	// obtem a nova posicao do nó
	int pos_node = secondary->num_node;
	// cria um novo nó
	secondary->num_node++;
	secondary->nodes = realloc(secondary->nodes, secondary->num_node * sizeof(secondary_node_t));
	initSecondaryNode(secondary->nodes + pos_node);
	// seta as nocas condições
	secondary->nodes[pos_node].cod = cod;
	secondary->nodes[pos_node].id = id;
	// atualiza todo o arquivo de idx
	FILE *fd = fopen(file_name, "a");
	fwrite(secondary->nodes+pos_node, sizeof(secondary_node_t), 1, fd);
	fclose(fd);
}

/**
 * carrega para a memória RAM um índice secundário
 * @param db        previamente inicializada
 * @param secondary não necessariamente inicializada, será alterada
 * @param file_name nome do arquivo principal de índice secundário
 */
void loadSecondaryIdx(database_t *db, secondary_t *secondary, char *file_name) {
	// carregando o arquivo principal
	FILE *fd_principal = fopen(file_name, "r");

	secondary->num_node = _file_size(fd_principal) / sizeof(secondary_node_t);
	// aloca
	secondary->nodes = malloc(secondary->num_node * sizeof(secondary_node_t));
	// carrega o principal
	fread(secondary->nodes, sizeof(secondary_node_t), secondary->num_node, fd_principal);

	fclose(fd_principal);
	// remove os ids = 0
	while(secondary->num_node > 0) {
		if(secondary->nodes[secondary->num_node-1].id != 0) {
			// ja removeu todo mundo
			break;
		}
		secondary->num_node--;
	}
}

/**
 * libera um índice secundário
 * @param secondary previamente inicializado
 */
void freeSecondaryIdx(secondary_t *secondary) {
	free(secondary->nodes);
}

/**
 * libera e encerra o db
 * @param db previamente inicializado
 */
void closeDB(database_t *db) {
	// fecha todos os arquivos
	fecharArquivoDB(db);
	fecharArquivoIdx(db);
	fecharArquivoIdade(db);
	fecharArquivoGeneros(db);

	// libera na memória
	freeSecondaryIdx(&db->idx_idade);
	freeSecondaryIdx(&db->idx_genero);
	freeSecondaryIdx(&db->idx_tu);
	free(db->idx_id);
}

/* ====================================================
   FUNCOES DE ORDENACAO E PESQUISA
   ==================================================== */
int qsort_secondary(const void *p1, const void *p2) { //Essa está sem comentário!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1<<<<<<<<<<AQUI>>>>>>>>>>>!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	secondary_node_t *lt = (secondary_node_t *) p1;
	secondary_node_t *gt = (secondary_node_t *) p2;
	// joga os zeros para a direita
	if(lt->id == 0) {
		return 1;
	} else if(gt->id == 0) {
		return -1;
	}
	// ordenação ascendente
	if(lt->id < gt->id) {
		return -1;
	} else if(lt->id > gt->id) {
		return 1;
	}
	return 0;
}

/**
 * Busca binária em índice secundário
 * @param  id        id do índice procurado
 * @return           retorna a posição do primeiro índice na memória
 */
int bsearchSecondary(secondary_t *secondary, id_type id) {
	// pesquisa binária
	// GARANTA QUE DB ESTÁ ORDENADO
	int beg = 0;
	int end = secondary->num_node-1;
	while(beg <= end) {
		int mid = (beg + end) / 2;
		if(secondary->nodes[mid].id == id) {
			// encontrou
			// anda com o mid para trás para char o primeiro elemento
			while(mid > 0) {
				if(secondary->nodes[mid-1].id == id) {
					mid--;
				} else {
					// se o ID de trás for de outra pessoa, sai do laço
					break;
				}
			}
			return mid;
		} else if(id < secondary->nodes[mid].id) {
			// está para a esquerda
			end = mid-1;
		} else if(id > secondary->nodes[mid].id) {
			// está para a direita
			beg = mid+1;
		}
	}
	// caso se não encontrar
	return -1;
}

int qsort_idx(const void *p1, const void *p2) { //Essa está sem comentário!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1<<<<<<<<<<AQUI>>>>>>>>>>>!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	idx_id_t *lt = (idx_id_t *) p1;
	idx_id_t *gt = (idx_id_t *) p2;
	// joga os zeros para a direita
	if(lt->id == 0) {
		return 1;
	} else if(gt->id == 0) {
		return -1;
	}
	// ordenação ascendente
	if(lt->id < gt->id) {
		return -1;
	} else if(lt->id > gt->id) {
		return 1;
	}
	return 0;
}

/**
 * funcao que retorna se tem registro ou não dentro do programa
 * @param  db previamente inicializado
 * @return    verdadeiro se tem registros dentro do arquivo
 */
bool temRegistro(database_t *db) {
	return db->num_id > 0;
}

void ordenarSecundario(database_t *db, secondary_t *secondary, FILE *fd) { //Essa está sem comentário!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1<<<<<<<<<<AQUI>>>>>>>>>>>!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	qsort(secondary->nodes, secondary->num_node, sizeof(secondary_node_t), qsort_secondary);
	// remove os ids = 0
	while(secondary->num_node > 0) {
		if(secondary->nodes[secondary->num_node-1].id != 0) {
			// ja removeu todos os zeros
			break;
		}
		secondary->num_node--;
	}
	fwrite(secondary->nodes, sizeof(secondary_node_t), secondary->num_node, fd);
}

void loadRegFromMemory(database_t *db, id_type id, registro_t *reg) { //Essa está sem comentário!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1<<<<<<<<<<AQUI>>>>>>>>>>>!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	setOrdenado(db, 1);
	reg->id = id;
	// carrega a idade
	int sec_pos = bsearchSecondary(&db->idx_idade, id);
	reg->idade = db->idx_idade.nodes[sec_pos].cod;
	// carrega os generos
	sec_pos = bsearchSecondary(&db->idx_genero, id);
	if(sec_pos == -1) {
		// vetor generos está vazio
		reg->generos[0] = 0;
	} else {
		int i = 0;
		while(sec_pos < db->idx_genero.num_node) {
			if(db->idx_genero.nodes[sec_pos].id != id) {
				break;
			}
			reg->generos[i] = db->idx_genero.nodes[sec_pos].cod;
			i++;
			sec_pos++;
		}
		reg->generos[i] = 0;
	}
	// carrega tu
	sec_pos = bsearchSecondary(&db->idx_tu, id);
	reg->tu = db->idx_tu.nodes[sec_pos].cod;
}

void removerSecondary(database_t *db, secondary_t *secondary, id_type id, char *file_name) { //Essa está sem comentário!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1<<<<<<<<<<AQUI>>>>>>>>>>>!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	FILE *fd = fopen(file_name, "r+");
	if(db->ordenado) {
		// faz busca binária
		int pos = bsearchSecondary(secondary, id);
		while(pos < secondary->num_node) {
			if(secondary->nodes[pos].id != id) {
				// acaba a varredura
				break;
			}
			// posiciona no arquivo para remoção
			offset_t offset = pos * sizeof(secondary_node_t);
			fseek(fd, offset, SEEK_SET);
			secondary->nodes[pos].id = 0;
			fwrite(secondary->nodes, sizeof(secondary_node_t), 1, fd);
		}
	} else {
		// faz busca sequencial
		int i;
		for(i=0; i<secondary->num_node; i++) {
			if(secondary->nodes[i].id != id) {
				continue;
			}
			// trabalha com este id
			// posiciona no arquivo para remoção
			offset_t offset = i * sizeof(secondary_node_t);
			fseek(fd, offset, SEEK_SET);
			secondary->nodes[i].id = 0;
			fwrite(secondary->nodes, sizeof(secondary_node_t), 1, fd);
		}
	}
	fclose(fd);
}

void ordenarDB(database_t *db) { //Essa está sem comentário!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1<<<<<<<<<<AQUI>>>>>>>>>>>!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if(db->ordenado) {
		// já está ordenado
		return ;
	}
	if(!temRegistro(db)) {
		// nao há arquivos para serem ordenados
		return ;
	}
	// ordena idx_id_t
	qsort(db->idx_id, db->num_id, sizeof(idx_id_t), qsort_idx);
	// remove os zeros no final
	while(db->num_id > 0) {
		if(db->idx_id[db->num_id-1].id != 0) {
			// ja removeu todos os zeros
			break;
		}
		db->num_id--;
	}

	// atualiza os arquivos
	FILE *fd = abrirArquivoIdx(db, "w");
	fputc(1, fd); // flag de ordenado
	if(db->num_id != 0) {
		fwrite(db->idx_id, sizeof(idx_id_t), db->num_id, fd);
	}
	fecharArquivoIdx(db);
	#ifdef DEBUG
		section("IMPRIMINDO INDICES");
		printf("Indice primário, num_id: %d\n", db->num_id);
		int i;
		for(i=0; i<db->num_id; i++) {
			printf("%2d => %2ld\n", db->idx_id[i].id, db->idx_id[i].offset);
		}
	#endif // DEBUG
	fd = abrirArquivoIdade(db, "w");
	ordenarSecundario(db, &db->idx_idade, fd);
	fecharArquivoIdade(db);
	fd = abrirArquivoGeneros(db, "w");
	ordenarSecundario(db, &db->idx_genero, fd);
	fecharArquivoGeneros(db);
}

/**
 * set na flag do arquivo idx, essa flag diz se os indices estao atualizados ou não
 * @param db   inicializado previamente
 * @param flag nova flag
 */
void setOrdenado(database_t *db, char flag) {
	if(db->ordenado == flag) {
		// já é igual, não precisa fazer nada
		return ;
	}
	if(flag == 1) {
		// se for para ordenar, que ordene!!
		ordenarDB(db);
	} else {
		// seta a nova flag para zero
		db->ordenado = flag;
		FILE *fd = abrirArquivoIdx(db, "r+");
		// insere no arquivo idx o status da flag
		fputc(flag, fd);
		fecharArquivoIdx(db);
	}
	#ifdef DEBUG
		printf("Modificando a FLAG para '%d'\n", flag);
	#endif // DEBUG
}

void novoIndice(database_t *db, registro_t *reg, offset_t offset) { //Essa está sem comentário!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1<<<<<<<<<<AQUI>>>>>>>>>>>!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// atualiza o registro secundario idade
	newSecondaryIdx(db, &db->idx_idade, reg->idade, reg->id, IDADEFILENAME);
	// atualiza o registro secundario de generos
	int i = 0;
	while(reg->generos[i]) {
		newSecondaryIdx(db, &db->idx_genero, reg->generos[i], reg->id, GENEROSFILENAME);
		i++;
	}
	// atualiza o registro secundario Tu
	newSecondaryIdx(db, &db->idx_tu, reg->tu, reg->id, TUFILENAME);
	// atualiza o indice primario
	db->num_id++;
	db->idx_id = realloc(db->idx_id, db->num_id * sizeof(idx_id_t));
	idx_id_t *idx_id = db->idx_id + db->num_id - 1;
	idx_id->id = reg->id;
	idx_id->offset = offset;
	#ifdef DEBUG
		printf("Novo índice para o ID: %d => %d\n", idx_id->id, idx_id->offset);
	#endif // DEBUG
	// abre o arquivo de idx principal
	// insere no final
	FILE *fd = abrirArquivoIdx(db, "a");
	fwrite(idx_id, sizeof(idx_id_t), 1, fd);
	fecharArquivoIdx(db);
}

void criarIndiceComFileDB(database_t *db) { //Essa está sem comentário!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1<<<<<<<<<<AQUI>>>>>>>>>>>!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	#ifdef DEBUG
		printf("Recriando o índice\n");
	#endif // DEBUG
	// insere a flag no arquivo
	FILE *fd = abrirArquivoIdx(db, "w");
	// insere no arquivo idx o status da flag
	fputc(0, fd);
	fecharArquivoIdx(db);
	// insere todos os índices nos arquivos
	// lê sequencialmente o arquivo principal
	abrirArquivoDB(db, "r");
	offset_t offset;
	registro_t reg;
	while((offset = lerRegistro(db, &reg)) != EOF) {
		if(reg.id == 0) {
			continue;
		}
		novoIndice(db, &reg, offset);
	}
	fecharArquivoDB(db);
	// fecha a conexão
	closeDB(db);
	// reabre
	initDB(db);
}