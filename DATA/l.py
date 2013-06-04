#!/usr/bin/python

data=open("hans.eeprom").read()

l=map(int,data.split(','))

for k in zip(l[0::3],l[1::3],l[2::3]):
	print k[0]*256+k[1],("00000000"+str(bin(k[2]))[2:])[-8:]
