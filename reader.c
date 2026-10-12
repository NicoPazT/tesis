/* Routine for evaluating population members  */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <string.h>
# include <unistd.h>
# include <sys/types.h>
#include <time.h>
# include "global.h"
# include "rand.h"

char ***matrix;  /*Definición de la matriz*/
int num_rows;    /*Número de filas*/
int num_columns; /*Número de columnas*/

int count_columns(const char *line) {
    int count = 0;
    const char *ptr = line;
    while (*ptr) {
        if (*ptr == ',') {
            count++;
        }
        ptr++;
    }
    return count; 
}

size_t find_max_line_length(FILE *file) {
    size_t max_length = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1) {
        if (read > max_length) {
            max_length = read;
        }
    }

    free(line);
    fseek(file, 0, SEEK_SET);

    return max_length;
}

int readFile(char* filePath) {

    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        printf("No se pudo abrir el archivo.\n");
        return -1;
    }

    size_t max_line_length = find_max_line_length(file);
    char line[max_line_length];

    fgets(line, max_line_length, file);
    nbin = count_columns(line);
    fseek(file, 0, SEEK_SET);

    fclose(file);
    return 0;
}
