# Quick Start Guide

## Installation & Building

### Step 1: Install Qt 6

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-multimedia-dev qt6-svg-dev cmake build-essential
```

**macOS:**
```bash
brew install qt@6 cmake
```

**Windows:**
- Download Qt from https://www.qt.io/download
- Install Qt 6 with Desktop components
- Install CMake from https://cmake.org/download/

### Step 2: Clone and Build

```bash
cd /home/admin/Documents/GitHub/DatasetCreator

# Create build directory
mkdir build && cd build

# Configure (adjust Qt path if needed)
cmake ..

# Or specify Qt path explicitly:
# cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64

# Build
cmake --build .

# Run
./DatasetCreator
```

### Step 3: Verify Installation

The application should launch with a simple window showing:
- Menu bar with File menu
- Status text showing supported file extensions
- Import and Export options

## First Dataset Creation

### Example 1: Text Dataset

```bash
# Create some sample text files
mkdir test_data
echo "Hello, this is sample 1" > test_data/sample1.txt
echo "This is another sample" > test_data/sample2.txt
echo "# Markdown Sample" > test_data/sample3.md

# Run DatasetCreator
./DatasetCreator
```

In the GUI:
1. File → Import Files
2. Select test_data/sample1.txt, sample2.txt, sample3.txt
3. File → Export Dataset
4. Save as `output.jsonl`

Result (`output.jsonl`):
```jsonl
{"_meta":{"name":"My Dataset","created":"2024-01-15T10:30:00"}}
{"type":0,"data":"Hello, this is sample 1","metadata":{"id":"sample1.txt","source_file":"..."}}
{"type":0,"data":"This is another sample","metadata":{"id":"sample2.txt","source_file":"..."}}
{"type":0,"data":"# Markdown Sample","metadata":{"id":"sample3.md","source_file":"..."}}
```

### Example 2: Image Dataset

```bash
# Copy some images to a folder
mkdir image_dataset
cp /path/to/images/*.png image_dataset/

# Run and import
./DatasetCreator
```

1. File → Import Files
2. Select all PNG files
3. File → Export Dataset → Save as `images.json`

### Example 3: Mixed Dataset (Text + Images)

```bash
mkdir mixed_dataset
cp some_text.txt mixed_dataset/
cp some_image.png mixed_dataset/
```

Import all files together - the plugin system will automatically detect and handle each format.

## Troubleshooting

### Qt Not Found
```bash
# Find Qt installation
find /usr -name "Qt6Config.cmake" 2>/dev/null
# or
find /opt -name "Qt6Config.cmake" 2>/dev/null

# Then specify in cmake
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6
```

### Compilation Errors

**Missing Qt modules:**
```bash
# Ubuntu/Debian
sudo apt install qt6-multimedia-dev qt6-svg-dev

# macOS
brew reinstall qt@6
```

**C++23 not supported:**
- Update compiler: GCC 11+, Clang 14+, or MSVC 2022+
- Or edit CMakeLists.txt to use C++20: `set(CMAKE_CXX_STANDARD 20)`

### LSP Errors in Editor

LSP errors showing "QString not found" are expected before building. After running CMake:
```bash
cd build
cmake ..  # This generates MOC files
```

Then point your editor's C++ LSP to the build directory for include paths.

## Next Steps

1. **Enhance GUI**: Implement DatasetView to display imported samples
2. **Add Metadata**: Create interface to add tags and labels
3. **Test Audio**: Integrate Qt Multimedia for audio file support
4. **Add Parquet**: Install Apache Arrow and implement ParquetWriter

## Command Line Testing (for developers)

A test CLI program (`test_cli.cpp`) is included that validates import/export functionality:

```bash
cd build
cmake --build . --target test_cli
./test_cli
```

**Verified Test Results (2026-02-12):**
- Successfully imported 4 files from test_data/: sample1.txt, sample2.txt, sample3.md, sample.csv
- Successfully exported to all three formats:
  - `output.jsonl` (1.3KB) - JSONL format with metadata header + line-per-sample
  - `output.json` (2.1KB) - Single JSON object with samples array
  - `output.csv` (650B) - Tabular CSV format
- All plugins (TextReader, CSVReader, JSONLWriter, JSONWriter, CSVWriter) working correctly
- Metadata preserved including source_file, timestamps, file_extension, file_size

## Development Workflow

1. Make changes to source files
2. Rebuild: `cd build && cmake --build .`
3. Test: `./DatasetCreator`
4. Debug with Qt Creator or gdb/lldb

## Contact & Support

- GitHub Issues: https://github.com/YourUsername/DatasetCreator
- Documentation: See IMPLEMENTATION.md and README.md
