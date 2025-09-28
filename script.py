import numpy as np
import pandas as pd
import argparse
from sklearn.model_selection import StratifiedKFold
from sklearn.neighbors import KNeighborsClassifier
from sklearn.metrics import accuracy_score
from collections import Counter

def load_data(file_name):
    data = pd.read_csv(file_name)
    X = data.iloc[:, 1:].values  
    y = data.iloc[:, 0].values   
    return X, y

def evaluate_feature_subset(X, y, selected_features, max_folds=10, random_state=42):
    if np.sum(selected_features) == 0:
        return 0, 0  

    selected_indices = np.where(selected_features == 1)[0]
    X_selected = X[:, selected_indices]

    min_class_size = min(Counter(y).values())
    n_splits = min(max_folds, min_class_size)  
    if n_splits < 2:
        return 0, 0  

    skf = StratifiedKFold(n_splits=n_splits, shuffle=True, random_state=random_state)
    accuracies = []

    for train_index, test_index in skf.split(X_selected, y):
        X_train, X_test = X_selected[train_index], X_selected[test_index]
        y_train, y_test = y[train_index], y[test_index]

        knn = KNeighborsClassifier(n_neighbors=3)
        knn.fit(X_train, y_train)
        y_pred = knn.predict(X_test)

        accuracies.append(accuracy_score(y_test, y_pred))

    return np.mean(accuracies), np.sum(selected_features)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("dataset_name", type=str)
    parser.add_argument("seed", type=int)
    args = parser.parse_args()

    X, y = load_data(args.dataset_name)
    num_features = X.shape[1]
    np.random.seed(args.seed)
    # selected_features = np.random.choice([0, 1], size=num_features)
    selected_features = np.ones(num_features, dtype=int) 

    acc, num_features = evaluate_feature_subset(X, y, selected_features, random_state=args.seed)
    print(acc, num_features)