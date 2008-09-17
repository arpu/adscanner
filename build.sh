rm adscanner
rm *.o
g++ -c src/*.cpp
g++ -o adscanner *.o  `pkg-config  --libs --cflags libswscale` `pkg-config  --libs --cflags libavcodec` `pkg-config  --libs --cflags libavformat`

