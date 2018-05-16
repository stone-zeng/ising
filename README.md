# Ising Model: a New Perspective

[![Build Status](https://img.shields.io/travis/Stone-Zeng/ising.svg)](https://travis-ci.org/Stone-Zeng/ising)

Ising model, machine learning and AdS/CFT

## Structure

- `ising/`

    C++ code for generating 2D Ising model lattice data and simulating critical behaviors.

    Build with Visual Studio 2017 or g++ 7.3.0.

    - `core/`

        Core classes and functions for 2D Ising model.

    - `run/`

        Program entry point of `ising`.

    - `test/`

        Unit tests based on Microsoft C++ Unit Test Framework.

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
