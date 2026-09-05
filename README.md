# Kith

<p align="center">
  <img src="assets/kith_128.png" width="128" alt="Kith Logo">
</p>

A fully customizable desktop calendar widget for Linux. Written in C++ with Qt Widgets.

## Installation

### Dependencies
- `build-essential`
- `cmake`
- `ninja-build`
- `qt6-base-dev`
- `qt6-base-dev-tools`

### Build from Source

```bash
git clone https://github.com/JacobEscoto.kith.git
cd kith

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**NOTE:** If you don't have installed `ninja`, remove `-G Ninja`.

If you want to install it at `/usr/local/bin` use:

```bash
sudo cmake --install build
```

### Run Kith

```bash
# Run it manually using the build generated binary
./build/kith

# If already installed run:
kith &
```

## Customization

Kith is fully customizable using QSS. Edit `~/.config/kith/style.qss` and launch again to see the changes.

### Available selectors:

| Selector                      | Description                                   |
|-------------------------------|-----------------------------------------------|
| `#container`                  | General panel background, and rounded borders |
| `#monthYearLabel`             | "Year Month" top label                        |
| `#divider`                    | Divisory line between panels                  |
| `#bottomPanel`                | Day cells container                           |
| `QLabel[header="true"]`       | Headers of Mo, Tu, We, Th, Fr, Sa, Su         |
| `QLabel#dayCell`              | Date cells (1-28/29/30/31)                    |
| `QLabel#dayCell[today="true"]`| Current day cell (accent color)               |

## License

[MIT](https://choosealicense.com/licenses/mit)
