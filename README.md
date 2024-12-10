# Try develop RTSP server on raspberryPi.
Use this repository to start developing an RTSP server on raspberrypi5.
Develop a server that streams audio and video.

# Manual

- Install
```bash
sudo apt update
sudo apt install build-essential pkg-config libopus-dev libasound2-dev   # For g++, make, library compile, Opus, ALSA
sudo apt install ffmpeg		     # client test ffmpeg
```

### for Build 
```bash
make
```

###  Check your speaker device.
```bash
aplay -l
```

# Start

### Start RTSP-server (Send)
```bash
# if start audio streaming
sudo ./rtsp-server.out -m Audio
# if start video streaming
sudo ./rtsp-server.out -m Video
```

### Check the sound by entering the speaker number in ffmpeg (Receive)
```bash
ffmpeg -i rtsp://localhost:554 -f alsa default
```
### Check the video by vlc (Receive)
```bash
vlc rtsp://127.0.0.1:554
```
