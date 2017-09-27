# Prerequisites

Before building polar-pong, you must download the following prerequisites.

* GLM
* SDL2
* SDL2\_ttf

# Building

From the root of the repository, run the following commands.

```sh
mkdir build && cd build
cmake .. \
  -DGLM_ROOT_DIR=/path/to/glm
  -DSDL2_ROOT_DIR=/path/to/sdl2
  -DSDL2TTF_ROOT_DIR=/path/to/sdl2_ttf
make
```

## macOS

If you have installed the frameworks for SDL2 and SDL2\_ttf, they should be
discovered automatically, and you should only need to set `GLM_ROOT_DIR`.
