# File Compressor & Evaluation Suite

A high-performance C++ implementation and benchmarking framework for lossless data compression algorithms: **Huffman Coding** (Canonical v2 with code-length run-length encoding), **LZ77 / LZSS** (sliding-window dictionary), and **DEFLATE** (hybrid LZSS + dual canonical Huffman trees).

---

## Features

- **Three Core Compression Modules**:
  - `DEFLATE`: LZSS sliding window (256-byte window) combined with dynamic dual Huffman trees (Literal/Length and Distance), plus code-length meta-compression.
  - `Huffman`: Canonical Huffman coding with code-length compression and frequency-scaling depth bounding ($\le 15$ bits).
  - `LZ77 / LZSS`: Sliding-window dictionary matching with bit-packed token streams and length headers.
- **Common In-Memory Interface (`ICompressor`)**:
  - Operates purely on `std::vector<uint8_t>` memory buffers, excluding disk I/O latency for accurate algorithmic throughput benchmarking.
- **High-Precision Timing Harness**:
  - Employs `std::chrono::high_resolution_clock`.
  - Performs 1 warm-up pass, followed by $N$ timed passes (adaptively selected based on warm-up duration).
  - Calculates mean, median, min/max, standard deviation, and throughput in MB/s.
- **Isolated Peak Memory Measurement**:
  - Leverages Windows `GetProcessMemoryInfo` (`psapi`) to retrieve `PeakWorkingSetSize`.
  - Spawns fresh, isolated child processes for each measurement to eliminate heap fragmentation and residual memory.
- **Automated Verification & Evaluation**:
  - Validates byte-for-byte round-trip equality (`decompressed == original`) before presenting results.
  - Produces clean, formatted CLI comparison tables with sizes, ratio, savings %, times, and peak memory.
- **Interactive CLI Menu**:
  - ANSI-colored interactive terminal interface with continuous loop and input validation.

---

## Directory Structure

```
File Compressor/
├── include/
│   ├── compressor_interface.hpp   # Common ICompressor interface and factory
│   ├── deflate.hpp                # DEFLATE compressor declarations
│   ├── huffman.hpp                # Huffman v1 declarations
│   ├── huffman2.hpp               # Canonical Huffman v2 declarations
│   ├── lzss.hpp                   # LZSS compressor declarations
│   ├── memory_tracker.hpp         # Windows psapi memory tracker & child process launcher
│   ├── priority_queue.hpp         # Min-heap priority queue for Huffman trees
│   ├── timing_harness.hpp         # High-resolution benchmark timing harness
│   └── utils.hpp                  # File I/O utilities
├── src/
│   ├── compressor_interface.cpp   # Concrete ICompressor adapters
│   ├── deflate.cpp                # DEFLATE implementation (in-memory & file)
│   ├── huffman.cpp                # Huffman v1 implementation
│   ├── huffman2.cpp               # Canonical Huffman v2 implementation
│   ├── lzss.cpp                   # LZSS implementation (in-memory & file)
│   ├── priority_queue.cpp         # Min-heap priority queue implementation
│   └── utils.cpp                  # readFile implementation
├── data/
│   └── input.txt                  # Sample test corpus (Shakespeare text, ~1.15 MB)
├── compile.bat                    # Build script for main executable
├── main.cpp                       # Application entry point (Menu, Single-Run, Benchmark)
├── run_timing_test.bat            # Test runner script for timing harness
└── test_timing.cpp                # Verification test suite for in-memory round-trips
```

---

## Building the Project

### Prerequisites
- Windows 10/11
- MinGW-w64 (GCC supporting C++17)

### Compile Command
Run the build script or use `g++` directly:

```bat
compile.bat
```

**Or manually via command line:**
```cmd
g++ -std=c++17 -O2 -Wall main.cpp src/utils.cpp src/priority_queue.cpp src/lzss.cpp src/huffman.cpp src/huffman2.cpp src/deflate.cpp src/compressor_interface.cpp -lpsapi -o deflate_app.exe
```

---

## Usage Modes

`deflate_app.exe` supports three distinct modes:

### 1. Interactive Menu (Default)
Run without arguments to launch the ANSI-colored menu:
```powershell
.\deflate_app.exe
```
Provides guided prompts to select algorithms, compression/decompression operations, and file paths in a continuous loop until `[0] Exit` is chosen.

### 2. Automated Benchmark Suite
Benchmark all three algorithms across a file in a single command:
```powershell
.\deflate_app.exe --benchmark data/input.txt
```
**Sample Output:**
```
============================================================================
           BENCHMARK EVALUATION SUITE
  File : data/input.txt (1155394 bytes)
============================================================================

Algorithm   Comp. Size     Ratio  Savings(%)  Comp. Time Decomp Time  Peak Mem (C)  Round-Trip
------------------------------------------------------------------------------------------------
Huffman         702781    0.6083      39.17%    55.07 ms   256.16 ms      19356 KB          OK
LZ77            948714    0.8211      17.89%   278.97 ms    10.78 ms      11700 KB          OK
DEFLATE         603380    0.5222      47.78%   352.92 ms   152.99 ms      13136 KB          OK
================================================================================================
```

### 3. Single-Run CLI Mode
Compress or decompress an individual file and view execution time, compression ratio, space savings, and peak memory:
```powershell
# Compress
.\deflate_app.exe compress deflate data/input.txt data/compressed.bin

# Decompress
.\deflate_app.exe decompress deflate data/compressed.bin data/restored.txt
```

---

## Evaluation Metric Definitions

- **Compression Ratio**:
  $$\text{Ratio} = \frac{\text{Compressed Size (bytes)}}{\text{Original Size (bytes)}}$$
  *(Lower is better; values $< 1.0$ indicate positive compression)*

- **Space Savings**:
  $$\text{Space Savings (\%)} = (1 - \text{Ratio}) \times 100$$
  *(Unclamped: negative values accurately reflect file expansion for incompressible data)*

- **Throughput**:
  $$\text{Throughput} = \frac{\text{File Size (MB)}}{\text{Execution Time (s)}}$$

- **Peak Working Set**:
  Measured via `GetProcessMemoryInfo` (`PeakWorkingSetSize`) inside an isolated child process to ensure pure, unfragmented memory recording.
