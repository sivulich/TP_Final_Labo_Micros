import csv
import sounddevice as sd
import time
f = open('Out.out','r')
dat = csv.reader(f,delimiter=',')
wav = []
for x in dat:
	for i in x:
		if(i!=''):
			wav.append(int(i))
f.close()
m = max(wav)
wav = [x/m for x in wav]
sd.play(wav,44100)
time.sleep(60)