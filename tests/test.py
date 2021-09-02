
from scipy.io import wavfile
from pathlib import Path
from pesq import pesq


from scipy.io import wavfile
from pesq import pesq

rate, deg = wavfile.read("./dgaudio/dgu_af1s01.wav")
rate, ref = wavfile.read("./audio/u_af1s01.wav")

print(pesq(8000, ref, deg, 'wb'))