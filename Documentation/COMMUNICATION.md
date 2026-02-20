# Communication

This document describes the STS serial communication protocol, packet format, register map, hardware interface, and electrical connections used by the ST3215 servo.

## Hardware Interface

### Physical Connection

ST3215 servos use a **half-duplex single-wire serial bus**. Multiple servos are daisy-chained on the same data line.

```
┌──────────┐     ┌──────────┐     ┌──────────┐
│  Host PC │     │ Servo #1 │     │ Servo #2 │
│          │     │ (ID: 1)  │     │ (ID: 2)  │
│  USB     ├─────┤ Data In  ├─────┤ Data In  │
│  Serial  │     │ Data Out ├─────┤ Data Out │
│  Adapter │     │          │     │          │
└──────────┘     └──────────┘     └──────────┘
                      │                │
                     VCC              VCC
                      │                │
                   Power Supply (6-12V)
```

### Wiring

| Pin | Signal | Description |
|-----|--------|-------------|
| 1 | GND | Ground (common for all servos) |
| 2 | VCC | Power supply (6-12V, typically 7.4V) |
| 3 | DATA | Half-duplex serial data line |

### USB-Serial Adapter

The host computer connects via a USB-to-serial adapter. Common adapters:

| Adapter | Chip | Max Baudrate | Linux Device |
|---------|------|-------------|--------------|
| Feetech Debug Board | CH340 | 1 Mbps | `/dev/ttyUSB0` |
| Generic USB-TTL | CP2102 | 1 Mbps | `/dev/ttyUSB0` |
| FTDI | FT232R | 3 Mbps | `/dev/ttyUSB0` |

### Serial Configuration

```
Baudrate:   1,000,000 bps (default)
Data bits:  8
Parity:     None
Stop bits:  1
Flow ctrl:  None
Mode:       Half-duplex (8N1)
```

## Supported Baudrates

| Code | Baudrate | Constant |
|------|----------|----------|
| 0 | 1,000,000 bps | `STS_1M` |
| 1 | 500,000 bps | `STS_0_5M` |
| 2 | 250,000 bps | `STS_250K` |
| 3 | 128,000 bps | `STS_128K` |
| 4 | 115,200 bps | `STS_115200` |
| 5 | 76,800 bps | `STS_76800` |
| 6 | 57,600 bps | `STS_57600` |
| 7 | 38,400 bps | `STS_38400` |

## STS Protocol

### Protocol Overview

The STS protocol is a master-slave protocol where the host (master) sends instruction packets and the servo (slave) responds with status packets.

```
Host  ──[Instruction Packet]──►  Servo
Host  ◄──[Status Packet]──────  Servo
```

### Packet Format

#### Instruction Packet (Host → Servo)

```
┌────────┬────────┬─────┬────────┬─────────────┬───────────┬──────────┐
│Header0 │Header1 │ ID  │Length  │Instruction  │Parameters │ Checksum │
│ 0xFF   │ 0xFF   │     │       │             │           │          │
├────────┼────────┼─────┼────────┼─────────────┼───────────┼──────────┤
│ 1 byte │ 1 byte │1 b  │ 1 byte│   1 byte    │ N bytes   │  1 byte  │
└────────┴────────┴─────┴────────┴─────────────┴───────────┴──────────┘
```

#### Status Packet (Servo → Host)

```
┌────────┬────────┬─────┬────────┬───────┬───────────┬──────────┐
│Header0 │Header1 │ ID  │Length  │Error  │Parameters │ Checksum │
│ 0xFF   │ 0xFF   │     │       │       │           │          │
├────────┼────────┼─────┼────────┼───────┼───────────┼──────────┤
│ 1 byte │ 1 byte │1 b  │ 1 byte│1 byte │ N bytes   │  1 byte  │
└────────┴────────┴─────┴────────┴───────┴───────────┴──────────┘
```

### Field Descriptions

| Field | Offset | Size | Description |
|-------|--------|------|-------------|
| Header0 | 0 | 1 | Always `0xFF` |
| Header1 | 1 | 1 | Always `0xFF` |
| ID | 2 | 1 | Servo ID (0-253, 254=broadcast) |
| Length | 3 | 1 | Number of bytes following (including instruction/error and checksum) |
| Instruction/Error | 4 | 1 | Instruction code (TX) or error flags (RX) |
| Parameters | 5+ | N | Instruction-specific data |
| Checksum | last | 1 | `~(ID + Length + Instruction + Params) & 0xFF` |

### Instruction Codes

| Code | Name | Constant | Description |
|------|------|----------|-------------|
| 1 | PING | `INST_PING` | Check if servo is alive |
| 2 | READ | `INST_READ` | Read data from servo registers |
| 3 | WRITE | `INST_WRITE` | Write data to servo registers |
| 4 | REG_WRITE | `INST_REG_WRITE` | Write data, defer execution until ACTION |
| 5 | ACTION | `INST_ACTION` | Execute deferred REG_WRITE |
| 130 (0x82) | SYNC_READ | `INST_SYNC_READ` | Read same registers from multiple servos |
| 131 (0x83) | SYNC_WRITE | `INST_SYNC_WRITE` | Write to same registers on multiple servos |

### Error Bits (Status Packet)

| Bit | Value | Constant | Meaning |
|-----|-------|----------|---------|
| 0 | 1 | `ERRBIT_VOLTAGE` | Input voltage error |
| 1 | 2 | `ERRBIT_ANGLE` | Angle sensor error |
| 2 | 4 | `ERRBIT_OVERHEAT` | Overheat error |
| 3 | 8 | `ERRBIT_OVERELE` | Over-current error |
| 5 | 32 | `ERRBIT_OVERLOAD` | Overload error |

### Communication Result Codes

| Code | Constant | Meaning |
|------|----------|---------|
| 0 | `COMM_SUCCESS` | Communication successful |
| -1 | `COMM_PORT_BUSY` | Port is in use |
| -2 | `COMM_TX_FAIL` | Failed to transmit packet |
| -3 | `COMM_RX_FAIL` | Failed to receive status packet |
| -4 | `COMM_TX_ERROR` | Incorrect instruction packet |
| -5 | `COMM_RX_WAITING` | Currently receiving status packet |
| -6 | `COMM_RX_TIMEOUT` | No response from servo |
| -7 | `COMM_RX_CORRUPT` | Corrupted status packet |
| -9 | `COMM_NOT_AVAILABLE` | Function not supported |

### Special IDs

| ID | Constant | Purpose |
|----|----------|---------|
| 0-253 | — | Individual servo IDs |
| 254 | `BROADCAST_ID` | Broadcast to all servos (no response expected) |
| 252 | `MAX_ID` | Maximum assignable servo ID |

## Packet Examples

### Ping Servo (ID=1)

**TX Packet:**
```
FF FF 01 02 01 FB
│  │  │  │  │  └── Checksum: ~(01+02+01) & 0xFF = 0xFB
│  │  │  │  └───── Instruction: PING (1)
│  │  │  └──────── Length: 2
│  │  └─────────── ID: 1
│  └────────────── Header: 0xFF
└───────────────── Header: 0xFF
```

**RX Packet (success):**
```
FF FF 01 02 00 FC
│  │  │  │  │  └── Checksum: ~(01+02+00) & 0xFF = 0xFC
│  │  │  │  └───── Error: 0 (no error)
│  │  │  └──────── Length: 2
│  │  └─────────── ID: 1
│  └────────────── Header
└───────────────── Header
```

### Read Position (ID=1)

**TX Packet:**
```
FF FF 01 04 02 38 02 BE
│  │  │  │  │  │  │  └── Checksum
│  │  │  │  │  │  └───── Data length: 2 bytes
│  │  │  │  │  └──────── Start address: 56 (0x38 = STS_PRESENT_POSITION_L)
│  │  │  │  └─────────── Instruction: READ (2)
│  │  │  └────────────── Length: 4
│  │  └───────────────── ID: 1
│  └──────────────────── Header
└─────────────────────── Header
```

**RX Packet (position = 2048 = 0x0800):**
```
FF FF 01 04 00 00 08 F2
│  │  │  │  │  │  │  └── Checksum
│  │  │  │  │  │  └───── Position high byte: 0x08
│  │  │  │  │  └──────── Position low byte: 0x00
│  │  │  │  └─────────── Error: 0
│  │  │  └────────────── Length: 4
│  │  └───────────────── ID: 1
│  └──────────────────── Header
└─────────────────────── Header
```

### Write Position (ID=1, Position=2048)

**TX Packet:**
```
FF FF 01 05 03 2A 00 08 C4
│  │  │  │  │  │  │  │  └── Checksum
│  │  │  │  │  │  │  └───── Position high: 0x08
│  │  │  │  │  │  └──────── Position low: 0x00
│  │  │  │  │  └─────────── Start address: 42 (0x2A = STS_GOAL_POSITION_L)
│  │  │  │  └────────────── Instruction: WRITE (3)
│  │  │  └───────────────── Length: 5
│  │  └──────────────────── ID: 1
│  └─────────────────────── Header
└────────────────────────── Header
```

## Register Map

### EEPROM Registers (Read-Only)

| Address | Name | Size | Description |
|---------|------|------|-------------|
| 3 | `STS_MODEL_L` | 1 | Model number (low byte) |
| 4 | `STS_MODEL_H` | 1 | Model number (high byte) |

### EEPROM Registers (Read-Write)

| Address | Name | Size | Description |
|---------|------|------|-------------|
| 5 | `STS_ID` | 1 | Servo ID (0-253) |
| 6 | `STS_BAUD_RATE` | 1 | Baudrate code (0-7) |
| 9 | `STS_MIN_ANGLE_LIMIT_L` | 1 | Minimum angle limit (low) |
| 10 | `STS_MIN_ANGLE_LIMIT_H` | 1 | Minimum angle limit (high) |
| 11 | `STS_MAX_ANGLE_LIMIT_L` | 1 | Maximum angle limit (low) |
| 12 | `STS_MAX_ANGLE_LIMIT_H` | 1 | Maximum angle limit (high) |
| 26 | `STS_CW_DEAD` | 1 | Clockwise dead band |
| 27 | `STS_CCW_DEAD` | 1 | Counter-clockwise dead band |
| 31 | `STS_OFS_L` | 1 | Position correction (low) |
| 32 | `STS_OFS_H` | 1 | Position correction (high) |
| 33 | `STS_MODE` | 1 | Operating mode (0-3) |

### SRAM Registers (Read-Write)

| Address | Name | Size | Description |
|---------|------|------|-------------|
| 40 | `STS_TORQUE_ENABLE` | 1 | Torque on/off (0=off, 1=on, 128=calibrate) |
| 41 | `STS_ACC` | 1 | Acceleration (0-254) |
| 42 | `STS_GOAL_POSITION_L` | 1 | Target position (low) |
| 43 | `STS_GOAL_POSITION_H` | 1 | Target position (high) |
| 44 | `STS_GOAL_TIME_L` | 1 | Goal time (low) |
| 45 | `STS_GOAL_TIME_H` | 1 | Goal time (high) |
| 46 | `STS_GOAL_SPEED_L` | 1 | Goal speed (low) |
| 47 | `STS_GOAL_SPEED_H` | 1 | Goal speed (high) |
| 55 | `STS_LOCK` | 1 | EEPROM lock (0=unlocked, 1=locked) |

### SRAM Registers (Read-Only)

| Address | Name | Size | Description |
|---------|------|------|-------------|
| 56 | `STS_PRESENT_POSITION_L` | 1 | Current position (low) |
| 57 | `STS_PRESENT_POSITION_H` | 1 | Current position (high) |
| 58 | `STS_PRESENT_SPEED_L` | 1 | Current speed (low) |
| 59 | `STS_PRESENT_SPEED_H` | 1 | Current speed (high) |
| 60 | `STS_PRESENT_LOAD_L` | 1 | Current load (low) |
| 61 | `STS_PRESENT_LOAD_H` | 1 | Current load (high) |
| 62 | `STS_PRESENT_VOLTAGE` | 1 | Current voltage (raw) |
| 63 | `STS_PRESENT_TEMPERATURE` | 1 | Current temperature (°C) |
| 65 | `STS_STATUS` | 1 | Status bits |
| 66 | `STS_MOVING` | 1 | Moving flag (0=stopped, 1=moving) |
| 69 | `STS_PRESENT_CURRENT_L` | 1 | Current draw (low) |
| 70 | `STS_PRESENT_CURRENT_H` | 1 | Current draw (high) |

### Operating Modes

| Mode | Value | Description |
|------|-------|-------------|
| Position | 0 | Servo moves to target position |
| Speed | 1 | Continuous rotation at set speed |
| PWM | 2 | Direct PWM control |
| Step | 3 | Step servo mode |

### Status Bits (Register 65)

| Bit | Name | Meaning when set |
|-----|------|-----------------|
| 0 | Voltage | Voltage error detected |
| 1 | Sensor | Angle sensor error |
| 2 | Temperature | Over-temperature |
| 3 | Current | Over-current |
| 4 | Angle | Angle limit exceeded |
| 5 | Overload | Mechanical overload |

When a status bit is 0, the corresponding sensor is OK. When set to 1, there is an error.

## Sync Operations

### Sync Write

Sync Write sends different data to multiple servos in one packet:

```
FF FF FE [length] 83 [start_addr] [data_len] [id1] [d1...] [id2] [d2...] [checksum]
```

Used for coordinated multi-servo motion (e.g., moving a robot arm).

### Sync Read

Sync Read requests the same register data from multiple servos:

```
TX: FF FF FE [length] 82 [start_addr] [data_len] [id1] [id2] ... [checksum]
```

Each servo responds individually with its data. The library collects and parses all responses.

## EEPROM Protection

EEPROM registers (ID, baudrate, angle limits, etc.) are protected by a lock:

```
Unlock: Write 0 to STS_LOCK (register 55)
Lock:   Write 1 to STS_LOCK (register 55)
```

The `changeId()` and `changeBaudrate()` methods automatically handle EEPROM unlocking and re-locking.
