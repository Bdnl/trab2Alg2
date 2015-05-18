#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "misc.h"
#include "registro.h"

// troca as funcoes cruciais
#define malloc _malloc
#define realloc _realloc

// retorna o tamnho de buffer
uint registroToBuffer(registro_t *reg, char *buffer) {
	return snprintf(buffer, REGSIZE, "%d|%s|%d|%c|%s|%d|",
		reg->id, reg->nome, reg->idade, reg->sexo, reg->generos, reg->tu);
}

// de buffer para registro_t
void bufferToReg(char *buffer, registro_t *reg) {
	reg->id = atoi(buffer);
	if(reg->id == 0) {
		// nesse ponto, o programa sabe que o registro foi removido
		// pq o primeiro char de id é *
		return ;
	}
	// procura o '|'
	while(*buffer != '|') buffer++;
	buffer++; // agora vale o primeiro char de nome
	// posicao da primeira letra do nome
	char *start = buffer;
	while(*buffer != '|') buffer++;
	// se eu quiser dar um malloc
	// reg->nome = malloc(buffer - start + 1);
	reg->nome[buffer - start] = '\0';
	memcpy(reg->nome, start, buffer-start);
	buffer++; // agora vale o primeiro char de idade
	reg->idade = atoi(buffer);
	while(*buffer != '|') buffer++;
	buffer++; // agora vale o primeiro char de sexo
	reg->sexo = *buffer;
	buffer+=2;
	start = buffer;
	while(*buffer != '|') buffer++;
	// caso eu queira um malloc
	// reg->generos = malloc(buffer - start + 1);
	reg->generos[buffer - start] = '\0';
	memcpy(reg->generos, start, buffer-start);
	buffer++;
	reg->tu = atoi(buffer);
}

// retorna EOF caso erro
offset_t novoRegistro(database_t *db, registro_t *reg) {
	// opcao 1
	#ifdef DEBUG
		printf("Inserindo um novo registro, Nome: '%s'\n", reg->nome);
	#endif // DEBUG
	// atualiza o arquivo principal
	if(pesquisarRegistro(db, reg->id) != EOF) {
		// isto indica que o registro já existe
		#ifdef DEBUG
			printf("Registro ID: '%d' já existe!\n", reg->id);
		#endif // DEBUG
		return EOF;
	}
	FILE *fd = abrirArquivoDB(db, "a");
	// offset indica o valor do primeiro caracter no arquivo
	offset_t offset = ftell(fd) + 1;
	char buffer[REGSIZE];
	uint8_t buffer_size;
	buffer_size = registroToBuffer(reg, buffer);
	fputc(buffer_size, fd);
	fwrite(buffer, sizeof(char), buffer_size, fd);
	fecharArquivoDB(db);
	// atualiza o registro secundario idade
	newSecundaryIdx(db, &db->idx_idade, reg->idade, reg->id, IDADEFILENAME, IDADELISTFILENAME);
	// atualiza o registro secundario de generos
	int i = 0;
	while(reg->generos[i]) {
		newSecundaryIdx(db, &db->idx_genero, reg->generos[i], reg->id, GENEROSFILENAME, GENEROSLISTFILENAME);
		i++;
	}
	// atualiza o indice primario
	db->num_id++;
	db->idx_id = realloc(db->idx_id, db->num_id * sizeof(idx_id_t));
	idx_id_t *idx_id = db->idx_id + db->num_id - 1;
	idx_id->id = reg->id;
	idx_id->offset = offset;
	// abre o arquivo de idx principal
	// insere no final
	fd = fopen(IDXFILENAME, "a");
	fwrite(idx_id, sizeof(idx_id_t), 1, fd);
	fclose(fd);
}

// retorna EOF quando acaba o arquivo
offset_t lerRegistro(database_t *db, registro_t *reg) {
	FILE *fd = abrirArquivoDB(db, "r");
	int regsize = fgetc(fd);
	if(regsize == EOF) {
		// não há mais registros no arquivos
		return EOF;
	}
	// retorna a posição do primeiro caractere do registro
	offset_t result = ftell(fd);
	char *buffer = malloc(regsize+1); // mais um para comportar o \0
	buffer[regsize] = 0;
	fread(buffer, sizeof(char), regsize, fd);
	bufferToReg(buffer, reg);
	free(buffer);
	fecharArquivoDB(db);
	return result;
}

// retorna a posicao do arquivo do primeiro byte do id
offset_t pesquisarRegistro(database_t *db, id_type id) {
	// opcao 3
	int i;
	for(i=0; i<db->num_id; i++) {
		if(db->idx_id[i].id == id) {
			return db->idx_id[i].offset;
		}
	}
	// nao encontrado
	return -1;
}

// retorna verdadeiro se foi um sucesso
bool removerRegistro(database_t *db, id_type id) {
	// opcao 2
	FILE *fd = abrirArquivoDB(db, "r+");
	offset_t pos = pesquisarRegistro(db, id);
	if(pos == EOF) {
		return false;
	}
	fseek(fd, pos, SEEK_SET);
	// insere o * indicando q o registro está apagado
	fputc('*', fd);
	fecharArquivoDB(db);
	return true;
}

// retorna verdadeiro se o registro curte o genero
bool regCurteGenero(registro_t *reg, genero_t genero) {
	int i = 0;
	// varre todos os generos q o usuario curte
	while(reg->generos[i]) {
		if(reg->generos[i] == genero) {
			return true;
		}
		i++;
	}
	return false;
}

/**
 * precisa de free
 * retorna os 10 gêneros mais populares para a faixa de idade
 * vetor do tipo [1, 2, 3, 4, 0, 0, 0, ...] ou tem 10 generos, ou acaba em 0
 */
genero_t *generosPopularesIdade(database_t *db, idade_t ini, idade_t fim) {
	// opcao 6
	genero_t *result = malloc(10 * sizeof(genero_t));
	// vetor com a quantidade de pessoas que escuta determinado genero
	int escutam[GENSIZE] = {0};
	registro_t reg;
	while(lerRegistro(db, &reg) != EOF) {
		if(reg.idade < ini || reg.idade > fim) {
			// pula os registros fora de idade
			continue;
		}
		int i = 0;
		// varre todos os generos da pessoa
		while(reg.generos[i]) {
			escutam[reg.generos[i]]++;
			i++;
		}
	}
	// define que os dez maiores sao os dez primeiros
	int i;
	for(i=0; i<10; i++) {
		result[i] = i+1;
	}
	// posicao do genero menos curtido em result
	int menos_curtido = 0;
	for(i=1; i<3; i++) {
		if(escutam[i] < escutam[menos_curtido]) {
			menos_curtido = i;
		}
	}
	// varre todos os registro até encontrar os maiores
	i = 11;
	while(i < GENSIZE) {
		// se ele é mais curtido do que o menor
		if(escutam[i] > escutam[menos_curtido]) {
			result[menos_curtido] = i;
		}
		menos_curtido = 0;
		// reencontra o menos curtido
		int j;
		for(j=1; j<10; j++) {
			if(escutam[j] < escutam[menos_curtido]) {
				menos_curtido = j;
			}
		}
		i++;
	}
	return result;
}

/**
 * precisa de free
 * lista de IDs das pessoas que gostam do genero e tem idade entre ini-fim
 * lista do tipo [id1, id2, 0]
 * o último elemento é sempre zero
 */
id_type *usariosPorGenero(database_t *db, genero_t genero, idade_t idade_ini, idade_t idade_fim) {
	// opcao 7
	id_type *result = NULL;
	abrirArquivoDB(db, "r");
	registro_t reg;
	int result_size = 0;
	while(lerRegistro(db, &reg) != EOF) {
		if(regCurteGenero(&reg, genero) && reg.idade >= idade_ini && reg.idade <= idade_fim) {
			result = _realloc(result, sizeof(id_type) * (result_size + 1));
			result[result_size] = reg.id;
			result_size++;
		}
	}
	result = _realloc(result, sizeof(id_type) * (result_size + 1));
	result[result_size] = 0;
	result_size++;
	fecharArquivoDB(db);
	return result;
}

/**
 * precisa de free
 * os 3 generos mais populares entres as pessoas que curtem os generos do parametro
 * generos deve ser algo como [1, 2, 3, 4, 0], o ultimo valor é sempre 0
 * o retorno é algo como [1, 2, 0], ler até o zero ou 3 elementos
 */
genero_t *generosPopularesGenero(database_t *db, genero_t *generos) {
	// opcao 4
	// mto parecida com a opcao 4
	genero_t *result = malloc(3 * sizeof(genero_t));
	if(generos[0] == 0) {
		// vetor de generos esta vazio
		return result;
	}
	// vetor com a quantidade de pessoas que escuta determinado genero
	int escutam[GENSIZE] = {0};
	registro_t reg;
	while(lerRegistro(db, &reg) != EOF) {
		// variavel que diz se o while debaixo deu break
		bool breaked = false;
		int i = 0;
		while(generos[i]) {
			if(!regCurteGenero(&reg, generos[i])) {
				// essa pessoa nao curte um dos generos requeridos
				breaked = true;
				break;
			}
			i++;
		}
		// passa pro proximo registro se o laço acima foi interrompido
		if(breaked) {
			continue;
		}
		i = 0;
		// varre todos os generos da pessoa
		while(reg.generos[i]) {
			escutam[reg.generos[i]]++;
			i++;
		}
	}
	// define que os 3 maiores sao os 3 primeiros
	int i;
	for(i=0; i<3; i++) {
		result[i] = i+1;
	}
	// posicao do genero menos curtido em result
	int menos_curtido = 0;
	for(i=1; i<3; i++) {
		if(escutam[i] < escutam[menos_curtido]) {
			menos_curtido = i;
		}
	}
	// varre todos os registro até encontrar os maiores
	i = 4;
	while(i < GENSIZE) {
		// se ele é mais curtido do que o menor
		if(escutam[i] > escutam[menos_curtido]) {
			result[menos_curtido] = i;
		}
		menos_curtido = 0;
		// reencontra o menos curtido
		int j;
		for(j=1; j<3; j++) {
			if(escutam[j] < escutam[menos_curtido]) {
				menos_curtido = j;
			}
		}
		i++;
	}
	return result;
}

/**
 * precisa de free
 * os 10 usuarios mais jovem de TU que curtem generos
 * se há somente 5 usuários o retorno é 1, 2, 3, 4, 5, 0, 0, 0, ...
 * generos deve ser algo como [1, 2, 3, 4, 0], o ultimo valor é sempre 0
 * ler até o 0 ou 10 elementos
 */
id_type *usuariosMaisJovems(database_t *db, genero_t *generos, tu_t tu) {
	// opcao 5
	idade_t idades[10];
	id_type *result = malloc(10 * sizeof(id_type));
	// preenche tudo com 0
	memset(result, 0, 10);
	registro_t reg;
	FILE *fd = abrirArquivoDB(db, "r");
	if(_file_size(fd) == 0) {
		// o arquivo esta vazio
		return result;
	}
	int i = 0;
	// preenche as 10 primeiras idades
	while(lerRegistro(db, &reg) != EOF && i < 10) {
		result[i] = reg.id;
		idades[i] = reg.idade;
		i++;
	}
	while(lerRegistro(db, &reg) != EOF) {
		for(i=0; i<10; i++) {
			if(reg.idade < idades[i]) {
				idades[i] = reg.idade;
				result[i] = reg.id;
				break;
			}
		}
	}
	fecharArquivoDB(db);
	return result;
}

genero_t generoStrToCod(database_t *db, char *str) {
	#ifdef DEBUG
		printf("Testando genero '%s'\n", str);
	#endif // DEBUG
	genero_table_t *genero_table = &db->genero_table;
	int i;
	for(i=0; i<genero_table->num_node; i++) {
		if(strcmp(genero_table->nodes[i].str, str) == 0) {
			// encontrado
			return genero_table->nodes[i].cod;
		}
	}
	// novo cod
	int cod = i+1;
	// se nao for encontrado, cria um novo
	genero_table->num_node++;
	genero_table->nodes = realloc(genero_table->nodes, genero_table->num_node * sizeof(genero_node_t));
	genero_node_t *node = genero_table->nodes + genero_table->num_node - 1;
	strcpy(node->str, str);
	// o novo valor passa a ser i
	node->cod = cod;
	// atualiza o arquivo de generos
	FILE *fd = fopen(GENEROSTABLEFILENAME, "a");
	// escrita binária
	// fwrite(node, sizeof(genero_node_t), 1, fd);
	// escrita para usuario
	fprintf(fd, "%s=%d\n", node->str, node->cod);
	fclose(fd);
	return cod;
}

/**
 * Altera o valor de str
 * converte a string: 'Pagode@Rock@Metal' para seus devidos codigos
 */
void generosStrToCod(database_t *db, char *str) {
	// conta a quantidade de arrobas
	#ifdef DEBUG
		printf("Convertendo a string para cod '%s'", str);
	#endif // DEBUG
	int i = 0;
	int num_generos = 0;
	while(str[i]) {
		if(str[i] == '@') {
			num_generos++;
			// ja define o @ como \0 para começar uma nova procura
			str[i] = '\0';
		}
		i++;
	}
	num_generos++;
	#ifdef DEBUG
		printf(", com %d generos\n", num_generos);
	#endif // DEBUG
	// ja sei qntos elementos tem
	// string com o proximo genero
	char *genero = str;
	// começa a varrer todos os generos
	for(i=0; i<num_generos; i++) {
		str[i] = generoStrToCod(db, genero);
		// avança para o proximo genero
		// encontra o \0
		while(*genero) genero++;
		// avança mais um
		genero++;
	}
	str[num_generos] = '\0';
	#ifdef DEBUG
		printfVerticaly(str);
	#endif // DEBUG
}