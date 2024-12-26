# Try develop RTSP server on raspberryPi.
Use this repository to start developing an RTSP server on raspberrypi5.
Develop a server that streams audio and video.

# Manual 

### for Build 
```bash
clone https://github.com/rtspMediaStream/raspberrypi5-rtsp-server
cd raspberrypi5-rtsp-server
mkdir build
cd build
cmake ..
make
cp librtspserver.so /usr/local/lib/
```

# Example Start

### Audio RTSP-server
![제목 없는 다이어그램 drawio (2)](https://github.com/user-attachments/assets/a30fcb3f-e982-425b-9e5e-d2a5ad302182)
- Install
```bash
sudo apt update
sudo apt install build-essential pkg-config libopus-dev libasound2-dev  # For g++, make, library compile, Opus, ALSA
```
- Build (Server)
```bash
cd raspberrypi5-rtsp-server/example/AudioServer
mkdir build
cd build
cmake ..
make
./AudioServer
```
- Test (Client)
```bash
sudo apt install ffmpeg
ffmpeg -i rtsp://localhost:8554 -f alsa default
```


### Video RTSP-server
- Install
```bash
sudo apt update
```
- Build (Server)
```bash
cd raspberrypi5-rtsp-server/example/VideoServer
mkdir build
cd build
cmake ..
make
./VideoServer
```
- Test (Client)
```bash
vlc rtsp://127.0.0.1:8554
```

### Camera RTSP-server
![제목 없는 다이어그램 drawio (1)](https://github.com/user-attachments/assets/102342bb-1b1e-4624-9c19-4ff910f6aa72)

