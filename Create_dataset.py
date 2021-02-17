import os
import random

file=open("big_dataset.txt","w")
file.write("20000000\n")
for i in range(1,2000000):
    x=random.randint(-1000,1000)
    y=random.randint(-1000,1000)
    z=random.randint(-1000,1000)
    file.write(str(x)+" "+str(y)+" "+str(z)+"\n")
    print(i)
file.close()
