import sys
import numpy as np
import pandas as pd
from sklearn.feature_selection import chi2
from sklearn.preprocessing import MinMaxScaler, KBinsDiscretizer
import re

# === LECTURA DE PARÁMETROS ===
input_path = sys.argv[1]        
nombre_salida = sys.argv[2]      
nombre_salida_indices = sys.argv[3] 
k_percent = float(sys.argv[4])

# === CARGA DE DATOS ===
df = pd.read_csv(input_path)
y = df.iloc[:, 0].values
X = df.iloc[:, 1:].values
feature_names = df.columns[1:]  # Nombres como "cat_1", "cat_2", ...

# === CONDICIÓN: usar discretización solo si NO es "chess" ===
if "chess" in nombre_salida.lower():
    X_proc = X
else:
    disc = KBinsDiscretizer(n_bins=5, encode="ordinal", strategy="uniform")
    X_proc = disc.fit_transform(X)

# === CHI² ===
chi2_scores, _ = chi2(X_proc, y)
chi2_scores = MinMaxScaler().fit_transform(chi2_scores.reshape(-1, 1)).flatten()

# === MÁSCARA BINARIA ===
k = max(1, int(len(chi2_scores) * k_percent))
top_indices = np.argsort(chi2_scores)[-k:]
mask = [1 if i in top_indices else 0 for i in range(X.shape[1])]

# === GUARDAR MÁSCARA ===
with open(nombre_salida, "w") as f:
    f.write(" ".join(map(str, mask)))

# === ORDENAR TODAS LAS FEATURES POR CHI² (100%) ===
all_features = [(i, chi2_scores[i], feature_names[i]) for i in range(len(chi2_scores))]
all_features_sorted = sorted(all_features, key=lambda x: x[1], reverse=True)

# Extraer solo el número X de "cat_X"
feature_numbers = []
for _, _, name in all_features_sorted:
    match = re.search(r'cat_(\d+)', name)
    if match:
        feature_numbers.append(match.group(1))

# === GUARDAR ARCHIVO ORDENADO COMPLETO ===
with open(nombre_salida_indices, "w") as f:
    f.write(" ".join(feature_numbers))