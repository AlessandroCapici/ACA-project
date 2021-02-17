from sklearn.cluster import KMeans
import pandas as pd

fileName = "../datasets/dataset_200000_4.txt"
df = pd.read_csv(fileName, sep=' ', header=None)
df.columns= ['x', 'y', 'z']
p = KMeans(n_clusters=3, max_iter=800)
cluster = p.fit(df.iloc[0:199]);
print(f"Centri dei cluster: \n {p.cluster_centers_}")
print(f"Numero iterazioni: {p.n_init}")
print(f"Algoritmo usato: {p.algorithm}")