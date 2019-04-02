import matplotlib.pyplot as plt
import numpy as np
import csv

path_to_csv= "ps_comp.csv"

rows = []
with open(path_to_csv) as csv_file:
	csv_reader = csv.reader(csv_file, delimiter=',')
	for row in csv_reader:
		rows.append(row)

data = np.zeros((len(rows), len(rows[0])))
for i in range(0, len(rows)):
	data[i] = np.asarray(rows[i]).astype(float)

data_flat = data.flatten();
data_flat = np.delete(data_flat, np.nonzero(data_flat<0))

print("Mean: " + str(data_flat.mean()))
print("Std: " + str(data_flat.std()))

plt.imshow(data,cmap='magma',interpolation='nearest')
plt.colorbar()
plt.show()