import struct
import wave
import progressbar

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
