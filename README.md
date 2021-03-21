## Dependencies

To compile the repository you need:

- C >=99 compiler
- CMake >=3.0
- GNU Make or Ninja

## Instructions

To clone and compile the repository you need to:

### GNU Make

```sh
git clone https://github.com/northy/router-simulation.git
cd router-simulation && mkdir build && cd build && cmake ..
make -j
```

### Ninja

```sh
git clone https://github.com/northy/router-simulation.git
cd router-simulation && mkdir build && cd build && cmake .. -G "Ninja"
ninja
```
