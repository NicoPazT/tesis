/* NSGA-II routine (implementation of the 'main' function) */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>

# include "global.h"
# include "rand.h"

int nreal;
int nbin;
int nobj;
int ncon;
int popsize;
double pcross_real;
double pcross_bin;
double pmut_real;
double pmut_bin;
double eta_c;
double eta_m;
int ngen;
int nbinmut;
int nrealmut;
int nbincross;
int nrealcross;
int *nbits;
double *min_realvar;
double *max_realvar;
double *min_binvar;
double *max_binvar;
int bitlength;
int choice;
int obj1;
int obj2;
int obj3;
int angle1;
int angle2;
int i;
int j;
char *csv;
char *train_csv;
char *test_csv;
double semilla;
char *instancia;
char *nuevo_nombre;
char *script_nombre;
char *rm_comando;
char *rm_comando2;
int flag = 0;
int base = 1;
int *orden;
float p_mut;
double percent_ini;

/*void guardar_info2_generacion(int generacion, population *pop, const char *instancia, float semilla) {
    char nombre_archivo[200];
    snprintf(nombre_archivo, sizeof(nombre_archivo), "info2/info2_%s_%.3f.txt", instancia, semilla);

    FILE *archivo = fopen(nombre_archivo, generacion == 1 ? "w" : "a");
    if (!archivo) {
        fprintf(stderr, "Error al abrir %s para escritura\n", nombre_archivo);
        return;
    }

    int *contador = (int *)calloc(nbin, sizeof(int));
    int i,j;

    for (i = 0; i < popsize; i++) {
        for (j = 0; j < nbin; j++) {
            if (pop->ind[i].xbin[j] == 1) {
                contador[j]++;
            }
        }
    }

    fprintf(archivo, "Generación %d\n", generacion);

    fprintf(archivo, "Presentes en todos: ");
    for (j = 0; j < nbin; j++) {
        if (contador[j] == popsize) {
            fprintf(archivo, "%d ", j);
        }
    }
    fprintf(archivo, "\n");

    fprintf(archivo, "Ausentes en todos: ");
    for (j = 0; j < nbin; j++) {
        if (contador[j] == 0) {
            fprintf(archivo, "%d ", j);
        }
    }
    fprintf(archivo, "\n\n");

    free(contador);
    fclose(archivo);
}

void guardar_info_generacion(int generacion, population *pop, const char *instancia, float semilla) {
    char nombre_archivo[200];
    snprintf(nombre_archivo, sizeof(nombre_archivo), "info/info_%s_%.3f.txt", instancia, semilla);

    FILE *archivo = fopen(nombre_archivo, generacion == 1 ? "w" : "a");
    if (!archivo) {
        fprintf(stderr, "Error al abrir %s para escritura\n", nombre_archivo);
        return;
    }

    double suma_error = 0.0, suma_characts = 0.0;
    double min_error = 1.0, max_error = 0.0;
    int min_characts = INT_MAX, max_characts = 0;

    int i;
    for (i = 0; i < popsize; i++) {
        double error = pop->ind[i].obj[0];
        int n_feat = (int) pop->ind[i].obj[1];

        suma_error += error;
        suma_characts += n_feat;

        if (error < min_error) min_error = error;
        if (error > max_error) max_error = error;

        if (n_feat < min_characts) min_characts = n_feat;
        if (n_feat > max_characts) max_characts = n_feat;
    }

    double prom_error = suma_error / popsize;
    double prom_characts = suma_characts / popsize;

    fprintf(archivo,
        "Gen %d\tError: min=%.4f max=%.4f avg=%.4f\tCharacts: min=%d max=%d avg=%.2f\n",
        generacion, min_error, max_error, prom_error,
        min_characts, max_characts, prom_characts);

    fclose(archivo);
}*/

void obtener_nombre_sin_extension(const char *path_completo, char *nombre_sin_ext) {
    const char *base = strrchr(path_completo, '/');
    if (!base) base = path_completo;
    else base++; 

    strncpy(nombre_sin_ext, base, 100);
    char *punto = strrchr(nombre_sin_ext, '.');
    if (punto) *punto = '\0';
}

/*void generar_archivos_train_test(const char *ruta_csv, double semilla, int base) {
    char nombre_instancia[100];
    obtener_nombre_sin_extension(ruta_csv, nombre_instancia);

    char nombre_train[200], nombre_test[200];
    snprintf(nombre_train, sizeof(nombre_train), "%s_%.3f_train_%d.csv", nombre_instancia, semilla, base);
    snprintf(nombre_test, sizeof(nombre_test), "%s_%.3f_test_%d.csv", nombre_instancia, semilla, base);

    char comando[8192];
    snprintf(comando, sizeof(comando),
        "awk -v seed=%f '"
        "BEGIN{srand(seed)} "
        "NR==1{header=$0; next} "
        "{line[NR]=$0} "
        "END{ "
        "   n=NR-1; "
        "   n_train=int(n*0.8); "
        "   for(i=2;i<=NR;i++){ "
        "       j=2+int(rand()*(NR-1)); "
        "       tmp=line[i]; line[i]=line[j]; line[j]=tmp; "
        "   } "
        "   print header > \"%s\"; "
        "   print header > \"%s\"; "
        "   for(i=2;i<=n_train+1;i++) print line[i] >> \"%s\"; "
        "   for(i=n_train+2;i<=NR;i++) print line[i] >> \"%s\"; "
        "}' %s",
        semilla, nombre_train, nombre_test, nombre_train, nombre_test, ruta_csv
    );

    system(comando);
}*/

void generar_archivos_train_test(const char *ruta_csv, double semilla, int base) {
    char nombre_instancia[100];
    obtener_nombre_sin_extension(ruta_csv, nombre_instancia);

    char semilla_str[50];
    snprintf(semilla_str, sizeof(semilla_str), "%.3f", semilla);

    char comando[2048];
    char archivo_test[256];
    snprintf(archivo_test, sizeof(archivo_test), "%s_%s_test_%d.csv",
             nombre_instancia, semilla_str, base);

    int valido = 0;

    while (!valido) {
        snprintf(comando, sizeof(comando),
            "awk -F, '"
            "BEGIN {srand()} "
            "NR==1 {print > \"%s_%s_train_%d.csv\"; print > \"%s_%s_test_%d.csv\"; next} "
            "{if (rand() <= 0.8) print > \"%s_%s_train_%d.csv\"; "
            "else print > \"%s_%s_test_%d.csv\"}' "
            "%s",
            nombre_instancia, semilla_str, base,
            nombre_instancia, semilla_str, base,
            nombre_instancia, semilla_str, base,
            nombre_instancia, semilla_str, base,
            ruta_csv
        );
        system(comando);

        size_t len = snprintf(NULL, 0, "Rscript verificar_test.R %s", archivo_test);
        char *comando_r = malloc(len + 1);
        if (!comando_r) {
            fprintf(stderr, "Error: no se pudo asignar memoria para comando_r\n");
            exit(1);
        }
        snprintf(comando_r, len + 1, "Rscript verificar_test.R %s", archivo_test);

        int status = system(comando_r);
        free(comando_r);

        if (status == 0) {
            valido = 1; 
        }
    }
}

/*void generar_archivos_train_test(const char *ruta_csv, double semilla, int base) {
    char nombre_instancia[100];
    obtener_nombre_sin_extension(ruta_csv, nombre_instancia);

    char semilla_str[50];
    snprintf(semilla_str, sizeof(semilla_str), "%.3f", semilla);

    char comando[2048];
    snprintf(comando, sizeof(comando),
        "awk -F, '"
        "BEGIN {srand()} "
        "NR==1 {print > \"%s_%s_train_%d.csv\"; print > \"%s_%s_test_%d.csv\"; next} "
        "{if (rand() <= 0.8) print > \"%s_%s_train_%d.csv\"; "
        "else print > \"%s_%s_test_%d.csv\"}' "
        "%s",
        nombre_instancia, semilla_str, base,
        nombre_instancia, semilla_str, base,
        nombre_instancia, semilla_str, base,
        nombre_instancia, semilla_str, base,
        ruta_csv
    );
    system(comando);
}*/

int main (int argc, char **argv)
{
    int i;

    time_t inicio_total, fin_total;
    time(&inicio_total);

    /*struct problem_instance * pi;*/
    population *parent_pop;
    population *child_pop;
    population *mixed_pop;
    if (argc<2)
    {
        printf("\n Usage ./nsga2r instance_route random_seed popsize ngen nobj pcross_bin pmut_bin\n./nsga2r 0.123 b-Instancia14_cap2_relacion7UnoUnoUnoTodosDistintos.dat 100 100 2 0.6 0.01\n");
        exit(1);
    }
    base = atoi(argv[8]);
    if (base != 0 && base != 1 && base != 2) {
        printf("Error: base debe ser 0 (aleatoria), 1 (chi-cuadrado) o 2 (híbrido)\n");
        exit(1);
    }
    percent_ini = atof(argv[9]);
    seed = (double)atof(argv[1]);
    if (seed<=0.0 || seed>=1.0){
        printf("\n Entered seed value is wrong, seed value must be in (0,1) \n");
        exit(1);
    }

    semilla = seed;

    csv = argv[2];

    char *start = strrchr(csv, '/');
    start++;
    char *end = strstr(start, ".csv");
    size_t len = end - start;
    instancia = malloc(len + 1);
    strncpy(instancia, start, len);
    instancia[len] = '\0';

    size_t size = snprintf(NULL, 0, "cols_%s_%.3f_%d", instancia, semilla, base) + 1;
    nuevo_nombre = malloc(size);
    snprintf(nuevo_nombre, size, "cols_%s_%.3f_%d", instancia, semilla, base);


    size = snprintf(NULL, 0, "scriptR_%s_%.3f_%d.csv", instancia, semilla, base) + 1;
    script_nombre = malloc(size);
    snprintf(script_nombre, size, "scriptR_%s_%.3f_%d.csv", instancia, semilla, base);
    generar_archivos_train_test(csv, semilla, base);

    char nombre_instancia[100];
    obtener_nombre_sin_extension(csv, nombre_instancia);
    size = snprintf(NULL, 0, "%s_%.3f_train_%d.csv", nombre_instancia, semilla, base) + 1;
    train_csv = malloc(size);
    snprintf(train_csv, size, "%s_%.3f_train_%d.csv", nombre_instancia, semilla, base);

    size = snprintf(NULL, 0, "%s_%.3f_test_%d.csv", nombre_instancia, semilla, base) + 1;
    test_csv = malloc(size);
    snprintf(test_csv, size, "%s_%.3f_test_%d.csv", nombre_instancia, semilla, base);
    readFile(csv);

    size = snprintf(NULL, 0, "rm -f %s.csv %s", nuevo_nombre, script_nombre) + 1;
    rm_comando = malloc(size);
    snprintf(rm_comando, size, "rm -f %s.csv %s", nuevo_nombre, script_nombre);

    size = snprintf(NULL, 0, "rm -f %s %s", train_csv, test_csv) + 1;
    rm_comando2 = malloc(size);
    snprintf(rm_comando2, size, "rm -f %s %s", train_csv, test_csv);

    popsize = atoi(argv[3]);
    if (popsize<4 || (popsize%4)!= 0){
        printf("\n population size read is : %d",popsize);
        printf("\n Wrong population size entered, hence exiting \n");
        exit (1);
    }
    ngen = atoi(argv[4]);
    if (ngen<1){
        printf("\n number of generations read is : %d",ngen);
        printf("\n Wrong nuber of generations entered, hence exiting \n");
        exit (1);
    }
    /*nobj = atoi(argv[5]);
    if (nobj<1){
        printf("\n number of objectives entered is : %d",nobj);
        printf("\n Wrong number of objectives entered, hence exiting \n");
        exit (1);
    }*/
    nobj = 2;

    nbits = (int *)malloc(nbin * sizeof(int));
    min_binvar = (double *)malloc(nbin * sizeof(double));
    max_binvar = (double *)malloc(nbin * sizeof(double));

    for (i = 0; i < nbin; i++) {
        nbits[i] = 1;            /*Cada característica es representada por 1 bit*/
        min_binvar[i] = 0.0;      /*Valor mínimo de la variable binaria (no seleccionada)*/
        max_binvar[i] = 1.0;      /*Valor máximo de la variable binaria (seleccionada)*/
    }

    pcross_bin = atof (argv[5]);
    if (pcross_bin<0.0 || pcross_bin>1.0){
        printf("\n Probability of crossover entered is : %e",pcross_bin);
        printf("\n Entered value of probability of crossover of binary variables is out of bounds, hence exiting \n");
        exit (1);
    }
    pmut_bin = atof (argv[6]);
    /*if (pmut_bin<0.0 || pmut_bin>1.0){*/
    if (pmut_bin<0.0){
        printf("\n Probability of mutation entered is : %e",pmut_bin);
        printf("\n Entered value of probability  of mutation of binary variables is out of bounds, hence exiting \n");
        exit (1);
    }
    p_mut = atof(argv[7]);
    if (p_mut < 0.0 || p_mut > 1.0) {
    printf("\nValor inválido de p_mut: %f\nDebe estar entre 0 y 1.\n", p_mut);
    exit(1);
    }

    pmut_bin = pmut_bin * (1.0/nbin);
    /*printf("nbin: %d\n", nbin);*/

    bitlength = 0;
    if (nbin!=0)
    {
        for (i=0; i<nbin; i++)
        {
            bitlength += nbits[i];
        }
    }
    nbinmut = 0;
    nrealmut = 0;
    nbincross = 0;
    nrealcross = 0;
    parent_pop = (population *)malloc(sizeof(population));
    child_pop = (population *)malloc(sizeof(population));
    mixed_pop = (population *)malloc(sizeof(population));
    allocate_memory_pop (parent_pop, popsize);
    allocate_memory_pop (child_pop, popsize);
    allocate_memory_pop (mixed_pop, 2*popsize);
    randomize();
    initialize_pop (parent_pop);
    /*printf("\n Initialization done, now performing first generation\n");*/
    /*decode_pop(parent_pop);*/
    evaluate_pop (parent_pop);
    assign_rank_and_crowding_distance (parent_pop);
    /*printf("Initial\n");
    report_feasible(parent_pop);*/
    /*guardar_info_generacion(1, parent_pop, instancia, seed);
    guardar_info2_generacion(1, parent_pop, instancia, seed);*/
    for (i=2; i<=ngen; i++)
    {
        selection (parent_pop, child_pop);
        mutation_pop (child_pop);
        /*decode_pop(child_pop);*/
        evaluate_pop(child_pop);
        merge (parent_pop, child_pop, mixed_pop);
        fill_nondominated_sort (mixed_pop, parent_pop);
        /*guardar_info_generacion(i, parent_pop, instancia, seed);
        guardar_info2_generacion(i, parent_pop, instancia, seed);*/
    }
    /*printf("\n Generations finished, now reporting solutions");*/
    printf("Training\n");
    report_feasible(parent_pop);
    flag = 1;
    evaluate_pop(parent_pop);
    /*assign_rank_and_crowding_distance (parent_pop);*/
    printf("Testing\n");
    report_feasible(parent_pop);
    if (nbin!=0)
    {
        free (min_binvar);
        free (max_binvar);
        free (nbits);
    }
    deallocate_memory_pop (parent_pop, popsize);
    deallocate_memory_pop (child_pop, popsize);
    deallocate_memory_pop (mixed_pop, 2*popsize);
    free (parent_pop);
    free (child_pop);
    free (mixed_pop);
    system(rm_comando);
    system(rm_comando2);
    if (instancia) free(instancia);
    if (nuevo_nombre) free(nuevo_nombre);
    if (train_csv) free(train_csv);
    if (test_csv) free(test_csv);
    if (script_nombre) free(script_nombre);
    if (rm_comando) free(rm_comando);
    if (rm_comando2) free (rm_comando2);
    if (orden) free (orden);

    /*printf("\n Routine successfully exited \n");*/
    time(&fin_total);
    double total_execution_time = difftime(fin_total, inicio_total);
    printf("Total execution time: %.2f seconds\n", total_execution_time);
    printf("Total execution time: %.2f minutes\n", total_execution_time / 60.0);
    return (0);
}
