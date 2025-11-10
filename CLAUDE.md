# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Dream DRM is a standalone Digital Radio Mondiale (DRM) decoder with xHE-AAC audio support. It's a sophisticated real-time signal processing application written in C++/Qt that handles DRM broadcast reception, decoding, and audio output. The codebase originated from a 2004 academic project and has evolved significantly with modern codec support.

**Build instructions and dependencies are documented in [README.md](README.md).**

## Core Architecture

### Signal Processing Pipeline
The DRM receiver follows a modular signal processing pipeline architecture:

```
Input Signal → Synchronization → OFDM Demodulation → Channel Estimation →
MLC Decoding → DRM Protocol Layers → Audio/Data Decoding → Output
```

### Key Architectural Components

**Main Entry Points:**
- `src/main.cpp` - Console mode entry point (headless operation)
- `src/main-Qt/main.cpp` - Qt GUI mode entry point

**Core Signal Processing Modules:**
- `src/sync/` - Time and frequency synchronization using pilot tones
- `src/OFDM.cpp/h` - OFDM demodulation and FFT processing
- `src/chanest/` - Channel estimation with multiple algorithms (TimeLinear, TimeWiener)
- `src/mlc/` - Multi-level coding (Viterbi decoder, bit de-interleaving, QAM demapping)
- `src/ofdmcellmapping/` - OFDM cell mapping and carrier management

**DRM Protocol Layers:**
- `src/FAC/` - Fast Access Channel (control information)
- `src/SDC/` - Service Description Channel (service metadata)
- `src/MSC/` - Main Service Channel (audio/data content)

**Audio Decoding:**
- `src/sourcedecoders/fdk_aac_codec.cpp/h` - xHE-AAC/USAC decoding (requires FDK-AAC v2.0.2+)
- `src/sourcedecoders/aac_codec.cpp/h` - Standard AAC decoding
- `src/sourcedecoders/opus_codec.cpp/h` - Opus audio codec support

**Data Services:**
- `src/datadecoding/Journaline.cpp/h` - News service decoding
- `src/datadecoding/MOTSlideShow.cpp/h` - Multimedia Object Transfer for slides
- `src/datadecoding/DABMOT.cpp/h` - DAB-compatible MOT implementation

### Mathematical Foundation
- `src/matlib/` - Core mathematical library (FFT operations, matrix algebra, signal processing toolbox)
- Uses FFTW3 for efficient FFT operations
- Custom IIR filter implementations with configurable time constants

### Input/Output Architecture
- `src/DataIO.cpp/h` - Unified data I/O management
- `src/creceivedata.cpp/h` - Input data reception and format conversion
- `src/ctransmitdata.cpp/h` - Output data transmission
- Supports multiple input sources: files, pipes, sound cards, SDR interfaces
- Real-time status broadcasting via Unix domain socket

## Critical Implementation Details

### Real-time Processing Constraints
- All signal processing uses fixed block sizes for deterministic timing
- IIR filters with configurable time constants (critical for numerical stability)
- Memory management optimized for low-latency processing
- Threaded architecture separates UI, processing, and I/O

### xHE-AAC Support (Critical Feature)
- Requires FDK-AAC v2.0.2+ with USAC (Unified Speech and Audio Coding) support
- Frame boundary calculation fixes in `xheaacsuperframe.cpp`
- Buffer overflow protection in `AudioSourceDecoder.cpp`
- Division by zero crash prevention in audio decoder initialization

### Modem Design Patterns
- Each processing module implements `CModul` base class with `Process()` method
- Consistent parameter management via `CParameter` singleton
- Real-time status updates via Unix socket interface
- Exception-based error handling throughout the processing chain

### Platform-Specific Implementations
- `src/linux/` - Linux-specific (PulseAudio, console I/O, ALSA)
- `src/windows/` - Windows audio and platform utilities
- `src/android/` - Android platform support
- `src/macx/` - macOS-specific implementations

## Configuration and Settings

### Build Configuration Options
- `CONFIG+=console` - Headless console mode (no Qt GUI dependency)
- `CONFIG+=qtconsole` - Qt-based console mode
- `CONFIG+=fdk-aac` - Enable xHE-AAC support
- `CONFIG+=debug` - Debug build with additional logging

### Runtime Configuration
- Settings stored in INI format files
- Command-line parameter parsing for all receiver parameters (see `./dream --help`)
- Service and frequency management with presets
- Real-time parameter adjustment via socket interface
- Real-time status monitoring: `socat UNIX-CONNECT:/tmp/dream_status.sock -`

### Key Parameters for DRM Reception
- Frequency tuning and bandwidth selection
- Robustness mode (A, B, C, D) and spectrum occupancy
- Audio codec selection and quality settings
- SDR input configuration and gain control

## Development Workflow

### Code Organization Principles
- Signal processing modules are self-contained with clear interfaces
- Mathematical operations centralized in `src/matlib/`
- Platform-specific code isolated in separate directories
- Consistent error handling and logging throughout

### Testing Approach
- Integration tests using real DRM broadcast recordings in `test_data/`
- Real-time monitoring via socket interface (`socat UNIX-CONNECT:/tmp/dream_status.sock -`)
- Limited unit test coverage in `DreamTests/`
- Socket cleanup utility: `test_socket_cleanup.sh`

### Quality Assurance Notes
- Mixed codebase: original 2004 code with gradual modernization
- Memory management: combination of new/delete and Qt object model
- Threading: Separate threads for UI, signal processing, and I/O
- Performance optimized for real-time constraints but could benefit from modern C++ features

## Known Issues and Technical Debt

### Signal Processing Stability
- IIR filter numerical stability issues under strong signal conditions
- **Critical Issue**: DC filter state accumulation causing "沙沙声" (static noise) after strong signals - see `AM_AGC.cpp` and `AMDemodulation.cpp`
- Time constants in IIR filters (e.g., `DC_IIR_FILTER_LAMBDA = 0.999`) may need adjustment for different signal conditions
- Consider filter state reset mechanisms for strong signal recovery

### Code Modernization Opportunities
- Mixed Qt4/Qt5/Qt6 compatibility layers
- Could benefit from C++11/14/17 features for better memory management
- Limited automated testing infrastructure
- Some legacy code patterns from original 2004 implementation

### Performance Considerations
- Real-time constraints require careful optimization
- CPU-intensive operations in FFT and Viterbi decoding
- Memory efficiency critical for embedded deployments
- Limited parallelization in signal processing pipeline

## Key Files for Understanding the System

**Core Architecture:**
- `src/DrmReceiver.cpp/h` - Main receiver implementation
- `src/Parameter.cpp/h` - Global parameter management
- `src/GlobalDefinitions.h` - System-wide constants and definitions

**Signal Processing Pipeline:**
- `src/OFDM.cpp/h` - OFDM demodulation implementation
- `src/sync/TimeSync.cpp/h` - Time domain synchronization
- `src/chanest/ChannelEstimation.cpp/h` - Channel estimation algorithms

**Audio Processing:**
- `src/sourcedecoders/fdk_aac_codec.cpp/h` - xHE-AAC decoder integration
- `src/sourcedecoders/AudioSourceDecoder.cpp/h` - Audio source management

**Platform Integration:**
- `src/linux/ConsoleIO.cpp/h` - Linux console interface
- `src/util/StatusBroadcast.cpp/h` - Real-time status broadcasting