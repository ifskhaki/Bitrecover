# Bitrecover Project Structure

```
bitrecover/
├── .github/
│   └── workflows/
│       └── build.yml              # CI/CD pipeline
├── AddressUtil/                   # Bitcoin address encoding/decoding
│   ├── Base58.cpp                 # Base58 encoding
│   └── hash.cpp                   # Address hashing
├── config/
│   └── config.json                # User-editable configuration
├── CmdParse/                       # Command-line argument parsing
│   ├── CmdParse.cpp/h
├── CryptoUtil/                     # Cryptographic utilities
│   ├── sha256.cpp                 # SHA256 hashing
│   ├── ripemd160.cpp              # RIPEMD160 hashing
│   ├── checksum.cpp               # Checksum utilities
│   ├── Rng.cpp                    # Random number generation
│   └── hash.cpp                   # Hash utilities
├── CudaKeySearchDevice/            # CUDA GPU search implementation
│   ├── CudaKeySearchDevice.cpp/cu/h
│   ├── CudaDeviceKeys.cu          # Device key management
│   ├── CudaHashLookup.cu          # Hash lookup on GPU
│   ├── CudaAtomicList.cu          # Atomic operations
│   └── cudabridge.cu              # CUDA bridge functions
├── cudaMath/                       # CUDA math headers
│   ├── secp256k1.cuh              # Elliptic curve math
│   ├── sha256.cuh                 # SHA256 CUDA kernels
│   └── ripemd160.cuh              # RIPEMD160 CUDA kernels
├── cudaUtil/                       # CUDA utility functions
│   └── cudaUtil.cpp/h
├── include/
│   └── bitrecover/
│       ├── types.h                # Type definitions
│       └── constants.h            # Constants
├── KeyFinder/                      # Device management
│   ├── DeviceManager.cpp/h        # GPU device detection
│   └── ConfigFile.cpp/h           # Configuration file parsing
├── KeyFinderLib/                   # Core key finder library
│   ├── KeyFinder.cpp/h            # Main key finder logic
│   ├── KeySearchDevice.h          # Device interface
│   └── KeySearchTypes.h           # Type definitions
├── Logger/                         # Logging system
│   └── Logger.cpp/h
├── scripts/
│   ├── send_email.py              # Email notification script
│   └── startup_notify.py          # Startup notification script
├── secp256k1lib/                   # Elliptic curve cryptography
│   └── secp256k1.cpp/h             # secp256k1 implementation
├── src/
│   ├── main.cpp                   # Main entry point
│   ├── BitrecoverEngine.cpp/h     # Core engine
│   ├── MultiGPUManager.cpp/h      # Multi-GPU coordination
│   ├── EmailNotifier.cpp/h        # Email notifications
│   ├── StatusDisplay.cpp/h        # Real-time status display
│   ├── ConfigManager.cpp/h        # Configuration management
│   └── RandomKeyGenerator.cpp/h   # Random key generation
├── util/                           # Utility functions
│   └── util.cpp/h
├── .gitignore                     # Git ignore rules
├── CMakeLists.txt                 # CMake build configuration
├── Makefile                       # Convenience Makefile
├── LICENSE                        # MIT License
├── README.md                      # Main documentation
├── QUICKSTART.md                  # Quick start guide
├── CONTRIBUTING.md                # Contribution guidelines
├── PROJECT_STRUCTURE.md           # This file
├── run_bitrecover.bat             # Windows one-click run script
└── address.txt                    # Target addresses (user-editable)
```

## Key Components

### Core Engine (`src/BitrecoverEngine.*`)
- Main orchestrator for the entire application
- Manages initialization, configuration, and execution flow
- Coordinates between GPU manager, email notifier, and status display

### Multi-GPU Manager (`src/MultiGPUManager.*`)
- Handles parallel execution across multiple GPUs
- Manages worker threads, one per GPU
- Aggregates results and statistics
- Provides real-time status updates

### Email Notifier (`src/EmailNotifier.*`)
- Sends startup notifications
- Sends match notifications when keys are found
- Integrates with Python email scripts

### Status Display (`src/StatusDisplay.*`)
- Real-time terminal-based dashboard
- Shows per-GPU statistics
- Displays matches prominently
- Cross-platform (Windows/Linux)

### Configuration Manager (`src/ConfigManager.*`)
- Loads configuration from `config/config.json`
- Provides defaults
- System information gathering

### Random Key Generator (`src/RandomKeyGenerator.*`)
- Generates random 256-bit keys for pre-2012 wallets
- Ensures unique starting points per GPU
- Validates key ranges

## Legacy Components (Integrated from BitCrack)

These components provide the core cryptographic and GPU acceleration functionality:

- **AddressUtil/**: Bitcoin address encoding/decoding (Base58, hashing)
- **CryptoUtil/**: Cryptographic primitives (SHA256, RIPEMD160, RNG)
- **CudaKeySearchDevice/**: CUDA GPU search kernels and device management
- **cudaMath/**: CUDA math headers for elliptic curve operations
- **cudaUtil/**: CUDA utility functions
- **KeyFinderLib/**: Core key search algorithms and device interfaces
- **KeyFinder/**: Device detection and configuration management
- **secp256k1lib/**: Elliptic curve cryptography implementation
- **Logger/**: Logging infrastructure
- **CmdParse/**: Command-line argument parsing
- **util/**: General utility functions

## Build System

### CMake
- Modern CMake 3.18+ configuration
- Auto-detects GPU architectures
- Supports multiple CUDA compute capabilities
- Integrates all legacy components

### Makefile
- Convenience wrapper around CMake
- Quick build commands
- Common development tasks

## Scripts

### Batch Scripts (Windows)
- `run_bitrecover.bat`: **One-click Windows launcher**
  - Checks if build exists
  - Validates required files (address.txt, config.json)
  - Creates sample files if missing
  - Runs bitrecover with proper config
  - Displays Success.txt if matches found
  - Beautiful terminal UI with status messages

### Python Scripts
- `send_email.py`: Handles email sending (startup and matches)
- `startup_notify.py`: Startup notification wrapper

## Configuration

- `config/config.json` (edit directly to personalize)
- `address.txt` (edit directly with your Bitcoin target addresses)

## Dependencies

### Required
- CUDA Toolkit 11.0+
- CMake 3.18+
- C++17 compiler
- Python 3.7+ (for scripts)

### Optional
- nlohmann/json (for full JSON config support)
- Email server access (for notifications)

## Build Output

```
build/
├── bin/
│   └── bitrecover          # Main executable
└── lib/                    # Libraries (if any)
```

## Usage Flow

1. **Initialization**
   - Edit `config/config.json` and `address.txt` as needed
   - Load configuration
   - Detect GPUs
   - Initialize GPU workers
   - Send startup email

2. **Execution**
   - Start parallel GPU threads
   - Update status display
   - Aggregate results
   - Handle matches

3. **Shutdown**
   - Stop all workers
   - Save final statistics
   - Cleanup resources

## Future Enhancements

- [ ] Full JSON config parser (nlohmann/json integration)
- [ ] Web dashboard for remote monitoring
- [ ] Database backend for match storage
- [ ] Distributed computing support
- [ ] Advanced GPU optimization profiles
- [ ] Checkpoint/resume per GPU
- [ ] Performance analytics and reporting

