# Plateau VST3

A lush stereo plate reverb audio plugin in VST3 format, built with JUCE.

## Features

- Dattorro 1997 plate reverb algorithm — rich, dense, natural-sounding plate decay
- **Tuned Mode** — adjust crossover and damping per frequency band
- **Diffuse Input** — blends diffused signal into the reverb tail
- **Freeze** — sustain the reverb infinitely
- **Clear** — reset the reverb tail instantly
- Dark/light panel themes switchable from the DAW

### Controls

| Parameter      | Range       | Description                               |
|----------------|-------------|-------------------------------------------|
| Pre-Delay      | 0–200 ms    | Delay before reverb onset                 |
| Decay          | 0–100%      | Reverb decay time                         |
| Low-Mid Xover  | 20–2000 Hz  | Crossover frequency between low/mid bands |
| Low Decay      | 0–100%      | Multiplier for low-frequency decay        |
| Mid-Hi Xover   | 200–20000 Hz| Crossover frequency between mid/hi bands  |
| Hi Decay       | 0–100%      | Multiplier for high-frequency decay       |
| Damping        | 0–100%      | High-frequency absorption in the tank     |
| Modulation     | 0–100%      | Depth of internal modulation for movement |
| Mix            | 0–100%      | Wet/dry mix                               |
| Output         | -24–+24 dB  | Output gain                               |

## Installation

Copy the `Plateau.vst3` folder to your system's VST3 directory:

- **Linux:** `~/.vst3/` or `/usr/lib/vst3/`
- **Windows:** `C:\Program Files\Common Files\VST3\`

## Building from Source

### Prerequisites

- CMake 3.20+
- JUCE 8.x
- C++17 compiler (GCC, Clang, or MSVC)
- Python 3 (for SVG binary data generation)

### Linux

```bash
git clone https://github.com/BinDumps/PlateauVST3.git
cd PlateauVST3
pip3 install -r /dev/null
cmake -B build -DCMAKE_BUILD_TYPE=Release .
cmake --build build --config Release -j$(nproc)
```

### Windows (cross-compile from Linux)

```bash
sudo apt install mingw-w64
cmake -B build-win \
    -DCMAKE_TOOLCHAIN_FILE=cmake/x86_64-w64-mingw32-cross.cmake \
    -DCMAKE_BUILD_TYPE=Release .
cmake --build build-win --config Release -j$(nproc)
```

## License

This project is provided for educational and personal use.
