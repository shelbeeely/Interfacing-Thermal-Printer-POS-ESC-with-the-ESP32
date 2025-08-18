# ESP32 Thermal Printer Interface


![ESP32 Thermal Printer](https://img.shields.io/badge/ESP32-Thermal%20Printer-blue?style=for-the-badge&logo=espressif)
![Arduino IDE](https://img.shields.io/badge/Arduino%20IDE-Compatible-green?style=for-the-badge&logo=arduino)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

**Interface PNP-500/RS203 Thermal Printers with ESP32 Microcontroller**

*Print Receipts • QR Codes • Barcodes • Images • Text*

---

[![Circuit Digest](https://img.shields.io/badge/Circuit%20Digest-Visit%20Site-orange?style=flat&logo=web)](https://circuitdigest.com)
[![Documentation](https://img.shields.io/badge/📖%20Full%20Documentation-Read%20Tutorial-blue?style=flat)](https://circuitdigest.com/microcontroller-projects/how-to-interface-thermal-printer-with-esp32)
[![GitHub](https://img.shields.io/badge/GitHub-Source%20Code-black?style=flat&logo=github)](https://github.com/Circuit-Digest/Interfacing-Thermal-Printer-POS-ESC-with-the-ESP32)



## 🚀 Overview

This project demonstrates how to interface thermal printers (PNP-500/RS203) with ESP32 microcontrollers for creating IoT-enabled printing solutions. Perfect for building POS systems, receipt printers, inventory management, and embedded printing applications.

### ✨ Key Features

- **Multiple Print Formats**: Text, Images, QR Codes, Barcodes (CODE128, UPC-A, EAN13)
- **Professional Output**: High-quality 8 dots/mm resolution printing
- **Easy Integration**: Simple UART communication interface
- **Memory Optimized**: Efficient bitmap processing for ESP32 constraints
- **Customizable**: Adjustable darkness, alignment, and formatting options
- **Real-world Ready**: Includes GST invoice printing example

## 🛠️ Hardware Requirements

| Component | Quantity | Purpose |
|-----------|----------|---------|
| ESP32 Development Board | 1 | Main microcontroller |
| PNP-500 or RS203 Thermal Printer | 1 | Printing device |
| 1KΩ Resistors | 2 | Pull-up resistors for signal stability |
| Push Buttons | 2 | Demo triggers (optional) |
| 2S Li-Ion Battery (7.4V) | 1 | Printer power supply |
| Breadboard & Jumper Wires | - | Circuit assembly |

## 📋 Supported Thermal Printers

### PNP-500 Specifications
- **Print Method**: Direct thermal line printing
- **Paper Width**: 57mm thermal paper
- **Print Width**: 48mm effective area
- **Print Speed**: 50-80mm/sec
- **Resolution**: 8 dots/mm (384 dots/line)
- **Interfaces**: TTL UART, RS232, USB
- **Operating Voltage**: 5-9V DC (6V+ recommended for image printing)
- **Dimensions**: 76.8×77.4×47.6mm (W×D×H)
- **Print Head Life**: Up to 50km of printing

## 🔌 Circuit Diagram

```
ESP32 GPIO Pins → Thermal Printer
GPIO16 (RX2)   → TTL TX
GPIO17 (TX2)   → TTL RX  
GPIO5  (GND)   → Signal GND (with 1KΩ pulldown)
GPIO21 (VCC)   → Signal VCC (with 1KΩ pullup)
GPIO22         → Button 1 (Print Next Image)
GPIO23         → Button 2 (Print Demo Page)

Power Supply:
7.4V Li-Ion    → Printer VCC
Common GND     → All GND connections
```

> **⚠️ Important**: Use 6V+ for optimal printing quality. 5V may result in light/poor image prints.

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

2. Open the project in Arduino IDE

3. Install required libraries:
   - ESP32 Board Package
   - Arduino Core Libraries (included)

4. Upload the code to your ESP32

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

- **Button 1 (GPIO22)**: Cycle through available images
- **Button 2 (GPIO23)**: Print comprehensive demo page with all formats

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
- **Memory Monitoring**: Built-in heap usage tracking and reporting

### Print Quality Optimization
- **Darkness Control**: Adjustable from 50% to 205% in 5% steps
- **Heat Timing**: Configurable break delays (0-1750μs)
- **Paper Feed**: Precise line spacing control
- **Signal Integrity**: Pull-up resistors for noise reduction

## 🔧 Troubleshooting

### Common Issues & Solutions

| Problem | Cause | Solution |
|---------|-------|----------|
| Light/faded prints | Low voltage supply | Use 6V+ power supply |
| Blurry images | Print head contamination | Clean with isopropyl alcohol |
| No output | Wrong wiring | Check TTL connections and power |
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
├── ESP32_Thermal_Printer.ino    # Main Arduino sketch
├── data.h                       # Bitmap image data
├── DOC/
│   └── User Manual PNP-500.pdf  # Printer documentation
├── images/                      # Sample images for testing
└── README.md                    # This file
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

`ESP32` `Thermal-Printer` `POS` `ESC-POS` `Receipt-Printer` `QR-Code` `Barcode` `IoT` `Arduino` `Microcontroller` `PNP-500` `RS203` `Embedded-Systems` `Print-Technology`
