import os
import progressbar
import struct
import subprocess
import wave

with open("dump.txt", "r") as data:
    output = wave.open("output.wav", "wb")
    frames = data.read().split("\n")
    rows = int(len(frames)) - 1
    print(rows, " frames found")

    audio = [None] * rows
    output.setframerate(44100)
    output.setnframes(rows)
    output.setnchannels(1)
    output.setsampwidth(2)

    bar = progressbar.ProgressBar(maxval=rows, widgets=[progressbar.Bar('=', '[', ']'), ' ', progressbar.Percentage()])
    bar.start()
    for f in range(rows):
        bar.update(f + 1)
        audio[f] = int(float(frames[f]))
    bar.finish()

    output.writeframes(struct.pack('{}h'.format(len(audio)), *audio))
    output.close()

    os.chdir(os.getcwd())
    subprocess.call(['ffmpeg', '-hide_banner', '-loglevel', 'panic', '-i', 'input.mp4', '-i', 'output.wav', '-c:v', 'copy', '-map', '0:v:0', '-map', '1:a:0', 'mixed.mp4'])
