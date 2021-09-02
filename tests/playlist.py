
from scipy.io import wavfile
from pathlib import Path
import pyaudio
import concurrent.futures
import wave
import time
from csv import writer
from pesq import pesq
from datetime import datetime


#length of data to read.
chunk = 1024

# validation. If a wave file hasn't been specified, exit.
# working dir 
data_dir = Path(__file__).parent  
audio_files_list = data_dir / 'wb_playlist.txt'
audio_sources_path = data_dir / '../audio/'
adeg_path = data_dir / '../dgaudio/'
resultes_path = data_dir / '../resultes/'
end_of_wave_file = b'' # for playout as  been stoped  


# PLAYOUT Functions that play audio from the list 
def play(wavefile):
    global playoutended
    # open the file for reading.
    wf = wave.open(wavefile, 'rb')
    # create an audio object
    p = pyaudio.PyAudio()

    # open stream based on the wave object which has been input.
    stream = p.open(format =
                    p.get_format_from_width(wf.getsampwidth()),
                    channels = wf.getnchannels(),
                    rate = wf.getframerate(),
                    output = True)

    # read data (based on the chunk size)
    data = wf.readframes(chunk)

    # play stream (looping from beginning of file to the end)
    print("Playing:{}".format(wavefile))
    while data != end_of_wave_file:
        # writing to the stream is what *actually* plays the sound.
        stream.write(data)
        data = wf.readframes(chunk)

    # cleanup stuff.
    stream.close()    
    p.terminate()
    print("End Playout:{}".format(wavefile))
    playoutended = 1


# Recoread
def rec(dgwave):
    global playoutended
    playoutended = 0
    CHUNK = 1024
    FORMAT = pyaudio.paInt32
    CHANNELS = 1
    RATE = 8000
    RECORD_SECONDS = 1
    WAVE_OUTPUT_FILENAME = dgwave

    p = pyaudio.PyAudio()

    stream = p.open(format=FORMAT,
                    channels=CHANNELS,
                    rate=RATE,
                    input=True,
                    frames_per_buffer=CHUNK)

    print("Rec:{}".format(WAVE_OUTPUT_FILENAME))

    frames = []
    #for i in range(0, int(RATE / CHUNK * RECORD_SECONDS)):
    while playoutended == 0:
        data = stream.read(CHUNK)
        frames.append(data)

    print("End Rec:{}".format(WAVE_OUTPUT_FILENAME))

    stream.stop_stream()
    stream.close()
    p.terminate()

    wf = wave.open(WAVE_OUTPUT_FILENAME, 'wb')
    wf.setnchannels(CHANNELS)
    wf.setsampwidth(p.get_sample_size(FORMAT))
    wf.setframerate(RATE)
    wf.writeframes(b''.join(frames))
    wf.close()


#Open File 
print("Working Dirictory Environment: {}".format(str(data_dir)))
print("Read Test Audio List: {}".format(audio_files_list))
Lines = open(audio_files_list).read().splitlines()

print("Source Audio Info\n --------------------")
for l in Lines: 
    wf = wave.open(str(audio_sources_path)+"/"+l, 'rb')
    print("Audio File Name:{} Info:{}".format(l,wf.getparams()))  
print("Play Audio")
pslist=[]
for l in Lines:
    print('\n Test Audio:{}\n----------------------------'.format(l))
    playoutended = 0
    with concurrent.futures.ThreadPoolExecutor() as executor:
        orig = str(audio_sources_path)+"/"+l # Original Audio 
        ref_rec = str(adeg_path)+"/dg"+l # Degredeted File  
        f_rec = executor.submit(rec,ref_rec) # Start Recrding 
        time.sleep(0.1)
        f_play = executor.submit(play,orig) # Start Playout 
    print('PESQ')
    rate, ref = wavfile.read(orig)
    rate, deg = wavfile.read(ref_rec)
    try:
        pslist.append(pesq(8000, ref, deg, 'nb'))
    except:
        print('pesq failed{}'.format(l))
        pslist.append(0)
    print(pslist)
aps = sum(pslist)/len(pslist)
print('\n AVG PESQ:{}'.format(aps))
if aps < 1.6:
    faps = aps
else:  
    faps = float(aps) * (1+(1/float(aps))) # add factor to incrise
print('\n AVG FPESQ:{}'.format(faps))

# Append resoults to CSV 
writer = open(str(resultes_path)+'/fmycsvfile.csv','a')
writer.seek(0,2)
writer.writelines("\r")
writer.writelines( (',').join([str(datetime.now().strftime("%H:%M:%S")),str(faps)]))
