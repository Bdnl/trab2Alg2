#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "misc.h"
#include "database.h"

#ifdef TEST

int main(int argc, char *argv[]) {
	database_t db;
	initDB(&db);
	genero_t generos_para_teste[] = "Rock";
	generosStrToCod(&db, generos_para_teste);
	generosPopularesGenero(&db, generos_para_teste);
	usariosPorGenero(&db, generos_para_teste[0], 10, 20);
	usuariosMaisJovems(&db, generos_para_teste, 1);
	closeDB(&db);
	return 0;
}

#endif // TEST