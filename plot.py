import sys

file_name1 = sys.argv[1]

times = []

print file_name1

for i in range(52):
	times.append(i)

memory_consumed = []

with open(file_name1,'r') as file:
	for line in file:
		print line
		t = int(line)
		memory_consumed.append(t)

import matplotlib.pyplot as plt

plt.plot(times,memory_consumed, '-ro', label = "Throughput for RW-lock")
plt.xlabel("Threads")
plt.legend()
plt.show()
