# Running and reproducing

## 1. Environment

Everything runs in a container built from `docker/Dockerfile` (Ubuntu 24.04, GCC 13.3, OpenMP, ImageMagick, ffmpeg).
The Dockerfile is unchanged: it already has every tool we need.

**Run every command in this file inside the container.**
Start a shell in it from the repository root:

```bash
./dockerrun.sh
```

The image is named **`598ape-hw1`**.
The first `./dockerrun.sh` builds it from `docker/Dockerfile`, which takes a few minutes; this is the same as running `docker build -t 598ape-hw1 docker` yourself.
Later runs reuse it.
Do not `docker pull wsmoses/598ape`: that image is arm64-only and does not run on x86-64.

## 2. Build and run the final version

```bash
make -j
./main.exe -i inputs/pianoroom.ray    --ppm -o output/pianoroom.ppm -W 1000 -H 1000
./main.exe -i inputs/globe.ray        --ppm -a inputs/globe.animate    -F 24 --movie -o output/globe.mp4
./main.exe -i inputs/elephant.ray     --ppm -a inputs/elephant.animate -F 24 --movie -W 100 -H 100 -o output/sphere.mp4
./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate -F 24 --movie -W 100 -H 100 -o output/elephant.mp4
```

`OMP_NUM_THREADS` sets the number of threads.

## 3. Commits

Each optimization is one commit, named after its report subsection:

| Name   | Commit    | Report section                                             |
| ------ | --------- | ---------------------------------------------------------- |
| `base` | `19bbc81` | Baseline (upstream code, unmodified)                       |
| `opt1` | `43be1d5` | 2.1 Apply Compilation Flag -O3                             |
| `opt2` | `ba08ccb` | 2.2 Find the Nearest Hit for Each Ray with Running Minimum |
| `opt3` | `d2b20ab` | 2.3 Make Vector Compute Header-Only and Inlined            |
| `opt4` | `3eb19ef` | 2.4 Avoid Using Linked Lists                               |
| `opt5` | `c035059` | 2.5 Add a Bounding Volume Hierarchy                        |
| `opt6` | `a50c6c7` | 2.6 Parallelize the Primary Ray Loop                       |
| `opt7` | `46022a0` | 2.7 Cache Each Plane's Basis Inverse                       |
| `opt8` | `fbf4ed4` | 2.8 Rebuild the Hierarchy Only When Shapes Move            |

## 4. Benchmarking each commit

| Script                     | What it does                                                |
| -------------------------- | ----------------------------------------------------------- |
| `bench/build.sh opt3`      | builds one commit into `bench/build/opt3/`                  |
| `bench/test.sh opt3 piano` | runs one test on it and prints the times                    |
| `bench/run.sh [names]`     | builds and tests the listed commits (default: all)          |
| `bench/check.sh`           | checks that every commit rendered the same images as `base` |

The four tests are `piano` (pianoroom, 1000x1000), `globe` (1000x1000), `sphere` (the 3,168-triangle mesh, 100x100), and `elephant` (the 111,748-triangle mesh, 100x100).
Set `REPS` (default 3), `THREADS` (default 1), and `FRAMES` (default 1) to change a run:

```bash
bench/run.sh                         # everything (about 15 minutes)
bench/check.sh
THREADS=32 bench/run.sh opt5 opt6    # one comparison
```

`base` and `opt1` time out on the elephant (after 120 s), because one frame takes more than a day.

The report's figures come from these commands, measured on a 2 x 32-core AMD EPYC 9334 node, so expect different absolute times:

| Report section | Command                                       |
| -------------- | --------------------------------------------- |
| 2.1 to 2.5     | `bench/run.sh base opt1 opt2 opt3 opt4 opt5`  |
| 2.6 and 2.7    | `THREADS=32 bench/run.sh opt5 opt6 opt7`      |
| 2.8            | `THREADS=32 FRAMES=24 bench/run.sh opt7 opt8` |

## 5. Known issue: the globe image

`bench/check.sh` reports the globe as different from `base` at some commits (opt4 to opt6 in this image).
This comes from the original code (`base`): `Shape` never initializes `mapX`, `mapY`, `mapOffX`, and `mapOffY`, and the globe's normal maps read them.
Their values are leftover memory, so the globe's image, and how long it takes, change whenever a commit changes the size of earlier allocations.
The other three tests render the same image at every commit.
