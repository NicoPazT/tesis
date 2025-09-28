args <- commandArgs(trailingOnly = TRUE)
file_test <- args[1]

datos <- read.csv(file_test, header = TRUE)
freqs <- table(datos[[1]])   # primera columna = clase

# Buscar clases con frecuencia 1
valido <- TRUE
for (cl in names(freqs)) {
  if (freqs[cl] == 1) {
    restantes <- freqs[names(freqs) != cl]
    if (sum(restantes > 0) == 1) {
      valido <- FALSE
      break
    }
  }
}

if (valido) {
  quit(status = 0)   # OK → código de salida 0
} else {
  quit(status = 1)   # inválido → código de salida 1
}