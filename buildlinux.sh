mkdir -pv ~/uzdoom_build
cd ~/uzdoom_build &&
git clone https://github.com/UZDoom/uzdoom.git &&
mkdir -pv uzdoom/build
a='' && [ "$(uname -m)" = x86_64 ] && a=64
c="$(lscpu -p | grep -v '#' | sort -u -t , -k 2,4 | wc -l)" ; [ "$c" -eq 0 ] && c=1
cd ~/uzdoom_build/uzdoom/build &&
rm -f output_sdl/liboutput_sdl.so &&
if [ -d ../fmodapi44464linux ]; then
f="-DFMOD_LIBRARY=../fmodapi44464linux/api/lib/libfmodex${a}-4.44.64.so \
-DFMOD_INCLUDE_DIR=../fmodapi44464linux/api/inc"; else
f='-UFMOD_LIBRARY -UFMOD_INCLUDE_DIR'; fi &&
cmake .. -DCMAKE_BUILD_TYPE=Release $f &&
make -j$c
