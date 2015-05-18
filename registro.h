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
 * remove o registro tanto em memória, quanto no arquivo
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
 * @param  idade_ini idade minima
 * @param  idade_fim idade máxima
 * @return           lista de IDs das pessoas que gostam do genero e tem idade entre ini-fim
 * lista do tipo [id1, id2, 0]
 * o último elemento é sempre zero
 */
id_type *usariosPorGenero(database_t *db, genero_t genero, idade_t idade_ini, idade_t idade_fim);

/**
 * precisa de free
 * os 3 generos mais populares entres as pessoas que curtem os generos do parametro
 * @param  db      previamente inicializada
 * @param  generos generos deve ser algo como [1, 2, 3, 4, 0], o ultimo valor é sempre 0
 * @return         é algo como [1, 2, 0], ler até o zero ou 3 elementos
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
#endif