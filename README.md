# Space War

### install sdl2
```
sudo apt-get install libsdl2-dev libsdl2-mixer-dev libsdl2-net-dev libsdl2-ttf-dev libsdl2-image-dev libsdl2-gfx-dev
```

### clone
```
git clone https://github.com/changethefuckingworld/spacewar
```

### build
```
cd spacewar
mkdir build
cd build
cmake ..
make
```

### run
```
./spacewar
```


### refs:

[event-handling]https://caveofprogramming.com/guest-posts/event-handling-sdl.html

[2D_collision_detection]https://developer.mozilla.org/en-US/docs/Games/Techniques/2D_collision_detection

[Simple-SDL2-Audio]https://github.com/jakebesworth/Simple-SDL2-Audio

### mp3 to wav

```
sudo apt install ffmpge
ffmpeg -i x.mp3 -acodec pcm_u8 -ar 22050 x.wav
```