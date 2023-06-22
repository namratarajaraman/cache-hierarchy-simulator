import os
import math
#graph 1
print("Making graph 1: ")
for i in range (11):
	size=2**(i+10)
	for j in range(4):
		assoc=2**(j)
		os.system("./sim 32 " + str(size) + " " + str(assoc) + " 0 0 0 0 " + "gcc_trace.txt > out_" + str(size) + "_" + str(assoc) + "_g1.txt")
	os.system("./sim 32 " + str(size)+" "+str(size/32)+" 0 0 0 0 " + "gcc_trace.txt > out_" + str(size) + "_" + "fully_assoc" + "_g1.txt")

#graph 1 CSV
g1data=[]
g3data=[]
g4data=[]
g5data=[]
g2data=[]


for i in range(11):
	size=2**(i+10)
	for j in range(4):
		assoc=2**(j)
		out_f1=open("out_"+str(size)+"_"+str(assoc)+"_g1.txt")
		for line in out_f1:
			if "L1 miss rate" in line:
				str_miss_rate=line.split(":")[-1]
				str_miss_rate=str_miss_rate.replace(" ", "")
				str_miss_rate=str_miss_rate.replace("\n", "")
				str_miss_rate=str_miss_rate.replace("\t", "")
				g1data.append([int(math.log2(size)), assoc, str_miss_rate])
		out_f1.close()
		out_fi=open("out_"+str(size)+"_"+"fully_assoc"+"_g1.txt")
		for line in out_fi:
			if "L1 miss rate" in line:
				str_miss_rate=line.split(":")[-1]
				str_miss_rate=str_miss_rate.replace(" ", "")
				str_miss_rate=str_miss_rate.replace("\n", "")
				str_miss_rate=str_miss_rate.replace("\t", "")
				g1data.append([int(math.log2(size)), assoc, str_miss_rate, "fully assoc"])
		out_fi.close()		



f1=open("graph1_data.csv", "w")
for entry in g1data:
	for item in entry:
		f1.write(str(item)+",")
	f1.write("\n")
f1.close()

#graph 2
for i in range(11):
	size=2**(i+10)
	for j in range(4):
		assoc=2**(j)
		out_f2=open("out_"+str(size)+"_"+str(assoc)+"_g1.txt")
		for line in out_f2:
			if "L1 reads" in line:
                                str_miss_rate=line.split(":")[-1]
                                str_miss_rate=str_miss_rate.replace(" ", "")
                                str_miss_rate=str_miss_rate.replace("\n", "")
                                str_miss_rate=str_miss_rate.replace("\t", "")
                                g2data.append([int(math.log2(size)), assoc, str_miss_rate, "read"])
			if "L1 writes" in line:
				str_miss_rate=line.split(":")[-1]
				str_miss_rate=str_miss_rate.replace("\n", "")
				str_miss_rate=str_miss_rate.replace("\t", "")
				g2data.append([int(math.log2(size)), assoc, str_miss_rate, "write"])
		out_f2.close()

		out_f2i=open("out_"+str(size)+"_"+"fully_assoc"+"_g1.txt")
		for line in out_f2i:
			if "L1 reads" in line:
				str_miss_rate=line.split(":")[-1]
				str_miss_rate=str_miss_rate.replace(" ", "")
				str_miss_rate=str_miss_rate.replace("\n", "")
				str_miss_rate=str_miss_rate.replace("\t", "")
				g2data.append([int(math.log2(size)), assoc, str_miss_rate, "read"])
			if "L1 writes" in line:
				str_miss_rate=line.split(":")[-1]
				str_miss_rate=str_miss_rate.replace(" ", "")
				str_miss_rate=str_miss_rate.replace("\n ", "")
				str_miss_rate=str_miss_rate.replace("\t ", "")
				g2data.append([int(math.log2(size)), assoc, str_miss_rate, "write"])						

f2=open("graph2_data.csv", "w")
for entry in g2data:
        for item in entry:
                f2.write(str(item)+",")
        f2.write("\n")
f2.close()



#graph 3
print("Making graph 3: ")
for i in range (4):
	size=2**(i+10)
	for j in range(4):
		assoc=2**(j)
		os.system("./sim 32 " + str(size) + " " + str(assoc) + " 16384 8 0 0 " + "gcc_trace.txt > out_" + str(size) + "_" + str(assoc) +"_g3" + ".txt")
	os.system("./sim 32 " + str(size)+" "+str(size/32)+" 0 0 0 0 " + "gcc_trace.txt > out_" + str(size) + "_" + "fully_assoc_g3" + ".txt")

for i in range (4):
	size=2**(i+10)
	for j in range(4):
		assoc=2**(j)
		out_f3=open("out_"+str(size)+"_"+str(assoc)+"_g3"+".txt")
		for line in out_f3:
			if "L2 writes" in line:
                                str_miss_rate=line.split(":")[-1]
                                str_miss_rate=str_miss_rate.replace(" ", "")
                                str_miss_rate=str_miss_rate.replace("\n", "")
                                str_miss_rate=str_miss_rate.replace("\t", "")
                                g3data.append([int(math.log2(size)), assoc, str_miss_rate, "write"])
			if "L2 reads" in line:
                                str_miss_rate=line.split(":")[-1]
                                str_miss_rate=str_miss_rate.replace(" ", "")
                                str_miss_rate=str_miss_rate.replace("\n", "")
                                str_miss_rate=str_miss_rate.replace("\t", "")
                                g3data.append([int(math.log2(size)), assoc, str_miss_rate, "read"])
		out_f3.close()



f3=open("graph3_data.csv", "w")
for entry in g3data:
        for item in entry:
                f3.write(str(item)+",")
        f3.write("\n")
f3.close()




#graph 4 
print("Making graph 4: ")
for i in range (6):
	size=2**(i+10)
	for j in range (4,8):
		blocksize=2**(j)
		os.system("./sim "+str(blocksize)+" " + str(size) + " " + "4" + " 0 0 0 0 " + "gcc_trace.txt > out_" + str(size) + "_" + str(blocksize) +"_g4" + ".txt")


for i in range(6):
	size=2**(i+10)
	for j in range (4, 8):
		blocksize=2**(j)
		out_f4=open("out_"+str(size)+"_"+str(blocksize)+"_g4"+".txt")
		for line in out_f4:
			if "L1 miss rate" in line:
                                str_miss_rate=line.split(":")[-1]
                                str_miss_rate=str_miss_rate.replace(" ", "")
                                str_miss_rate=str_miss_rate.replace("\n", "")
                                str_miss_rate=str_miss_rate.replace("\t", "")
                                g4data.append([int(math.log2(size)), blocksize, str_miss_rate])

	out_f4.close()

#graph 4 CSV

f4=open("graph4_data.csv", "w")
for entry in g4data:
        for item in entry:
                f4.write(str(item)+",")
        f4.write("\n")
f4.close()


#graph 5
print("Making graph 5: ")
for i in range (4):
	l1_size=2**(i+10)
	for j in range(4, 7):
		l2_size=2**(j+10)
		os.system("./sim 32 " + str(l1_size)+" "+"4 "+str(l2_size)+" 8 0 0 " + "gcc_trace.txt > out_" + str(l1_size) + "_" + "g5" + ".txt")

for i in range(4):
	l1_size=2**(i+10)
	for j in range(4,7):
		l2_size=2**(j+10)
		out_f5=open("out_"+str(l1_size)+"_"+"g5"+".txt")
		for line in out_f5:
			if "L2 writes" in line:
                                str_miss_rate=line.split(":")[-1]
                                str_miss_rate=str_miss_rate.replace(" ", "")
                                str_miss_rate=str_miss_rate.replace("\n", "")
                                str_miss_rate=str_miss_rate.replace("\t", "")
                                g5data.append([int(math.log2(l1_size)), l2_size, str_miss_rate, "write"])
			if "L2 reads" in line:
                                str_miss_rate=line.split(":")[-1]
                                str_miss_rate=str_miss_rate.replace(" ", "")
                                str_miss_rate=str_miss_rate.replace("\n", "")
                                str_miss_rate=str_miss_rate.replace("\t", "")
                                g5data.append([int(math.log2(l1_size)), l2_size, str_miss_rate, "read"])
	f5.close()



f5=open("graph5_data.csv", "w")
for entry in g5data:
        for item in entry:
                f5.write(str(item)+",")
        f5.write("\n")
f5.close()


