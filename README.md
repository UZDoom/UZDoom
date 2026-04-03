# ZMusic
UZDoom's music system as a standalone library

Welcome! This repository is a library for use with the projects [UZDoom](https://github.com/UZDoom/UZDoom).

Compile instructions are pretty simple for most systems.

```
git clone https://github.com/UZDoom/ZMusic.git
mkdir ZMusic/build
cd ZMusic/build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

On Unix/Linux you may also supply `sudo make install` in the build folder to push the compiled library directly into the file system so that it can be found by the previously mentioned projects.
