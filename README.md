# XIAO ESP32S3 Thermal Printer Interface


![XIAO ESP32S3 Thermal Printer](https://img.shields.io/badge/XIAO%20ESP32S3-Thermal%20Printer-blue?style=for-the-badge&logo=espressif)
![Arduino IDE](https://img.shields.io/badge/Arduino%20IDE-Compatible-green?style=for-the-badge&logo=arduino)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

**Interface ESC/POS TTL Thermal Printers with the Seeed Studio XIAO ESP32S3 Sense**

*Print Receipts • QR Codes • Barcodes • Images • Text*

---

[![Circuit Digest](https://img.shields.io/badge/Circuit%20Digest-Visit%20Site-orange?style=flat&logo=web)](https://circuitdigest.com)
[![Documentation](https://img.shields.io/badge/📖%20Full%20Documentation-Read%20Tutorial-blue?style=flat)](https://circuitdigest.com/microcontroller-projects/how-to-interface-thermal-printer-with-esp32)
[![GitHub](https://img.shields.io/badge/GitHub-Source%20Code-black?style=flat&logo=github)](https://github.com/Circuit-Digest/Interfacing-Thermal-Printer-POS-ESC-with-the-ESP32)



## 🚀 Overview

This project shows how to drive an ESC/POS thermal receipt printer from a microcontroller to build IoT printing projects: POS systems, receipt printers, inventory tags and other embedded printing applications.

> **2026 update:** The sketches now target the **[Seeed Studio XIAO ESP32S3 Sense](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)** (ESP32-S3R8: dual-core 240 MHz, 8 MB PSRAM, 8 MB flash, native USB-C). They build on the current Arduino-ESP32 3.x core. The recommended printer is the **DFRobot Embedded Thermal Printer V2.0 (DFR0503-EN)**, which is still in stock. The PNP-500/RS203 used in the original tutorial works unchanged. The two Adafruit TTL printers that most older tutorials use are now discontinued.

### ✨ Key Features

- **Multiple Print Formats**: Text, Images, QR Codes, Barcodes (CODE128, UPC-A, EAN13)
- **Professional Output**: High-quality 8 dots/mm resolution printing
- **Easy Integration**: Simple UART communication interface
- **Memory Optimized**: Efficient bitmap processing, with 8 MB PSRAM available on the ESP32-S3R8
- **Customizable**: Adjustable darkness, alignment, and formatting options
- **Real-world Ready**: Includes GST invoice printing example

## 🛒 Bill of Materials (2026)

| Component | Qty | Where to get it | Notes |
|-----------|-----|-----------------|-------|
| [Seeed Studio XIAO ESP32S3 Sense](https://www.seeedstudio.com/XIAO-ESP32S3-Sense-p-5639.html) | 1 | Seeed Studio (SKU 113991115, US$13.99, in stock Oct 2026), Digi-Key, Mouser | ESP32-S3R8 with camera/mic/microSD expansion board. The plain XIAO ESP32S3 also works with the same pins. |
| [DFRobot Embedded Thermal Printer V2.0 (DFR0503-EN)](https://www.dfrobot.com/product-1799.html) | 1 | DFRobot (US$39, in stock Oct 2026), [RobotShop](https://www.robotshop.com/products/dfrobot-embedded-thermal-printer-ttl-serial), [Core Electronics](https://core-electronics.com.au/embedded-thermal-printer-ttl-serial.html) | TTL + USB + RS485 + RS232, ESC/POS, 9600 baud, 203 DPI / 384 dots, 58 mm paper. [Wiki](https://wiki.dfrobot.com/dfr0503-en/) |
| 12 V ≥ 2 A DC power supply | 1 | Any | The V2.0 printer is rated 9–24 V DC at 0.5–2.5 A. Check the label on your unit, because the original V1 board was rated 5–9 V. |
| 58 mm thermal paper rolls | 1+ | Any office supplier | Roll diameter ≤ 30 mm for the DFR0503 |
| Push buttons | 2 | Any | Demo triggers (optional) |
| USB-C cable | 1 | Any | Power and programming for the XIAO |
| Jumper wires | - | Any | |

### Alternative printers

Any 58 mm ESC/POS printer with a **TTL serial** input should work. Set `PRINTER_BAUD` in the sketch to the baud rate printed on the printer's self-test page.

- **Generic 58 mm embedded panel printer modules** (sold on Amazon/eBay/AliExpress as "58mm embedded thermal receipt printer USB/RS232/TTL", e.g. [Maikrt](https://www.amazon.com/Maikrt-Embedded-Thermal-Printing-Commands/dp/B07PX9NYR3)). These are cheaper, usually run from 5–9 V, and default to 9600 baud. Quality and documentation vary between sellers.
- **PNP-500 / RS203**, the printer from the original tutorial. Its specs are below.
- **Adafruit Mini (#597) and Tiny (#2751) thermal printers**: discontinued. If you already own one, it still works. Note that the Mini runs at 19200 baud.

### PNP-500 Specifications (original printer)
- **Print Method**: Direct thermal line printing
- **Paper Width**: 57mm thermal paper
- **Print Width**: 48mm effective area
- **Print Speed**: 50-80mm/sec
- **Resolution**: 8 dots/mm (384 dots/line)
- **Interfaces**: TTL UART, RS232, USB
- **Operating Voltage**: 5-9V DC (6V+ recommended for image printing)
- **Dimensions**: 76.8×77.4×47.6mm (W×D×H)
- **Print Head Life**: Up to 50km of printing

## 🔌 Wiring (XIAO ESP32S3 Sense)

```
XIAO ESP32S3 Sense          Thermal Printer (TTL connector)
D6 / GPIO43 (TX)   ──────►  RX
D7 / GPIO44 (RX)   ◄─ ─ ─   TX   (optional: see logic-level note)
GND                ──────►  GND

D0 / GPIO1         ── Button 1 ── GND   (Print next image)
D1 / GPIO2         ── Button 2 ── GND   (Print demo page)

Power:
12V DC supply (+)  ──────►  Printer power VIN
12V DC supply (−)  ──────►  Printer power GND ── XIAO GND (common ground)
USB-C              ──────►  XIAO (power + Serial Monitor)
```

- **Common ground is required.** The printer's supply ground and the XIAO's GND must be connected.
- **Don't power the printer from the XIAO.** The print head draws 1.5–2.5 A peaks, far more than the XIAO's 5V pin can supply.
- **Logic levels.** The XIAO's 3.3 V TX output drives the TTL RX input of these printers directly. The printer's TX line may be 5 V, and **ESP32-S3 GPIOs are not 5 V tolerant**. The sketches never read from the printer, so you can leave printer TX unconnected. If you want status read-back, add a bidirectional level shifter (BSS138-type).
- **Pin choice on the Sense.** D0–D7 are free. The Sense expansion board's microSD slot uses D8–D10 and GPIO21, and GPIO21 is also the onboard user LED. The camera and mic use internal pins that aren't broken out. With this wiring the camera, mic and microSD stay available.
- No pull-up resistors and no GPIO "power" pins are needed. The original ESP32 build drove GPIO5/GPIO21 as signal GND/VCC; on the XIAO, use the real GND pin.

> **ℹ️ Note:** The circuit-diagram images in `Images/` show the original ESP32 DevKit + PNP-500 build. For the XIAO, use the pin mapping above.

> **⚠️ Important**: Under-powering the printer causes light or streaky image prints. Use a supply that meets the printer's rating (12 V for the DFR0503 V2.0, 6 V+ for the PNP-500).

## 🖥️ Software Features

### Serial Commands Interface
Control the printer via Serial Monitor with these commands:

| Command | Usage | Description |
|---------|-------|-------------|
| `HELP` | `HELP` | Show all available commands |
| `QRCODE` | `QRCODE Hello World` | Print QR code with data |
| `BARCODE` | `BARCODE CODE128 123456` | Print barcode (CODE128/UPCA/EAN13) |
| `BITMAP` | `BITMAP circuitDigestLogo` | Print stored image |
| `ALIGN` | `ALIGN 1` | Set alignment (0=left, 1=center, 2=right) |
| `DARKNESS` | `DARKNESS 80 500` | Set print darkness % and delay μs |
| `TEXTMODE` | `TEXTMODE Hello` | Print native text |
| `UPSIDEDOWN` | `UPSIDEDOWN 1` | Enable/disable upside-down printing |
| `UNDERLINE` | `UNDERLINE 1` | Set underline mode (0=off, 1=thin, 2=thick) |
| `INVERSE` | `INVERSE 1` | Enable white text on black background |
| `FEED` | `FEED 3` | Feed paper n lines |

### Barcode Support
- **CODE128**: Variable length alphanumeric
- **UPC-A**: 11-digit product codes
- **EAN13**: 12-digit international article numbers

### QR Code Features
- **Error Correction Levels**: L(7%), M(15%), Q(25%), H(30%)
- **Module Sizes**: 3-16 dots per module
- **Data Types**: Text, URLs, contact info, WiFi credentials

### Image Printing
- **Supported Format**: 1-bit monochrome bitmaps
- **Max Resolution**: 384 pixels wide (printer limitation)
- **Memory Optimized**: Chunked processing for large images
- **Rotation Support**: 180° rotation for upside-down printing

## 🚀 Quick Start

### 1. Hardware Assembly
1. Connect ESP32 to thermal printer according to circuit diagram
2. Connect 7.4V power supply to printer
3. Add optional push buttons for demo functionality

### 2. Software Setup
1. Clone this repository:
   ```bash
   git clone https://github.com/Circuit-Digest/Interfacing-Thermal-Printer-POS-ESC-with-the-ESP32.git
   ```

2. In Arduino IDE 2.x, open **File → Preferences** and add this Board Manager URL:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```

3. In **Boards Manager**, install **esp32 by Espressif Systems** (3.x; tested with 3.3.12).

4. Open `Code/ESP32_Thermal_Printer_Interfacing/ESP32_Thermal_Printer_Interfacing.ino` and select:
   - **Board**: `XIAO_ESP32S3`
   - **PSRAM**: `OPI PSRAM` (enables the 8 MB PSRAM on the ESP32-S3R8)
   - **USB CDC On Boot**: `Enabled` (the default; routes `Serial` to the USB-C port)

5. Upload. If the board isn't detected, hold **BOOT**, tap **RESET**, release **BOOT**, then upload again.

   Or with `arduino-cli`:
   ```bash
   arduino-cli compile -b esp32:esp32:XIAO_ESP32S3:PSRAM=opi Code/ESP32_Thermal_Printer_Interfacing
   arduino-cli upload  -b esp32:esp32:XIAO_ESP32S3:PSRAM=opi -p /dev/ttyACM0 Code/ESP32_Thermal_Printer_Interfacing
   ```

6. If the printout is garbled, check the baud rate on the printer's self-test page (hold the FEED button while powering on) and update `PRINTER_BAUD` in the sketch.

### 3. Testing
1. Open Serial Monitor (115200 baud)
2. Type `HELP` to see all available commands
3. Try basic commands:
   - `QRCODE https://circuitdigest.com`
   - `BARCODE CODE128 123456`
   - `BITMAP circuitDigestLogo`

## 🖼️ Adding Custom Images

### Convert Images to Bitmap Data

1. **Prepare Image**: Resize to max 384px width, convert to monochrome
2. **Use Converter**: Visit [image2cpp](https://javl.github.io/image2cpp/)
3. **Settings**:
   - Set width ≤ 384 pixels
   - Select "Arduino Code, single bitmap"
   - Invert image if needed (white areas print black)
4. **Integration**: Copy generated code to `data.h` file
5. **Update Code**: Add new image to `availableImages[]` array

### Example Image Addition
```cpp
// In data.h
const unsigned char myCustomImage[] PROGMEM = {
  0x00, 0x00, 0xFF, ... // Generated bitmap data
};

// In main code
const BitmapImage availableImages[] = {
  { "myCustomImage", myCustomImage, 200, 100 },
  // ... other images
};
```

## 📱 Button Functions

- **Button 1 (D0 / GPIO1)**: Cycle through available images
- **Button 2 (D1 / GPIO2)**: Print comprehensive demo page with all formats

## 🧠 Technical Details

### ESC/POS Command Implementation
The printer uses standard ESC/POS commands for control:

- **Text Formatting**: ESC commands for alignment, underline, inverse
- **Graphics**: GS v command for bitmap printing with chunking
- **Barcodes**: GS k command with type-specific parameters
- **QR Codes**: GS ( k command sequence for modern QR generation

### Memory Management
- **PROGMEM Storage**: Images stored in flash memory to preserve RAM
- **Chunked Processing**: Large images processed in 24-line chunks
- **Dynamic Allocation**: Temporary buffers for image rotation and processing
- **Memory Monitoring**: Built-in heap and PSRAM usage tracking and reporting
- **PSRAM**: With `OPI PSRAM` enabled, large image buffers are allocated from the ESP32-S3R8's 8 MB PSRAM

### Print Quality Optimization
- **Darkness Control**: Adjustable from 50% to 205% in 5% steps
- **Heat Timing**: Configurable break delays (0-1750μs)
- **Paper Feed**: Precise line spacing control
- **Signal Integrity**: Keep the UART wires short and share a solid ground with the printer supply

## 🔧 Troubleshooting

### Common Issues & Solutions

| Problem | Cause | Solution |
|---------|-------|----------|
| Light/faded prints | Low voltage supply | Use a supply that meets the printer's rating (12 V for DFR0503 V2.0, 6 V+ for PNP-500) |
| Blurry images | Print head contamination | Clean with isopropyl alcohol |
| No output | Wrong wiring | Check that XIAO D6 goes to printer RX, that grounds are common, and that the printer has power |
| Garbled characters | Baud rate mismatch | Read the baud rate off the self-test page and set `PRINTER_BAUD` |
| Nothing in Serial Monitor | USB CDC disabled | Set **USB CDC On Boot: Enabled** and re-upload |
| XIAO resets while printing | Shared/weak supply | Power the printer from its own supply, not the XIAO |
| Memory errors | Large images | Reduce image size or increase chunk processing |
| Paper jam | Wrong paper type | Use 57mm thermal paper |

### Status LED Indicators
- **1 blink**: Working properly
- **2 blinks**: No printer detected
- **3 blinks**: No paper detected
- **5 blinks**: Print head overheating
- **10 blinks**: Font IC missing

## 📚 Documentation & Resources

### 📖 Complete Tutorial
**[How to Interface Thermal Printer with ESP32](https://circuitdigest.com/microcontroller-projects/how-to-interface-thermal-printer-with-esp32)**

*Comprehensive guide covering hardware overview, circuit diagrams, code explanation, and practical demonstrations.*

### 🔗 Related Projects
- [Thermal Printer with Arduino Uno](https://circuitdigest.com/microcontroller-projects/thermal-printer-interfacing-with-arduino-uno)
- [Thermal Printer with Raspberry Pi](https://circuitdigest.com/microcontroller-projects/thermal-printer-interfacing-with-raspberry-pi-zero-to-print-text-images-and-bar-codes)
- [Smart Shopping Cart with POS System](https://circuitdigest.com/microcontroller-projects/smart-shopping-cart-with-automatic-billing-system-using-raspberry-pi)

### 📋 Additional Resources
- **Printer Manual**: [PNP-500 User Manual](DOC/User%20Manual%20PNP-500.pdf)
- **ESC/POS Commands**: Complete command reference included in manual
- **Image Converter**: [Online Bitmap Converter](https://javl.github.io/image2cpp/)

## 🏗️ Project Structure

```
├── Code/
│   ├── ESP32_Thermal_Printer_Interfacing/        # Main demo sketch (serial commands + buttons)
│   │   ├── ESP32_Thermal_Printer_Interfacing.ino
│   │   └── data.h                                # Bitmap image data
│   └── ESP32_Thermal_Printer_Invoice_Printing/   # GST invoice printing example
├── DOC/                                          # PNP-500 manual and leaflet
├── Images/                                       # Tutorial images (original ESP32 build)
└── README.md                                     # This file
```

## ⚡ Advanced Features

### Professional Invoice Printing
The code includes a complete GST invoice printing example with:
- Company letterhead and logo
- Itemized billing with calculations
- Tax breakdowns and totals
- QR codes for digital verification
- Professional formatting

### Customization Options
- **Power Management**: Software-controlled power pins
- **Image Processing**: 180° rotation support for upside-down mounting
- **Error Handling**: Comprehensive validation and error reporting
- **Debug Interface**: Memory usage monitoring and command logging

## 🤝 Contributing

We welcome contributions! Please:
1. Fork the repository
2. Create a feature branch
3. Test thoroughly with hardware
4. Submit a pull request

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🔗 Connect With Us



[![Website](https://img.shields.io/badge/🌐%20Website-circuitdigest.com-blue?style=flat)](https://circuitdigest.com)
[![YouTube](https://img.shields.io/badge/📺%20YouTube-Circuit%20Digest-red?style=flat&logo=youtube)](https://youtube.com/circuitdigest)
[![GitHub](https://img.shields.io/badge/💻%20GitHub-Circuit--Digest-black?style=flat&logo=github)](https://github.com/Circuit-Digest)

**Questions? Issues? Feature Requests?**  
💬 [Open an Issue](https://github.com/Circuit-Digest/Interfacing-Thermal-Printer-POS-ESC-with-the-ESP32/issues) | 📧 [Contact Us](https://circuitdigest.com/contact)

---

*Built with ❤️ by the Circuit Digest Team*



## 🏷️ Tags

`ESP32` `ESP32-S3` `XIAO-ESP32S3` `Seeed-Studio` `DFR0503` `Thermal-Printer` `POS` `ESC-POS` `Receipt-Printer` `QR-Code` `Barcode` `IoT` `Arduino` `Microcontroller` `PNP-500` `RS203` `Embedded-Systems` `Print-Technology`
