import sys
import numpy as np
import pandas as pd
from sklearn.feature_selection import chi2
from sklearn.preprocessing import MinMaxScaler, KBinsDiscretizer
import re

# === LECTURA DE PARÁMETROS ===
input_path = sys.argv[1]         # dataset CSV
nombre_salida_indices = sys.argv[2]   # archivo .txt de salida

# === CARGA DE DATOS ===
df = pd.read_csv(input_path)
y = df.iloc[:, 0].values
X = df.iloc[:, 1:].values
feature_names = df.columns[1:]

# === CONDICIÓN: usar discretización solo si NO es "chess" ===
if "chess" in nombre_salida_indices.lower():
    X_proc = X
else:
    disc = KBinsDiscretizer(n_bins=5, encode="ordinal", strategy="uniform", subsample=None)
    X_proc = disc.fit_transform(X)

# === CHI² ===
chi2_scores, _ = chi2(X_proc, y)
chi2_scores = MinMaxScaler().fit_transform(chi2_scores.reshape(-1, 1)).flatten()

# === FILTRAR FEATURES CON CHI² >= 0.1 ===
selected_features = []
for i, score in enumerate(chi2_scores):
    if score >= 0.1:
        match = re.search(r'cat_(\d+)', feature_names[i])
        if match:
            selected_features.append(match.group(1))

# === GUARDAR ARCHIVO CON LOS ÍNDICES ===
with open(nombre_salida_indices, "w") as f:
    f.write(" ".join(selected_features))