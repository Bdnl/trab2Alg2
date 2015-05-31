#ifndef __REGISTRO_H__
#define __REGISTRO_H__

#include "database.h"

/**
 * converte de registro_t para uma string
 * @param  reg    previamente inicializado
 * @param  buffer string que será alterada
 * @return        strlen(buffer)
 */
uint registroToBuffer(registro_t *reg, char *buffer);

/**
 * de buffer para registro_t
 * @param buffer string com o buffer do tipo ID|Nome|...
 * @param reg    nao necessariamente inicializado
 */
void bufferToReg(char *buffer, registro_t *reg);

/**
 * criar um novo registro
 * @param  db  previamente inicializada
 * @param  reg conteúdo do registro novo
 * @return     retorna a posição do primeiro byte do registro ou EOF caso erro
 */
offset_t novoRegistro(database_t *db, registro_t *reg);

/**
 * lê um registro do arquivo
 * o cursor do arquivo deve estar apontado para a posição anterior do primeiro byte do registro
 * @param  db  previamente inicializada
 * @param  reg nao necessariamente inicializado, será alterada
 * @return     a pos~ição do primeiro byte do registro ou EOF caso erro
 */
offset_t lerRegistro(database_t *db, registro_t *reg);

/**
 * pesquisa o registro com base nos índices carregados em memória sequencialmente
 * @param  db previamente inicializada
 * @param  id valor do id procurado
 * @return    retorna a posicao do arquivo do primeiro byte do id
 */
offset_t pesquisarRegistro(database_t *db, id_type id);

/**
 * remove o registro tanto em memória, quanto no arquivo principal e nos de índex
 * @param  db previamente inicializada
 * @param  id do registro que será removido
 * @return    true caso seja verdadeiro
 */
bool removerRegistro(database_t *db, id_type id);

/**
 * pergunta se o registro reg curte o genero
 * @param  reg    previamente inicializada
 * @param  genero genero a ser pesquisado
 * @return        retorna verdadeiro se o registro curte o genero
 */
bool regCurteGenero(registro_t *reg, genero_t genero);

bool regCurteGeneros(registro_t *reg, genero_t *generos);

/*
Função que monta o conjunto com as pessoas que estão numa determinada faixa etária
Variáveis:
	db- banco de dados em memória
	ini- idade inicial
	fim- idade final
	conj_pessoas- conjunto de pessoas a ser montado
	i- contador
	parametro- usado para ver se foi encontrada pelo menos uma pessoa que atende aos requisitos

Retorno:
	Ponteiro para um conjunto de pessoas (seus idx) - Se encontrada pelo menos uma pessoa que atende aos requisitos
	NULL- Não foi encontrado ninguém 
*/
id_type* monta_conjuntoPopIdad(database_t *db, idade_t ini, idade_t fim);

/**
 * precisa de free
 * pesquisa os generos mais populares por idade
 * @param  db  previamente inicializada
 * @param  ini idade mínima
 * @param  fim idade máxima
 * @return     os 10 gêneros mais populares para a faixa de idade
 * vetor do tipo [1, 2, 3, 4, 0, 0, 0, ...] ou tem 10 generos, ou acaba em 0
 */
genero_t *generosPopularesIdade(database_t *db, idade_t ini, idade_t fim);

/**
 * precisa de free
 * pesquisa os usuários que gostam de um genero e estão dentro da faixa de idade
 * @param  db        previamente inicializada
 * @param  genero    genero a ser pesquisado
 * @param  ini idade minima
 * @param  fim idade máxima
 * @return           lista de IDs das pessoas que gostam do genero e tem idade entre ini-fim
 * lista do tipo [id1, id2, 0]
 * o último elemento é sempre zero
 */
id_type *usariosPorGenero(database_t *db, genero_t genero, idade_t ini, idade_t fim);

/*
Função que passa um registro do arquivo para a memória através do índex
Váriaveis:
	db- Banco de dados presente em memória
	id- id da pessoa em questão
	reg- registro completo correpondente à pessoa
	db_file- arquivo correspondente ao banco de dados
	buffer- buffer do registro da pessoa em questão
	reg_size- tamanho do registro
*/
void idToRegistro(database_t *db, id_type id, registro_t *reg);

/*
Função que verifica se uma pessoa curte um determinado gênero pelo seu id
Variáveis:
	db- Banco de dados presente em memória
	id- id da pessoa em questão
	genero- código do gênero procurado
Retorno:
	true- A pessoa curte o gênero
	false- A pessoa não curte o gênero
*/
bool idCurteGenero(database_t *db, id_type id, genero_t genero);

/*
Função que verifica se uma dada pessoa curte todos os gêneros necessários
Variáveis:
	db- Banco de dados presente em memória
	id- id da pessoa em questão
	generos- conjunto de gêneros não vazio
	i- contador
	reg- registro completo correpondente à pessoa

Retorno:
	false- a pessoa não curte pelo menos um dos gêneros no conjunto
	true- a pessoa curte todos os gêneros do conjunto
*/
bool pessoaCurteGeral(database_t *db, id_type id, genero_t *generos);

/**
 * Varre todos os IDs, obtendo todas as informações disponíveis da memória RAM
 * @param db  previamente inicializada
 * @param reg NULL para começar uma varredura nova, ponteiro carregado para saida das informações
 */
bool forAllIds(database_t *db, registro_t *reg);

/**
 * adiciona ao vetor escutam os generos que o registro curte
 * @param reg     previamente inicializado
 * @param escutam vetor de inteiros de tamanho GENSIZE
 */
void preencheEscutam(registro_t *reg, int *escutam);

/**
 * precisa de free
 * os 3 generos mais populares entres as pessoas que curtem os generos do parametro
 * @param  db      previamente inicializada
 * @param  generos generos deve ser algo como [1, 2, 3, 4, 0], o ultimo valor é sempre 0
 * @return         é algo como [1, 2, 3, 0], DEVE LER ATÉ O 0, não necessariamente tem 3 elementos
 */
genero_t *generosPopularesGenero(database_t *db, genero_t *generos);

/**
 * precisa de free
 * os 10 usuarios mais jovem de TU que curtem generos
 * se há somente 5 usuários o retorno é 1, 2, 3, 4, 5, 0, 0, 0, ...
 * generos deve ser algo como [1, 2, 3, 4, 0], o ultimo valor é sempre 0
 * ler até o 0 ou 10 elementos
 */
id_type *usuariosMaisJovems(database_t *db, genero_t *generos, tu_t tu);

/**
 * converte uma string como 'Rock' para seu código
 * @param  db  previamente inicializada
 * @param  str sem arrobas
 * @return     o valor do cod da string
 */
genero_t generoStrToCod(database_t *db, char *str);

/**
 * converte a string: 'Pagode@Rock@Metal' para seus devidos codigos
 * @param db  previamente inicializada
 * @param str será alterada
 */
void generosStrToCod(database_t *db, char *str);

/**
 * Converte Cod para uma string
 * @param  db  inicializado previamente
 * @param  cod código do genero
 * @param  str string para ser copiado
 * @return     strlen(str)
 */
uint generoCodToStr(database_t *db, genero_t cod, char *str);

uint generoCodToStr(database_t *db, genero_t cod, char *str);
void generosCodToStr(database_t *db, char *str);

id_type* monta_conjuntoGeneros(database_t *db, genero_t *generos);
bool pessoaCurteGeral(database_t *db, id_type id, genero_t *generos);
bool idCurteGenero(database_t *db, id_type id, genero_t genero);
void fill_escutam(database_t *db, id_type *conj_pessoas, int *escutam);
void idToRegistro(database_t *db, id_type id, registro_t *reg);
bool eh_tipo(database_t *db, id_type id, tu_t tu);

/**
 * converte de código para string com arrobas
 * @param db  inicializado previamente
 * @param str string com códigos dos generos
 */
void generosCodToStr(database_t *db, char *str);
#endif