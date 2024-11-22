ffmpeg -i rtsp://127.0.0.1:554 -f alsa default

# audio example.
# check your speaker device.
    aplay -l
# Check the sound by entering the speaker number in ffmpeg.
    ffmpeg -i rtsp://127.0.0.1:554 -ac 1 -f alsa plughw:1,0