rm -rf build
mkdir build

cd build || exit

emcmake cmake ..
emmake make
