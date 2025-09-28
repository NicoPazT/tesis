/* Routine for evaluating population members  */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <string.h>
# include <time.h>

# include "global.h"
# include "rand.h"

/* Función para convertir xbin a lista binaria (0 o 1) */
void convertir_a_lista_binaria(const double *xbin, int *lista) {
    int j;
    for (j = 0; j < nbin; j++) {
        if (xbin[j] > 0.5) {
            lista[j] = 1;
        } else {
            lista[j] = 0;
        }
    }
}

void construir_comando_awk(char *comando, const int *lista) {
    comando[0] = '\0';
    int i;
    char buffer[100];

    strcpy(comando, "gawk -F, '{OFS=\",\";print $1");

    for (i = 0; i < nbin; i++) {
        if (lista[i] == 1) {
            size_t used = strlen(comando);
            size_t need = 1 + snprintf(NULL, 0, "$%d", i + 2);
            if (used + need + 1 >= 100000) {
                fprintf(stderr, "Buffer overflow prevented at column %d\n", i + 2);
                break;
            }
            strcat(comando, ",");
            snprintf(buffer, sizeof(buffer), "$%d", i + 2);
            strcat(comando, buffer);
        }
    }

    strcat(comando, "}' ");

    size_t used = strlen(comando);
    size_t suffix_len = strlen(train_csv) + strlen(nuevo_nombre) + 10;
    if (used + suffix_len >= 100000) {
        fprintf(stderr, "Buffer overflow prevented when appending filenames\n");
        return;
    }

    strcat(comando, train_csv);
    strcat(comando, " > ");
    strcat(comando, nuevo_nombre);
    strcat(comando, ".csv");
}

void construir_comando_awk_test(char *comando, const int *lista) {
    comando[0] = '\0';
    int i;
    char buffer[100];

    strcpy(comando, "gawk -F, '{OFS=\",\";print $1");

    for (i = 0; i < nbin; i++) {
        if (lista[i] == 1) {
            size_t used = strlen(comando);
            size_t need = 1 + snprintf(NULL, 0, "$%d", i + 2);
            if (used + need + 1 >= 100000) {
                fprintf(stderr, "Buffer overflow prevented at column %d in test\n", i + 2);
                break;
            }
            strcat(comando, ",");
            snprintf(buffer, sizeof(buffer), "$%d", i + 2);
            strcat(comando, buffer);
        }
    }

    strcat(comando, "}' ");
    strcat(comando, test_csv);
    strcat(comando, " > ");
    strcat(comando, nuevo_nombre);
    strcat(comando, ".csv");
}

double ejecutar_script_Python(int seed, int *num_variables, individual *ind) {
    char comando[256];
    double acc = 0.0;
    int i;

    if (ind->characts != NULL) {
        free(ind->characts);
        ind->characts = NULL;
    }
    printf("La semilla es: %d", seed);
    snprintf(comando, sizeof(comando), "python3 script.py %s.csv %d > %s\n", nuevo_nombre, seed, script_nombre);

    clock_t inicio = clock();
    system(comando);
    clock_t fin = clock();
    double tiempo_transcurrido = ((double)(fin - inicio)) / CLOCKS_PER_SEC;

    FILE *file = fopen(script_nombre, "r");
    if (file == NULL) {
        fprintf(stderr, "Error abriendo scriptPython.csv\n");
        return -1;
    }

    fscanf(file, "%lf %d", &acc, num_variables);

    fclose(file);

    return acc;
}

double ejecutar_script_R(int seed, int *num_variables, individual *ind) {
    char comando[256];
    double acc = 0.0;
    int i;

    if (ind->characts != NULL) {
        free(ind->characts);
        ind->characts = NULL;
    }

    /*snprintf(comando, sizeof(comando), "Rscript script3.R %s %d > %s\n", nuevo_nombre, seed, script_nombre);*/
    snprintf(comando, sizeof(comando), "Rscript script2.R %s.csv %d > %s\n", nuevo_nombre, seed, script_nombre);

    clock_t inicio = clock();
    system(comando);
    clock_t fin = clock();
    double tiempo_transcurrido = ((double)(fin - inicio)) / CLOCKS_PER_SEC;

    FILE *file = fopen(script_nombre, "r");
    fscanf(file, "%lf %d", &acc, num_variables);

    /*ind->characts = (int *)malloc(*num_variables * sizeof(int));

    for (i = 0; i < *num_variables; i++) {
        fscanf(file, "%d", &(ind->characts[i]));
    }*/
    /*printf("Número de variables: %d\n", *num_variables);*/

    fclose(file);

    /*printf("Características utilizadas:\n");
    for (i = 0; i < *num_variables; i++) {
        printf("%d ", ind->characts[i]);
    }
    printf("\n");

    printf("Tiempo de ejecución de Rscript: %f segundos\n", tiempo_transcurrido);*/

    return acc;
}

void evaluate_pop (population *pop)
{
    int i;
    for (i=0; i<popsize; i++)
    {
        if (flag == 1){
            evaluate_ind (&(pop->ind[i]));
        } else {
            evaluate_ind (&(pop->ind[i]));
        }
    }
    return;
}

void evaluate_ind (individual *ind)
{
    if (ind->evaluado != 1 || flag == 1) {
        int *lista_binaria = (int *)malloc(nbin * sizeof(int));
        convertir_a_lista_binaria(ind->xbin, lista_binaria);
        int j;

        int num_unos = 0;
        for (j = 0; j < nbin; j++) {
            if (lista_binaria[j] == 1) {
                num_unos++;
            }
        }

        if (num_unos == 0) {
            ind->obj[0] = 1.0;
            ind->obj[1] = 1000000;
            ind->evaluado = 1;
            free(lista_binaria);
            return;
        }
        
        char comando[100000];
        if (flag == 1) {
            construir_comando_awk_test(comando, lista_binaria);
        } else {
            construir_comando_awk(comando, lista_binaria);
        }
        system(comando);
        int num_variables;
        /*double acc = ejecutar_script_R(100, &num_variables, ind);*/
        double err = ejecutar_script_R(100, &num_variables, ind);
        
        if (err != -1) {
            ind->obj[0] = 1 - err;       
            ind->obj[1] = num_variables;
        } else {
            ind->obj[0] = 1.0;
            ind->obj[1] = 100000;
        }

        /*if (flag == 1) {
            getchar();
        }*/

        free(lista_binaria);

        if (ncon==0)
        {
            ind->constr_violation = 0.0;
        }
        else
        {
            ind->constr_violation = 0.0;
            for (j=0; j<ncon; j++)
            {
                if (ind->constr[j]<0.0)
                {
                    ind->constr_violation += ind->constr[j];
                }
            }
        }
        ind->evaluado = 1;
    }
    return;
}
