#!/usr/bin/python

import sys

m=[]

for datei in sys.argv[1:]:
	data=open(datei).read()
	l=map(int,data.split(','))
	m+=zip(l[0::3],l[1::3],l[2::3])

res=[]
for k in m:
	res.append((k[0]*256+k[1],("00000000"+str(bin(k[2]))[2:])[-8:]))

res.sort()

print "\n".join(map(lambda x:str(x[0])+" "+str(x[1]),res))
