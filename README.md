# 🔐 Bitrecover - Multi-GPU Bitcoin Key Recovery Tool

**Professional-grade Bitcoin private key recovery tool for pre-2012 wallets with multi-GPU support, real-time monitoring, and email notifications.**

## ✨ Features

- 🚀 **Multi-GPU Support**: Automatically utilizes all available CUDA GPUs in parallel
- 📧 **Email Notifications**: Startup and match notifications via SMTP
- 📊 **Real-Time Status**: Live dashboard showing speed, progress, and GPU utilization
- 🎲 **Random 256-bit Keys**: Full keyspace search for pre-2012 Bitcoin wallets
- ⚡ **High Performance**: Optimized CUDA kernels for maximum throughput
- 🔄 **Resume Support**: Checkpoint/resume functionality
- 📁 **Flexible I/O**: Read from `address.txt`, write matches to `Success.txt`
## 🏗️ Architecture

```
bitrecover/
├── src/              # Core C++/CUDA source code
├── include/          # Header files
├── scripts/          # Python utilities and email scripts
├── config/           # Configuration files
└── CMakeLists.txt    # Build system
```

## 📋 Requirements

- **CUDA Toolkit** 11.0+ (for GPU acceleration)
- **NVIDIA GPU** with Compute Capability 7.0+
- **CMake** 3.18+
- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **Python 3.7+** (for email scripts)

## 🚀 Quick Start

### 1. Preparation

- **Edit `config/config.json`**
  - Enter your own email, GPU, and search settings in this file before running.
- **Edit `address.txt`**
  - Enter one Bitcoin address per line (your targets).

### 2. Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 3. Run

```bash
# Normal run
./bitrecover

# With custom config
./bitrecover --config config/config.json

# Command-line mode
./bitrecover --targets address.txt --output Success.txt --random256
```

## ⚙️ Configuration

All configuration is in `config/config.json`. Edit it directly, e.g.:

```json
{
    "email": { ... },
    "gpu": { ... },
    "search": { ... },
    "display": { ... }
}
```

## 📊 Performance

Expected speeds (approximate):
- **RTX 4090**: ~1.2-1.5 GKeys/s
- **RTX 3090**: ~800-1000 MKeys/s
- **RTX 3080**: ~600-800 MKeys/s
- **Multi-GPU**: Near-linear scaling (95%+ efficiency)

## 📝 License

MIT License - See LICENSE file for details

## ⚠️ Disclaimer

This tool is for educational purposes and recovery of your own wallets only. Use responsibly and ethically.

## 🤝 Contributing

Contributions welcome! Please open an issue or submit a pull request.

## 📧 Support

For issues and questions, please open a GitHub issue.

---

**Built with ❤️ for the Bitcoin community**

