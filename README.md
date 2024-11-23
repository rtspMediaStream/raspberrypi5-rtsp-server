# Manual

- Install
```bash
sudo apt update
sudo apt install libasound2-dev  // ALSA 
sudo apt install libopus-dev     // OPUS codec
sudo apt install ffmpeg		 // ffmpeg 
```
## Build

### for Build 
    make

###  Check your speaker device.
```bash
    aplay -l
```

# Start

### Start RTSP-server (Send)
```bash
./rtsp-opus-codec
```

### Check the sound by entering the speaker number in ffmpeg (Receive)
```bash
ffmpeg -i rtsp://localhost:554 -f alsa default
```
