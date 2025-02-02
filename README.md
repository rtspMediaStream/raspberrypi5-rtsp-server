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
sudo cp librtspserver.so /usr/local/lib/
```

# Example Start

### Audio RTSP-server
<div align="center">
  <img src="https://github.com/user-attachments/assets/b37cf3d4-46d5-4b3b-8dc9-ebb8e5839ca6" alt="AudioServerDevice" height="300" />
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
ffmpeg -i rtsp://127.0.0.1:8554 -f alsa default
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
  <img src="https://github.com/user-attachments/assets/6169338b-6dc5-427f-9e10-4ac6841d92ba" alt="CameraServerDevice" height="300" />
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

