# PaulaNet — Amiga WiFi via the Floppy Port

**Using Paula's DMA for what she never intended**

![PaulaNET PCB](Gerber/picture.jpg)

[Buy from PCBWay](https://www.pcbway.com/project/shareproject/PaulaNET_Amiga_Wifi_Network_Adapter_a4e3f0e2.html)

PaulaNet is a WiFi network adapter for the Amiga that uses the floppy disk port as its data interface. It requires no internal hardware modification — just plug into the external floppy connector.

[Watch the Video!](https://youtu.be/AhAg-fT83o8)

---

## How It Works

The Amiga's floppy port provides a 500kbps MFM bitstream driven by Paula's DMA engine. PaulaNet exploits this as a data channel:

- A **Raspberry Pi Pico W** or **Raspberry Pi Pico 2 W** (Pico 2 W Recommended) sits between the Amiga's floppy port and the WiFi radio
- The Amiga sees PaulaNet as a standard floppy drive (eg DF1:)
- Special tracks (76, 77 and 78) are used as command/data channels
- The Pico handles WiFi via the onboard CYW43439 chip and forwards raw ethernet frames to and from the Amiga

Data is transferred using the Amiga's standard `trackdisk.device` raw read/write commands (`ETD_RAWREAD` / `ETD_RAWWRITE`), so no custom hardware drivers are needed at the transport level. 
A custom `PaulaNET.device` SanaII device sits above this and presents a standard network interface to Roadshow (or other TCP/IP stacks).
The Pico and Amiga can also compress the data to improve speed and reliability.

### Track Map

| Track | Function                                                  |
|-------|-----------------------------------------------------------|
| 0–74  | Normal AmigaDOS disk (contains driver and config program) |
| 75    | Access point scan results                                 |
| 76    | Device configuration                                      |
| 77    | Ethernet data                                             |

### Data Protocol

- **Amiga → Pico** (lower disk side): Raw data with optional RLE compression 
- **Pico → Amiga** (upper disk side): Raw write with optional RLE compression.
- A small sync header is transmitted first so the Amiga can determine exact data size before issuing the full read, minimising latency

The RLE compression is designed so that it produces an output that can always be locked onto by Paula and the Pico regardless of the data input. This allows maximum transfer speeds.
The algorithm minimises loss as bes as possible. A worse case random full size packet (1536 bytes) would end up being 1549, 13 extra bytes. Not too bad overhead. Best case, could shrink a packet 1536 packet to 26 bytes
If this is disabled, data is MFM encoded which halfs the data rate.

### Hardware

The PCB uses 2N7002 MOSFETs as level shifters between the 5V floppy signals and the Pico's 3.3V GPIOs, implementing open-collector compatible outputs. 
All floppy signals are brought out to the Pico's GPIO pins. Gerber files for the PCB are included in the `Gerber` folder.

---

## Repository Structure

| Folder             | Contents                                                                   |
|--------------------|----------------------------------------------------------------------------|
| `Pico/`            | Raspberry Pi Pico 1/2 W firmware (C++, CMake)                              |
| `NetDevice/`       | `PaulaNET.device` Amiga Exec network device driver                         |
| `AmigaConfig/`     | Workbench configuration program (GadTools UI)                              |
| `MakeDiskImage/`   | PC-side tool to generate the ADF and export sector data to `DiskImage.cpp` |
| `Gerber/`          | PCB hardware design files                                                  |

---

## Current Status & Test Results

- Transfers at approximately **43 KB/s** sustained throughput
- Ping times of **80–120ms** on an unaccelerated Amiga 1200
- Tested with **Roadshow** TCP/IP stack
- Works on stock Amiga hardware (no accelerator required)
- Compatible with Raspberry Pi Pico W and 2W 
- Workbench mounts the built-in disk image containing the driver and config program automatically
- WiFi configuration (SSID, password, hostname) stored in Pico flash

### Comparison with PlipBox

|                       | PaulaNet      | PlipBox                              |
|-----------------------|---------------|--------------------------------------|
| Connection            | Floppy port   | Parallel port                        |
| Throughput            | ~43 KB/s      | ~30–60 KB/s (faster with better CPU) |
| Ping (unaccelerated)  | 80–120ms      | 60–100ms                             |
| CPU overhead          | Minimal (DMA) | High (bit-banging)                   |
| Hardware mod required | No            | No                                   |

PaulaNet's DMA-based approach means CPU speed has minimal impact on transfer performance, giving it an advantage on slower unaccelerated machines.

---

## Building

### Pico Firmware

Requires the Raspberry Pi Pico SDK 2.2.0 or later.

Flash the resulting `.uf2` to the Pico via BOOTSEL mode (hold BOOTSEL button while powering up then drag-drop the uf2 file to the drive that appeared) or `picotool`.

### Amiga Software

Built using the [Bartman GCC Amiga toolchain](https://github.com/bebbo/amiga-gcc). Open the relevant folder in VS Code with the Amiga debug extension installed.
Driver uses the VBCC compiler

---

## Potential Future Plans

- **bsdsocket.library proxy** — stub the Amiga's socket library so TCP/IP calls are forwarded to the Pico's lwIP stack, eliminating the need for a third-party TCP/IP stack on the Amiga entirely
- **OTA firmware updates** — allow the Pico firmware to be updated over WiFi from the Amiga config program
- **Improved ping latency** — investigation into bypassing `trackdisk.device` via `disk.resource` for lower-overhead direct hardware access
- **Compression improvements** — better compression algorithms for improved throughput on compressible traffic
- **Amiga OS 3.x icons and Workbench integration** — polished Workbench presence for the config program

---

## Credits & Acknowledgements

- Inspired by projects like [PlipBox](https://github.com/cnvogelg/plipbox) and the broader Amiga networking community
- Built with the [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) and [lwIP](https://savannah.nongnu.org/projects/lwip/)

---

## Licence

Copyright © 2026 RobSmithDev. All rights reserved.
