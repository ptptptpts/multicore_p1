import random

size = input("3D Matrix size : ")
d1 = input("D1 : ")
d2 = input("D2 : ")
l1 = input("L1 : ")
l2 = input("L2 : ")
steps = input("steps : ")
threads = input("threads : ")

f = open ('input.life', 'w')

data = "%d %d %d %d %d %d %d\n" % (size, d1, d2, l1, l2, steps, threads)
f.write (data)

for i in range(0, size):
	for i in range(0, size):
		for i in range(0, size):
			f.write (random.choice(['0 ', '1 ']))
		f.write ("\n")

f.close()
