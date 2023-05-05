if [ ! -d "build" ]; then
    mkdir build
fi
bspc rule -a '*' -o state=floating      # REMOVE THIS EVENTUALLY
g++ ./src/main.cpp -g -pedantic -o ./build/main -lSDL2
./build/main
