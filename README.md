# Ising Model: a New Perspective

[![Build Status](https://img.shields.io/travis/Stone-Zeng/ising.svg)](https://travis-ci.org/Stone-Zeng/ising)

Ising model, machine learning and AdS/CFT

## Structure

- `ising/`

    C++ code for generating 2D Ising model lattice data and simulating critical behaviors.

    Build with Visual Studio 2017.

    - `ising-core/`

        Core classes and functions for 2D Ising model.

    - `ising-lattice-data/`

        Lattice data generating and convergence analysis.

    - `ising-run/`

        Analysis critical behaviors.

    - `ising-test/`

        Unit tests based on Microsoft C++ Unit Test Framework.

- `ising-exact/`

    C++ / Wolfram Language code for exact solution of Ising Model (finite size).

    Build with Visual Studio 2017 (C++).

- `rbm/`

    RBM (Restrict Boltzmann Machine) network.

- `tensorflow/`

    TensorFlow playground.

## Dependencies

- [vietjtnguyen/argagg](https://github.com/vietjtnguyen/argagg)
- [Tencent/rapidjson](https://github.com/Tencent/rapidjson)
- [TensorFlow](https://github.com/tensorFlow/tensorFlow)

## License

Copyright (C) 2018 by Xiangdong Zeng.

Licensed under the [MIT](LICENSE) License.
