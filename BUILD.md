# Build all firmware for the lard61

```sh
export PICO_SDK_PATH=/path/to/sdk
mkdir build
cmake ..
make
```

Alternatively, `cmake -DCMAKE_BUILD_TYPE=Debug ..` to build
with debug symbols.

The projects can also be built for a Pico instead, by adding
`-DPICO_BOARD=pico`.

