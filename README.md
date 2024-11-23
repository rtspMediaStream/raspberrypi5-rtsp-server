# Try develop RTSP server on raspberryPi.
Use this repository to start developing an RTSP server on raspberrypi5.
Develop a server that streams audio and video.

# Manual

- Install
```bash
sudo apt update
sudo apt install libasound2-dev  // ALSA 
sudo apt install libopus-dev     // OPUS codec
sudo apt install ffmpeg		     // ffmpeg 
```
## ⚠️ Please download CMake version 3.29.2(supported by Kitware)

### for Build 
    mkdir build
    cmake ..
    make

###  Check your speaker device.
```bash
    aplay -l
```

# Start

### Start RTSP-server (Send)
```bash
./rtsp
```

### Check the sound by entering the speaker number in ffmpeg (Receive)
```bash
ffmpeg -i rtsp://localhost:554 -f alsa default
```
