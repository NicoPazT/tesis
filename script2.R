library(FNN)

load_data <- function(file_name) {
  data <- read.csv(file_name)
  X <- as.matrix(data[, -1])
  y <- data[, 1]
  return(list(X = X, y = y))
}

# Función para crear folds de manera estratificada
create_stratified_folds <- function(y, k) {
  class_indices <- split(1:length(y), y)
  folds <- vector("list", k)
  for (i in 1:k) folds[[i]] <- numeric(0)

  for (class_idx in class_indices) {
    shuffled_idx <- sample(class_idx)
    for (i in seq_along(shuffled_idx)) {
      fold_idx <- (i - 1) %% k + 1
      folds[[fold_idx]] <- c(folds[[fold_idx]], shuffled_idx[i])
    }
  }
  return(folds)
}

evaluate_feature_subset <- function(X, y, selected_features, max_folds = 10, random_seed = 42) {
  if (sum(selected_features) == 0) {
    return(list(accuracy = 0, n_features = 0))
  }

  selected_indices <- which(selected_features == 1)
  X_selected <- X[, selected_indices, drop = FALSE]

  # Eliminar clases con solo una muestra
  class_counts <- table(y)
  rare_classes <- names(class_counts[class_counts == 1])
  if (length(rare_classes) > 0) {
    keep_indices <- !(y %in% rare_classes)
    X_selected <- X_selected[keep_indices, , drop = FALSE]
    y <- y[keep_indices]
  }

  if (length(unique(y)) < 2) {
    return(list(accuracy = 0, n_features = 100000))
  }

  class_counts <- table(y)
  min_class_size <- min(class_counts)
  n_splits <- min(max_folds, min_class_size)

  if (n_splits < 2) {
    return(list(accuracy = 0, n_features = 100000))
  }

  set.seed(random_seed)
  folds <- create_stratified_folds(y, n_splits)
  accuracies <- numeric(length(folds))

  for (i in seq_along(folds)) {
    test_indices <- folds[[i]]
    train_indices <- setdiff(seq_along(y), test_indices)

    X_train <- X_selected[train_indices, , drop = FALSE]
    X_test <- X_selected[test_indices, , drop = FALSE]
    y_train <- y[train_indices]
    y_test <- y[test_indices]

    k_val <- min(3, nrow(X_train))

    # Asegurarse de que haya al menos dos clases en el conjunto de entrenamiento
    if (length(unique(y_train)) < 2) {
      accuracies[i] <- 0  # Si no hay suficientes clases en el entrenamiento, asignamos 0
    } else {
      predictions <- knn(train = X_train,
                         test = X_test,
                         cl = y_train,
                         k = k_val)
      accuracies[i] <- mean(predictions == y_test)
    }
  }

  return(list(
    accuracy = mean(accuracies),
    n_features = sum(selected_features)
  ))
}

# Script execution
args <- commandArgs(trailingOnly = TRUE)

if (length(args) < 2) {
  stop("Se requieren dos argumentos: nombre_dataset semilla")
}

dataset_name <- args[1]
seed <- as.integer(args[2])

set.seed(seed)

data <- load_data(dataset_name)
X <- data$X
y <- data$y

num_features <- ncol(X)
selected_features <- rep(1, num_features)

# Evaluar subconjunto
result <- evaluate_feature_subset(X, y, selected_features, random_seed = seed)

cat(result$accuracy, result$n_features, "\n")