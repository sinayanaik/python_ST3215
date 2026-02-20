# ST3215 C++ Library — Documentation Index

Welcome to the comprehensive documentation for the ST3215 C++ servo control library. This library is a faithful C++ port of the [Python ST3215 library](https://github.com/Mickael-Roger/python-st3215) with additional type safety, performance, and modern C++ features.

## Documentation Files

| Document | Description |
|----------|-------------|
| [Architecture](ARCHITECTURE.md) | System architecture, class hierarchy, layered design, and component relationships |
| [Engineering](ENGINEERING.md) | Build system, toolchain, compilation, testing, and CI/CD details |
| [Design](DESIGN.md) | Design patterns, decisions, trade-offs, and rationale |
| [Code](CODE.md) | Code structure, conventions, file layout, and contribution guide |
| [Math](MATH.md) | Motion math, kinematics, position encoding, and timing calculations |
| [Communication](COMMUNICATION.md) | STS serial protocol, packet format, register map, and hardware interface |
| [Syntax](SYNTAX.md) | Full API reference with method signatures, parameters, return types, and examples |
| [Installation](INSTALLATION.md) | Step-by-step build, install, integration, and troubleshooting instructions |

## Quick Links

- **New users**: Start with [Installation](INSTALLATION.md) then [Syntax](SYNTAX.md)
- **Contributors**: Read [Code](CODE.md) and [Engineering](ENGINEERING.md)
- **Hardware engineers**: See [Communication](COMMUNICATION.md) and [Math](MATH.md)
- **System architects**: Review [Architecture](ARCHITECTURE.md) and [Design](DESIGN.md)

## Library Overview

The ST3215 C++ library provides complete control of Feetech ST3215 serial bus servo motors. It supports:

- Servo discovery (ping, scan)
- Position control with configurable speed and acceleration
- Continuous rotation mode
- Telemetry reading (position, speed, voltage, current, temperature, load)
- Configuration (ID, baudrate, mode, calibration)
- Synchronized multi-servo operations (GroupSyncRead, GroupSyncWrite)
- EEPROM read/write protection

All functionality has been verified against the original Python library for behavioral equivalence.
