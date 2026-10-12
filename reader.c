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

void findDef(FILE *f, char *def) {
    char word[1024];
    /* assumes no word exceeds length of 1023 */
    while (fscanf(f, " %1023s", word)) {
        if(strcmp(word,def) == 0) break;
    }
}

void removeSemicolon(char *line){
    strtok(line, ";");
}

int countWords(char *line){
    int words;
    char linet[1024], *token;
    strcpy(linet, line);

    words = 0;
    token = strtok(linet, " ");

    while( token != NULL ) {
        words ++;
        token = strtok(NULL, " ");
    }
   return words;
}

void readCTrucks(FILE *f, problem_instance *pi) {
    int debug=0, capacity, id;

    char line[1024];
    fgets(line, sizeof(line), f);
    fgets(line, sizeof(line), f);
    if (debug) printf("Line: %s\n", line);
    removeSemicolon(line);
    capacity = atoi(line);
    if (debug) printf("Capacity: %d\n", capacity);
    for ( id=0; id<pi->nTrucks; id++ )
        pi->trucks[id].capacity = capacity;
}

void readAlternativeNames(FILE *f, problem_instance *pi, int index) {
    int debug=0, id=0;
    char *token;

    char line[1024];
    fgets(line, sizeof(line), f);

    if (debug) printf("Line: %s\n", line);
    removeSemicolon(line);

    token=strtok(line, " ");
    while( token != NULL ) {
        if (debug) printf("%s\n", token);
        if (debug) getchar();
        pi->depots[id].names[index]=atoi(token);
        token=strtok(NULL, " ");
        id++;
    }
    if (debug) {
        for ( id=0; id<pi->nDepots; id++ )
            printf ("%d, %ld\n", pi->depots[id].id, pi->depots[id].names[index]);
    }
    if (debug) getchar();
}

void readClients(FILE *f, problem_instance *pi) {
    int debug=0, id=0;
    char * token;

    char line[1024];
    fgets(line, sizeof(line), f);

    if (debug) printf("Line: %s\n", line);
    removeSemicolon(line);
    if (debug) printf("Line: %s\n", line);
    pi->nClients=countWords(line);

    pi->clients=malloc(pi->nClients*sizeof(client));

    if (debug) printf("nClients: %d\n", pi->nClients);

    token=strtok(line, " ");
    while( token != NULL ) {
        pi->clients[id].id=id;
        pi->clients[id].name=atoi(token);
        if (debug) printf("%s\n", token);
        token = strtok(NULL, " ");

        id++;
    }
    if (debug) {
        for ( id=0; id<pi->nClients; id++ )
            printf ("%d, %d\n", pi->clients[id].id, pi->clients[id].name);
    }
    if (debug) getchar();
}


void readTrucks(FILE *f, problem_instance *pi) {
    int debug=0, id=0, i;
    char * token;

    char line[1024];
    fgets(line, sizeof(line), f);

    if (debug) printf("Line: %s\n", line);
    removeSemicolon(line);
    if (debug) printf("Line: %s\n", line);
    pi->nTrucks=countWords(line);

    pi->trucks=malloc(pi->nTrucks*sizeof(truck));

    if (debug) printf("nTrucks: %d\n", pi->nTrucks);

    token=strtok(line, " ");
    while( token != NULL ) {
        pi->trucks[id].id = id;
        pi->trucks[id].ndStart = 0;
        pi->trucks[id].ndIn = 0;
        pi->trucks[id].ndEnd = 0;
        pi->trucks[id].dStart = malloc(pi->nDepots*sizeof(int));
        pi->trucks[id].dIn = malloc(pi->nDepots*sizeof(int));
        pi->trucks[id].dEnd = malloc(pi->nDepots*sizeof(int));
        if (debug) printf("############# %d ###################\n", id);
        for(i=0;i<pi->nDepots; i++){
            if (debug) printf("%d ", pi->trucks[id].dStart[i]);
            pi->trucks[id].dStart[i] = -1;
        }

        if (debug) printf("###################################\n");
        /*if (debug) printf("%s\n", token);*/
        token = strtok(NULL, " ");
        id++;
    }
    if (debug) getchar();
}

void readDistances(FILE *f, problem_instance *pi) {
    int debug=0, i;
    int totalNodes = pi->nDepots + pi->nClients;
    int idFrom, idTo;
    char *from, *to, *distance;
    char line[1024];

    pi->distances = (int **)malloc(totalNodes * sizeof(int*));
    for(i=0; i<totalNodes; i++)
        pi->distances[i] = (int *)malloc(totalNodes * sizeof(int));

    fgets(line, sizeof(line), f);

    while (1){
        fgets(line, sizeof(line), f);
        if (debug) printf("Line: %s\n", line);

        from=strtok(line, " ");
        if (strchr(from, ';') != NULL) break;

        to  = strtok(NULL, " ");
        distance = strtok(NULL, " ");
        idFrom = getNodeId(atoi(from), *pi);
        idTo = getNodeId(atoi(to), *pi);

        if (debug) printf("from: %s, to:%s , distance: %s\n", from, to, distance);
        pi->distances[idFrom][idTo] = atoi(distance);

    }
}

void readDemands(FILE *f, problem_instance *pi) {
    int debug=0;
    int idNode;
    char *id, *demand;
    char line[1024];

   fgets(line, sizeof(line), f);

    while (1){
        fgets(line, sizeof(line), f);
        if (debug) printf("Line: %s\n", line);

        id=strtok(line, " ");
        if (strchr(id, ';') != NULL) break;

        if (!isADepot(atoi(id), *pi)){
            demand = strtok(NULL, " ");
            idNode = getNodeIndex(atoi(id), *pi);
            if (debug) printf("id: %s, idNode: %d demand: %s\n", id, idNode, demand);
            pi->clients[idNode].demand = atoi(demand);
        }
    }
}

void readTrucksDepots(FILE *f, problem_instance *pi, char * type) {
    int debug=0;
    int idDepot;
    char *depot, *truck, *flag;
    char line[1024];

    fgets(line, sizeof(line), f);

    while (1){
        fgets(line, sizeof(line), f);
        if (debug) printf("Line: %s\n", line);

        depot=strtok(line, " ");
        if (strchr(depot, ';') != NULL) break;

        truck = strtok(NULL, " ");
        int idTruck = atoi(truck)-1;
        flag = strtok(NULL, " ");
        if(atoi(flag) == 1){
            idDepot = getNodeId(atoi(depot), *pi);

            if (debug) printf("depot: %d, truck: %d, type: %s\n", idDepot, idTruck, type);

            if(strcmp(type, "start") == 0){
                if (debug) printf("start menu\n");
                if (debug) printf("agregando depot start: %d a camión %d\n", idDepot, idTruck);
                pi->trucks[idTruck].dStart[pi->trucks[idTruck].ndStart] = idDepot;
                pi->trucks[idTruck].ndStart++;
            }
            if(strcmp(type, "in") == 0){
                if (debug) printf("in menu\n");
                if (debug) printf("agregando depot in: %d a camión %d\n", idDepot, idTruck);
                pi->trucks[idTruck].dIn[pi->trucks[idTruck].ndIn] = idDepot;
                pi->trucks[idTruck].ndIn++;
            }
            if(strcmp(type, "end") == 0){
                if (debug) printf("end menu\n");
                if (debug) printf("agregando depot end: %d a camión %d\n", idDepot, idTruck);
                pi->trucks[idTruck].dEnd[pi->trucks[idTruck].ndEnd] = idDepot;
                pi->trucks[idTruck].ndEnd++;
            }
        }
    }
}

