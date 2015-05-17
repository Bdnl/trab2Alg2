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

int main(int argc, char *argv[]) {
	#ifdef DEBUG
		registro_t reg_new = {1, "Nome", 20, 'M', "Rock@Pagode@Axe", 1};
		database_t db;
		initDB(&db);
		generosStrToCod(&db, reg_new.generos);
		novoRegistro(&db, &reg_new);
		closeDB(&db);
	#endif // DEBUG
	return 0;
}