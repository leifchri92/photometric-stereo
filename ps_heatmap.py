import matplotlib.pyplot as plt
import numpy as np
import csv

hist_flag = sys.argv[1]

path_to_csv= "ps_comp" + sys.argv[2] + ".csv"

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

if hist_flag:
	cm = plt.cm.get_cmap('magma')
	n, bins, patches = plt.hist(data_flat, bins=50)
	bin_centers = 0.5 * (bins[:-1] + bins[1:])
	# scale values to interval [0,1]
	col = bin_centers - min(bin_centers)
	col /= max(col)
	for c, p in zip(col, patches):
	    plt.setp(p, 'facecolor', cm(c))
else:
	plt.imshow(data,cmap='magma',interpolation='nearest')
	plt.colorbar()

plt.show()