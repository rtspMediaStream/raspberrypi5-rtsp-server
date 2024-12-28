# Stream video or audio on raspberryPi5.
You can create CCTV or call applications.

Use this repository to start developing an RTSP server on raspberrypi5.
![VideoServerExample](https://github.com/user-attachments/assets/5e84ad7b-d117-45f4-bb3a-c74ff88f51db)

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
<div align="center">
  <img src="https://github.com/user-attachments/assets/14d123ca-4a1a-4d39-8c45-53528003d27b" alt="AudioServerDevice" height="300" />
</div>

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
No installation required
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
<div align="center">
  <img src="https://github.com/user-attachments/assets/0a438fdb-1738-43e6-be2e-4464f8646a80" alt="CameraServerDevice" height="300" />
</div>

- Install
```bash
sudo apt update
sudo apt install libcamera-dev
sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev
```
- Build (Server)
```bash
cd raspberrypi5-rtsp-server/example/CameraServer
mkdir build
cd build
cmake ..
make
./CameraServer
```
- Test (Client)
```bash
vlc rtsp://127.0.0.1:8554
```

