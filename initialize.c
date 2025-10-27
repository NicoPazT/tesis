/* Data initializtion routines */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>

# include "global.h"
# include "rand.h"

int* leer_chi_ini(const char* filename) {
    int i;
    int* chi = (int*)malloc(nbin * sizeof(int));
    if (chi == NULL) {
        printf("Error: malloc falló al asignar memoria para chi (%d enteros)\n", nbin);
        exit(1);
    }
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error al abrir el archivo de chi: %s\n", filename);
        exit(1);
    }
    for (i = 0; i < nbin; i++) {
        fscanf(file, "%d", &chi[i]);
    }
    fclose(file);
    return chi;
}

int* leer_orden(const char* filename) {
    int i;
    int* vec = (int*)malloc(nbin * sizeof(int));
    if (vec == NULL) {
        printf("Error: malloc falló al asignar memoria para chi (%d enteros)\n", nbin);
        exit(1);
    }
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error al abrir el archivo de orden: %s\n", filename);
        exit(1);
    }
    for (i = 0; i < nbin; i++) {
        fscanf(file, "%d", &vec[i]);
        vec[i]--;
    }
    fclose(file);
    return vec;
}

/* Function to initialize a population randomly */
void initialize_pop(population *pop) {
    int i;

    if (base == 0) {
        for (i = 0; i < popsize; i++) {
            initialize_ind(&(pop->ind[i]), i);
        }
    } else if (base == 1) {
        float prob;
        if (nbin < 100) {
            prob = 0.1;
        } else {
            prob = 0.001;
        }
        int limite = nbin / 2;
        float temp_variacion = limite * prob;
        int variacion = (int)ceil(temp_variacion);

        char command[256];
        char nombre_archivo[128];
        char nombre_orden[128];
        if (instancia == NULL) {
            printf("ERROR: 'instancia' es NULL\n");
            exit(1);
        }
        sprintf(nombre_archivo, "ini_%s_%.3f_%d.txt", instancia, semilla, base);
        sprintf(nombre_orden, "orden_%s_%.3f_%d.txt", instancia, semilla, base);
        sprintf(command, "python3 chi2.py %s %s %s %.2f", train_csv, nombre_archivo, nombre_orden, percent_ini);
        int result = system(command);
        if (result != 0) {
            printf("Error al ejecutar el script Python para generar el archivo de inicialización.\n");
            exit(1);
        }
        orden = leer_orden(nombre_orden);
        int* chi_ini = leer_chi_ini(nombre_archivo);
        for (i = 0; i < popsize; i++) {
            initialize_ind_chi(&(pop->ind[i]), i, chi_ini, variacion, limite);
        }
        free(chi_ini);

        char rm_command[128];
        sprintf(rm_command, "rm %s %s", nombre_archivo, nombre_orden);
        system(rm_command);
    } else if (base == 2) {
        char command[256];
        char nombre_archivo[128];
        char nombre_orden[128];
        if (instancia == NULL) {
            printf("ERROR: 'instancia' es NULL\n");
            exit(1);
        }
        sprintf(nombre_archivo, "ini_%s_%.3f_%d.txt", instancia, semilla, base);
        sprintf(nombre_orden, "orden_%s_%.3f_%d.txt", instancia, semilla, base);
        sprintf(command, "python3 chi2.py %s %s %s %.2f", train_csv, nombre_archivo, nombre_orden, percent_ini/2); 
        int result = system(command);
        if (result != 0) {
            printf("Error al ejecutar el script Python para generar el archivo de inicialización.\n");
            exit(1);
        }

        orden = leer_orden(nombre_orden);
        int* chi_ini = leer_chi_ini(nombre_archivo);

        for (i = 0; i < popsize; i++) {
            initialize_ind_chi_random(&(pop->ind[i]), chi_ini);
        }

        free(chi_ini);

        char rm_command[128];
        sprintf(rm_command, "rm %s %s", nombre_archivo, nombre_orden);
        system(rm_command);
    } else if (base == 3) {
    char command[256];
    char nombre_orden[128];

    if (instancia == NULL) {
        printf("ERROR: 'instancia' es NULL\n");
        exit(1);
    }

    sprintf(nombre_orden, "orden_%s_%.3f_%d.txt", instancia, semilla, base);
    sprintf(command, "python3 chi2_nuevo.py %s %s %.2f", train_csv, nombre_orden, percent_ini);
    int result = system(command);
    if (result != 0) {
        printf("Error al ejecutar el script Python (base 3).\n");
        exit(1);
    }

    FILE* file = fopen(nombre_orden, "r");
    if (!file) {
        printf("Error al abrir el archivo: %s\n", nombre_orden);
        exit(1);
    }

    int* orden_local = (int*)malloc(nbin * sizeof(int));
    if (!orden_local) {
        printf("Error: malloc falló al asignar memoria para orden_local (%d enteros)\n", nbin);
        exit(1);
    }

    int count = 0;
    while (fscanf(file, "%d", &orden_local[count]) == 1) {
        orden_local[count]--;
        count++;
    }
    fclose(file);

    if (count == 0) {
        printf("Error: archivo %s vacío o sin índices válidos\n", nombre_orden);
        exit(1);
    }

    nbin = count;
    printf("nbin: %d\n", nbin);
    orden = orden_local; 

    int i;
    for (i = 0; i < popsize; i++) {
        initialize_ind_order(&(pop->ind[i]), orden);
    }

    char rm_command[128];
    sprintf(rm_command, "rm %s", nombre_orden);
    system(rm_command);

    /*printf("Inicialización base=3 completada correctamente (count=%d, percent_ini=%.2f)\n", count, percent_ini);*/
    }
    /*int suma_total_caracteristicas = 0;
    int j;
    for (i = 0; i < popsize; i++) {
        int contador = 0;
        for (j = 0; j < nbin; j++) {
            if (pop->ind[i].xbin[j] == 1.0) {
                contador++;
            }
        }
        suma_total_caracteristicas += contador;
    }
    float promedio_caracteristicas = (float)suma_total_caracteristicas / popsize;
    printf("Promedio de características seleccionadas en la población inicial: %.2f\n", promedio_caracteristicas);*/
    return;
}

/* Function to initialize an individual randomly */
void initialize_ind (individual *ind, int i)
{
    float percent = 1 - percent_ini;
    int j, k;
    /*if (i == 0)
    {
        percent = 0.0;
    }*/
    if (nreal!=0)
    {
        for (j=0; j<nreal; j++)
        {
            ind->xreal[j] = rndreal (min_realvar[j], max_realvar[j]);
        }
    }
    if (nbin!=0)
    {
        /*ind->characts = NULL;*/
        /*for (j=0; j<nbin; j++)
        {
            for (k=0; k<nbits[j]; k++)
            {
                if (randomperc() <= percent)
                {
                    ind->gene[j][k] = 0;
                }
                else
                {
                    ind->gene[j][k] = 1;
                }
            }
        }*/
        for (j = 0; j < nbin; j++) {
            ind->xbin[j] = (randomperc() <= percent) ? 0.0 : 1.0;
        }
    }
    return;
}

void initialize_ind_chi(individual *ind, int i, int* chi_ini, int variacion, int limite) {
    int j;

    if (i == 0) {
        for (j = 0; j < nbin; j++) {
            ind->xbin[j] = (float)chi_ini[j];
        }
        return;
    }
    
    if (variacion > limite) {
        printf("ERROR: variacion (%d) no puede ser mayor que limite (%d)\n", variacion, limite);
        exit(1);
    }

    int* usados = (int*)calloc(nbin, sizeof(int));
    if (!usados) {
        printf("Error: malloc falló al asignar memoria para usados (%d enteros)\n", nbin);
        exit(1);
    }
    for (j = 0; j < nbin; j++) {
        ind->xbin[j] = (float)chi_ini[j];
    }

    int mutaciones_realizadas = 0;
    while (mutaciones_realizadas < variacion) {
        int r = rnd(0, limite - 1);
        int pos = orden[r];

        if (!usados[pos]) {
            usados[pos] = 1;
            ind->xbin[pos] = 1.0 - ind->xbin[pos];
            mutaciones_realizadas++;
        }
    }

    free(usados);
    return;
}

void initialize_ind_chi_random(individual *ind, int* chi_ini) {
    int j;

    int* selected_by_chi = (int*)calloc(nbin, sizeof(int)); 
    if (!selected_by_chi) {
        printf("Error: malloc falló al asignar memoria para selected_by_chi (%d enteros)\n", nbin);
        exit(1);
    }

    for (j = 0; j < nbin; j++) {
        ind->xbin[j] = (float)chi_ini[j]; 
        if (ind->xbin[j] == 1.0) { 
            selected_by_chi[j] = 1;
        }
    }

    double percent_ini_2 = percent_ini / 2;
    int num_random_features = (int)ceil(nbin * percent_ini_2); 

    int random_count = 0;
    while (random_count < num_random_features) {
        int r = rnd(0, nbin - 1); 
        if (!selected_by_chi[r]) { 
            ind->xbin[r] = 1.0;
            selected_by_chi[r] = 1;
            random_count++;
        }
    }

    free(selected_by_chi); 
    return;
}

void initialize_ind_order(individual *ind, int *orden)
{
    float percent = 1 - percent_ini;
    int j;

    for (j = 0; j < nbin; j++) {
        ind->xbin[j] = 0.0;
    }

    for (j = 0; j < nbin; j++) {
        int z = orden[j];
        if (z >= 0 && z < nbin) {
            if (randomperc() > percent) {
                ind->xbin[z] = 1.0;
            }
        }
    }
}