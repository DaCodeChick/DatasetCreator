# DatasetCreator

A Qt 6 C++23 desktop application for creating AI datasets with support for multiple file formats and metadata management.

## Features

- **Multi-format Input Support**
  - Text files: .txt, .md, markdown, source code files
  - Images: PNG, JPEG, BMP, GIF, SVG, TIFF, WEBP (via Qt)
  - Audio: MP3, WAV, OGG, FLAC (via Qt Multimedia)
  - CSV/TSV files

- **Multiple Output Formats**
  - JSONL (JSON Lines) - LLM/NLP standard format
  - JSON - Human-readable format
  - CSV - Tabular data export
  - Parquet - Large-scale ML datasets (optional, requires Apache Arrow)
  - HDF5 - Scientific/numerical datasets (optional, requires HighFive)

- **Hierarchical Metadata System**
  - Dataset-level metadata
  - Subset groupings (characters, locations, concepts, etc.)
  - Sample-level metadata with tags, labels, and annotations

- **Extensible Plugin System**
  - Built-in plugins for common formats
  - Dynamic plugin loading for custom formats
  - Easy plugin API for adding new input/output formats

## Building

### Requirements

- Qt 6 (Core, Gui, Widgets, Multimedia, Svg)
- CMake 3.16+
- C++23 compatible compiler (GCC 11+, Clang 14+, MSVC 2022+)
- Optional: Apache Arrow (for Parquet support)
- Optional: HighFive (for HDF5 support)

### Build Instructions

```bash
mkdir build && cd build
cmake ..
cmake --build .
./DatasetCreator
```

### With Optional Libraries

```bash
# Install Apache Arrow for Parquet support
# Ubuntu/Debian: sudo apt install libarrow-dev libparquet-dev
# macOS: brew install apache-arrow

# Install HighFive for HDF5 support  
# Ubuntu/Debian: sudo apt install libhdf5-dev
# Then build HighFive from source or use vcpkg

mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6
cmake --build .
```

## Usage

1. Launch the application
2. Import files via File → Import Files
3. Add metadata and organize into subsets
4. Export dataset via File → Export Dataset

## Project Structure

```
DatasetCreator/
├── src/
│   ├── core/               # Core data structures
│   │   ├── Dataset.h/cpp
│   │   ├── DatasetSample.h/cpp
│   │   ├── Metadata.h/cpp
│   │   └── PluginInterface.h
│   ├── plugins/            # Plugin system
│   │   ├── PluginManager.h/cpp
│   │   ├── readers/        # Input format readers
│   │   └── writers/        # Output format writers
│   ├── managers/           # Business logic
│   ├── gui/                # Qt GUI components
│   └── utils/              # Utility functions
├── tests/                  # Unit tests
└── plugins/                # External plugin examples
```

## Creating Custom Plugins

See `docs/PLUGIN_DEVELOPMENT.md` for detailed instructions on creating custom file format plugins.

## License

[Your License Here]

## Contributing

Contributions are welcome! Please feel free to submit pull requests.
