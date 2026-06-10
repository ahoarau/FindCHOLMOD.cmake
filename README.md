# FindCHOLMOD.cmake

A backward-compatible CMake find module for the
[CHOLMOD](https://people.engr.tamu.edu/davis/suitesparse.html) sparse
Cholesky factorization library from SuiteSparse.

The motivating problem: **Ubuntu 22.04's `libsuitesparse-dev` (SuiteSparse 5.x)
does not ship a `CHOLMODConfig.cmake`**, so `find_package(CHOLMOD CONFIG)`
fails. This module fills that gap while still preferring the upstream
config when it is available (SuiteSparse 7.x: Ubuntu 24.04, conda-forge,
Homebrew, etc.).

## Usage

Drop `FindCHOLMOD.cmake` next to your project and add its directory to
`CMAKE_MODULE_PATH`:

```cmake
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(CHOLMOD REQUIRED)

add_executable(my_app main.c)
target_link_libraries(my_app PRIVATE SuiteSparse::CHOLMOD)
```

Targets exposed (matching SuiteSparse 7.x's namespace):

| Target                         | Purpose                              |
| ------------------------------ | ------------------------------------ |
| `SuiteSparse::CHOLMOD`         | Main library (transitively links the rest) |
| `SuiteSparse::SuiteSparseConfig` | Common configuration symbols       |
| `SuiteSparse::AMD`             | Approximate Minimum Degree           |
| `SuiteSparse::CAMD`            | Constrained AMD                      |
| `SuiteSparse::COLAMD`          | Column AMD                           |
| `SuiteSparse::CCOLAMD`         | Constrained Column AMD               |

CHOLMOD pulls in BLAS and LAPACK automatically via CMake's
`find_package(BLAS)` / `find_package(LAPACK)`.

To bypass the upstream config and force the manual search, set
`-DCHOLMOD_NO_CONFIG=ON`.

## Building the example

### macOS / Linux (with [pixi](https://pixi.sh))

```bash
pixi run test
```

This fetches `cmake`, `ninja`, and `suitesparse` from conda-forge into a
local environment, configures the example, and runs the smoke test.

### Ubuntu (apt)

```bash
sudo apt-get install build-essential cmake ninja-build libsuitesparse-dev
cmake -S example -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

### Ubuntu via [Apple Containers](https://github.com/apple/container)

The pixi `ubuntu22` / `ubuntu24` environments delegate to
`scripts/test-ubuntu.sh`, which spins up the matching Ubuntu image and
runs the apt-based build inside it:

```bash
pixi run -e ubuntu22 test    # apt install libsuitesparse-dev + ctest
pixi run -e ubuntu24 test
pixi run -e ubuntu22 shell   # drop into an interactive Ubuntu shell at /work
```

The underlying script also accepts `docker` or `podman` if `container`
is unavailable; you can call it directly without pixi:

```bash
scripts/test-ubuntu.sh 22.04
```

## Verified targets

| Platform                | Source           | SuiteSparse | Path through FindCHOLMOD |
| ----------------------- | ---------------- | ----------- | ------------------------ |
| macOS / Linux / Windows | pixi (suitesparse7) | 7.10.1   | upstream `CHOLMODConfig` |
| macOS / Linux / Windows | pixi (suitesparse5) | 5.x      | **manual fallback**      |
| Ubuntu 24.04            | apt              | 7.6.1       | upstream `CHOLMODConfig` |
| Ubuntu 22.04            | apt              | 5.1.2       | **manual fallback**      |

All combinations above are exercised by [`.github/workflows/ci.yml`](.github/workflows/ci.yml).

## Layout

```
.
├── FindCHOLMOD.cmake        # the find module
├── pixi.toml                # conda-forge dev environment + tasks
├── example/
│   ├── CMakeLists.txt       # consumer using find_package(CHOLMOD REQUIRED)
│   └── main.c               # solves a 2x2 SPD system A x = b
└── scripts/
    └── test-ubuntu.sh       # apple-container/docker/podman driver
```
