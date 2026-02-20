# Math

This document explains the mathematical models, kinematics, position encoding, speed calculations, and timing formulas used in the ST3215 servo library.

## Position Encoding

### 12-Bit Position Range

The ST3215 servo uses a 12-bit absolute position encoder:

```
Position range: 0 to 4095  (2^12 - 1)
Resolution:     360° / 4096 ≈ 0.0879° per step
Middle point:   2048
```

| Position | Degrees | Description |
|----------|---------|-------------|
| 0 | 0° | Minimum position |
| 1024 | ~90° | Quarter turn |
| 2048 | ~180° | Center / middle |
| 3072 | ~270° | Three-quarter turn |
| 4095 | ~360° | Maximum position |

### Degrees to Position Conversion

```
position = degrees × (4096 / 360)
degrees  = position × (360 / 4096)
```

In C++:
```cpp
uint16_t degreesToPosition(double degrees) {
    return static_cast<uint16_t>(degrees * 4096.0 / 360.0);
}

double positionToDegrees(uint16_t position) {
    return position * 360.0 / 4096.0;
}
```

## Speed Encoding

### Speed Units

Speed is measured in **steps per second** (step/s):

```
Range:  0 to 3400 step/s
```

### Speed to Physical Units

```
Angular velocity (°/s)  = speed × (360 / 4096)
Angular velocity (RPM)  = speed × (360 / 4096) / 6
                        = speed × 60 / 4096
```

| Speed (step/s) | °/s | RPM |
|----------------|-----|-----|
| 0 | 0 | 0 |
| 500 | ~43.9 | ~7.3 |
| 1000 | ~87.9 | ~14.6 |
| 2400 | ~210.9 | ~35.2 |
| 3400 | ~298.8 | ~49.8 |

### Signed Speed (for Rotation)

In continuous rotation mode, speed direction is encoded in bit 15:

```
Positive speed: clockwise rotation
Negative speed: counterclockwise rotation

Encoding:
  if speed >= 0:  value = speed
  if speed < 0:   value = |speed| | (1 << 15)
```

In C++:
```cpp
// Encoding (in rotate())
uint16_t speed_magnitude = std::abs(speed);
std::vector<uint8_t> txpacket = {lobyte(speed_magnitude), hibyte(speed_magnitude)};
if (speed < 0) {
    txpacket[1] |= (1 << 7);  // Set bit 15 of the 16-bit value (bit 7 of high byte)
}

// Decoding (in readSpeed())
int16_t speed = toHost(raw_speed, 15);
// toHost: if bit 15 is set, negate the remaining bits
```

## Acceleration Encoding

### Acceleration Units

Acceleration is measured in **100 × steps per second²**:

```
Range:  0 to 254
Unit:   100 step/s²
```

Actual acceleration = `acc × 100` step/s²

| Register Value | Actual Acceleration (step/s²) | Approximate (°/s²) |
|---------------|-------------------------------|---------------------|
| 0 | 0 (instant) | 0 |
| 1 | 100 | ~8.8 |
| 50 | 5,000 | ~439 |
| 100 | 10,000 | ~879 |
| 254 | 25,400 | ~2,231 |

## Motion Timing Calculation

### `moveTo()` Wait Time Formula

When `moveTo()` is called with `wait=true`, the library estimates the motion duration using trapezoidal velocity profile kinematics:

```
Given:
  d = |target_position - current_position|  (distance in steps)
  v = speed                                 (max speed in step/s)
  a = acc × 100                             (acceleration in step/s²)
```

#### Phase 1: Acceleration Time

Time to reach maximum speed:
```
t_acc = v / a
```

#### Phase 2: Distance During Acceleration

Distance covered while accelerating and decelerating (assuming symmetric):
```
d_acc = 0.5 × a × t_acc²
```

#### Case A: Triangular Profile (Short Distance)

If `d_acc ≥ d`, the servo never reaches full speed:
```
t_wait = √(2 × d / a)
```

```
Speed ▲
      │    /\
      │   /  \
      │  /    \
      │ /      \
      └──────────► Time
         t_wait
```

#### Case B: Trapezoidal Profile (Long Distance)

If `d_acc < d`, the servo reaches full speed and cruises:
```
d_cruise = d - d_acc
t_wait = t_acc + d_cruise / v
```

```
Speed ▲
      │   ┌──────┐
      │  /│      │\
      │ / │      │ \
      │/  │      │  \
      └───┴──────┴───► Time
      t_acc cruise decel
```

### C++ Implementation

```cpp
bool ST3215::moveTo(uint8_t sts_id, uint16_t position, uint16_t speed,
                    uint8_t acc, bool wait) {
    if (wait && curr_pos.has_value()) {
        uint16_t distance = std::abs(
            static_cast<int>(position) - static_cast<int>(curr_pos.value())
        );

        double time_to_speed = static_cast<double>(speed) / (acc * 100.0);
        double distance_acc = 0.5 * (acc * 100.0) * time_to_speed * time_to_speed;

        double time_wait;
        if (distance_acc >= distance) {
            // Triangular profile
            time_wait = std::sqrt(2.0 * distance / acc);
        } else {
            // Trapezoidal profile
            double remain_distance = distance - distance_acc;
            time_wait = time_to_speed + (remain_distance / speed);
        }

        std::this_thread::sleep_for(std::chrono::duration<double>(time_wait));
    }
}
```

## Position Correction Encoding

### Correction Value Format

Position correction uses an 11-bit magnitude with a sign bit at bit 11:

```
Bits 0-10:  Magnitude (0-2047)
Bit 11:     Sign (0 = positive, 1 = negative)
Bits 12-15: Unused
```

### Encoding

```cpp
uint16_t correction_magnitude = std::abs(correction);
if (correction_magnitude > 2047) {
    correction_magnitude = 2047;  // Clamp to MAX_CORRECTION
}

std::vector<uint8_t> txpacket = {lobyte(correction_magnitude), hibyte(correction_magnitude)};
if (correction < 0) {
    txpacket[1] |= (1 << 3);  // Set bit 11 (bit 3 of high byte)
}
```

### Decoding

```cpp
uint16_t mask = 0x07FF;
int16_t bits = correction & mask;
if ((correction & 0x0800) != 0) {
    bits = -1 * (bits & 0x7FF);
}
```

## Voltage and Current Conversion

### Voltage

Raw register value to volts:
```
voltage_V = raw_value × 0.1
```

Example: raw value 72 → 7.2V

### Current

Raw register value to milliamps:
```
current_mA = raw_value × 6.5
```

Example: raw value 10 → 65mA

### Load

Raw register value to percentage:
```
load_percent = raw_value × 0.1
```

Example: raw value 500 → 50.0%

## Packet Timing

### Transmission Time

Time to transmit one byte at a given baudrate:
```
tx_time_per_byte = (1000 / baudrate) × 10  [ms]
```

The factor of 10 accounts for start bit + 8 data bits + stop bit = 10 bits per byte.

| Baudrate | Time per Byte |
|----------|---------------|
| 1,000,000 | 0.01 ms |
| 500,000 | 0.02 ms |
| 115,200 | 0.0868 ms |
| 38,400 | 0.2604 ms |

### Packet Timeout

```
timeout = tx_time_per_byte × packet_length + tx_time_per_byte × 3 + LATENCY_TIMER
```

Where `LATENCY_TIMER = 50 ms` accounts for USB-serial adapter latency.

## Calibration (TareServo) Math

The `tareServo()` function calibrates a servo by finding its physical limits:

### Algorithm

1. Reset position correction to 0
2. Rotate counterclockwise at speed -250 until blocked → `min_position`
3. Rotate clockwise at speed 250 until blocked → `max_position`
4. Calculate the correction to center the range at position 0

### Distance Calculation

```
if min_position >= max_position:
    distance = (MAX_POSITION - min_position + max_position) / 2
else:
    distance = (max_position - min_position) / 2
```

### Correction Calculation

```
if min_position > MAX_POSITION / 2:
    correction = min_position - MAX_POSITION - 1
else:
    correction = min_position
```

After applying the correction:
```
new_min = 0
new_max = distance × 2
center  = distance
```

The servo is then moved to the center position.

## Checksum Calculation

The STS protocol uses a simple inverted sum checksum:

```
checksum = ~(sum of bytes from ID to last parameter) & 0xFF
```

In C++:
```cpp
uint8_t checksum = 0;
for (size_t idx = 2; idx < total_packet_length - 1; ++idx) {
    checksum += txpacket[idx];
}
txpacket[total_packet_length - 1] = ~checksum & 0xFF;
```

This is verified on reception by computing the same sum and comparing.
