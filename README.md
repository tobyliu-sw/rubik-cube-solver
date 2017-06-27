## Rubik's Cube Solver

[rubik-cube-solver](https://github.com/tobyliu-sw/rubik-cube-solver) is a C++ open source project to solve Rubik's cube.

### System environment

1. Ubuntu 16.04 as VMWare guest OS
2. built-in gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.4)
3. cmake version 3.5.1 
```
sudo apt-get install cmake
```

### Features

1. RubikCube class supports 3x3x3, 4x4x4, and 5x5x5 cubes.
2. RubikCubeSolver is the base class for different solvers, for example, RubikCube3BasicSolver.
3. Currently, only basic solver is implemented, which solves 3x3x3 Rubik's cube by layers.


### Build:
```
./build.sh
```

### Run:
```
./build/rubik-cube-solver
```

### Reference:

1. RubikCube3BasicSolver refers to the layer-by-layer basic solution websites below.
    - [https://ruwix.com/the-rubiks-cube/how-to-solve-the-rubiks-cube-beginners-method/](https://ruwix.com/the-rubiks-cube/how-to-solve-the-rubiks-cube-beginners-method/)
    - [http://www.ryanheise.com/cube/beginner.html](http://www.ryanheise.com/cube/beginner.html)
