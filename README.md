This is the VScode/PIOArduino source code and the PCB that was created using EasyEDA Pro.

## Odometer CAN message

The module broadcasts the odometer in standard CAN frame `0x604` (DLC 8).
Bytes 0-3 contain the `U32` `CANDataField` value as an unsigned big-endian
integer in hundredths of a mile; bytes 4-7 are zero. For example,
`0x000004D2` represents 12.34 miles.

The ESP32 stores the cumulative VSS pulse count in NVS and writes it after each
`ODOMETER_SAVE_INTERVAL_MILES` traveled (default 1 mile). This limits flash
writes; an unexpected power loss can lose up to one configured save interval
of newly accumulated distance. The pulse-to-mile conversion uses
`PULSES_PER_MILE` in `src/configuration.h`. If NVS cannot be opened, an error
is reported over Serial and the odometer continues in volatile mode.
