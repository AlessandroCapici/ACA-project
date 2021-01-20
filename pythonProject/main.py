# This is a sample Python script.

# Press Maiusc+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.
from sklearn.cluster import KMeans
import pandas as pd

def print_hi(name):
    # Use a breakpoint in the code line below to debug your script.
    print(f'Hi, {name}')  # Press Ctrl+F8 to toggle the breakpoint.


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    fileName = "./dataset_10000_4.txt"
    df = pd.read_csv(fileName, sep=' ', header=None)
    df.columns= ['x', 'y', 'z']
    print(df)
    p = KMeans(n_clusters=3, max_iter=800)
    cluster = p.fit(df.iloc[0:199]);
    print(f"Centri dei cluster: \n {p.cluster_centers_}")
    print(f"Numero iterazioni: {p.n_init}")
    print(f"Algoritmo usato: {p.algorithm}")