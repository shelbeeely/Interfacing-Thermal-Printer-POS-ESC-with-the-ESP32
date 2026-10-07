/*
 * Thermal Printer Controller for the Seeed Studio XIAO ESP32S3 Sense
 * 
 * Author: Rithik Krisna M / CircuitDigest.com
 * Date: August/2025
 * Updated: October/2026 - ported to the XIAO ESP32S3 Sense (ESP32-S3R8)
 * 
 * Purpose:
 * This code controls a thermal printer via ESP32, providing various printing capabilities
 * including text, barcodes, QR codes, and bitmap images. It offers both serial command
 * interface and physical button control.
 * 
 * Features:
 * - Supports multiple bitmap images stored in PROGMEM
 * - Handles various barcode types (CODE128, UPCA, EAN13)
 * - QR code generation capability
 * - Text formatting options (alignment, underline, inverse)
 * - Upside-down printing mode
 * - Memory-efficient bitmap printing with chunking
 * - Serial command interface for remote control
 * - Physical buttons for demo modes
 * - Power management for printer
 * - Memory monitoring for ESP32
 * 
 * Working:
 * 1. Initializes printer hardware and serial interfaces
 * 2. Provides two control methods:
 *    - Serial commands via USB (for debugging and remote control)
 *    - Physical buttons (for standalone operation)
 * 3. Processes images in chunks to handle large bitmaps with limited RAM
 * 4. Implements rotation for upside-down printing
 * 5. Offers demo modes to showcase all printer capabilities
 * 
 * Hardware Connections (XIAO ESP32S3 Sense):
 * - XIAO D6 (GPIO43, TX) -> printer TTL RX
 * - XIAO D7 (GPIO44, RX) <- printer TTL TX (optional, see README - the
 *   sketch never reads from the printer, so this can be left unconnected)
 * - XIAO GND -> printer TTL GND and printer power supply GND
 * - Buttons on D0/GPIO1 (Key1) and D1/GPIO2 (Key2), wired to GND
 *
 * Arduino IDE board settings:
 * - Board: "XIAO_ESP32S3" (esp32 by Espressif Systems, core 3.x)
 * - PSRAM: "OPI PSRAM" (enables the 8MB PSRAM on the ESP32-S3R8)
 * - USB CDC On Boot: "Enabled" (so Serial goes to the USB-C port)
 * 
 * Serial Commands:
 * Type 'HELP' for full list of available commands
 */

#include <Arduino.h>
#include "data.h"
#include <esp_heap_caps.h>

// Struct for bitmap images - contains metadata for each available image
struct BitmapImage {
  const char* name;           // Human-readable name for the image
  const unsigned char* data;  // Pointer to image data in PROGMEM
  int width;                  // Image width in pixels
  int height;                 // Image height in pixels
};

// List of all available images - stored in PROGMEM to save RAM
const BitmapImage availableImages[] = {
  { "circuitDigestLarge", circuitDigestLarge, 360, 360 },
  { "tony", tony, 380, 528 },
  { "naruto", naruto, 380, 615 },
  { "circuitDigestSmall", circuitDigestSmall, 200, 56 },
  { "luffy", luffy, 340, 263 },
  { "luffy2", luffy2, 340, 287 },
  { "SpritedAway", SpritedAway, 380, 395 },
  { "Totoro", Totoro, 360, 500 },
  { "minato", minato, 380, 458 },
  { "itachi", itachi, 380, 733 },
  { "starkIndustriesLogo", starkIndustriesLogo, 380, 61 },
  { "AvengersLogo", AvengersLogo, 380, 456 },
  { "gojo", gojo, 380, 609 }
};

// Calculate number of available images at compile time
const int numAvailableImages = sizeof(availableImages) / sizeof(availableImages[0]);

// Hardware pin definitions for the XIAO ESP32S3 Sense.
// D0-D7 are free on the Sense; D8-D10 and GPIO21 are shared with the
// expansion board's microSD slot, so avoid them if you use the SD card.
#define PRINTER_RX 44  // D7 - UART1 RX (from printer TX, optional)
#define PRINTER_TX 43  // D6 - UART1 TX (to printer RX)
#define PRINTER_BAUD 9600  // Check your printer's self-test page if output is garbled

// Button input pins with internal pullup
#define Key1 1   // D0 - Button 1 - cycles through images
#define Key2 2   // D1 - Button 2 - prints demo page

// Serial communication setup. Serial is the native USB-C port on the XIAO,
// so UART1 is free for the printer.
HardwareSerial printerSerial(1);

// Constants for memory and performance optimization
const int IMAGE_WIDTH = 200;   // Default image width (not currently used)
const int IMAGE_HEIGHT = 56;   // Default image height (not currently used)
const unsigned long debounceDelay = 200;  // Button debounce delay in milliseconds

// Global state variables
bool globalUpsideDown = true;          // Global upside-down printing flag
unsigned long lastKey1Press = 0;      // Last time Key1 was pressed (for debouncing)
unsigned long lastKey2Press = 0;      // Last time Key2 was pressed (for debouncing)
int imageIndex = 0;                   // Current image index for button cycling

// Function prototypes - forward declarations for better organization
void printBitmapGS_Method(const unsigned char* progmemData, int width, int height);
void printTestPattern(int width, int height, int pattern);
void handleSerialCommand(String cmd);
void printMemoryStats(const char* tag);
void setGlobalUpsideDown(bool enable);

// Memory diagnostics helper - prints current heap status for ESP32
void printMemoryStats(const char* tag) {
  Serial.print("[MEM] ");
  Serial.print(tag);
  Serial.print(" | FreeHeap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.print(" | MinFreeHeap: ");
  Serial.print(ESP.getMinFreeHeap());
  Serial.print(" | MaxAllocHeap: ");
  Serial.print(ESP.getMaxAllocHeap());
  Serial.print(" | HeapSize: ");
  Serial.print(ESP.getHeapSize());
  Serial.print(" | FreePSRAM: ");
  Serial.print(ESP.getFreePsram());
  Serial.print(" / ");
  Serial.println(ESP.getPsramSize());

}

// Set global upside-down printing mode for all output
void setGlobalUpsideDown(bool enable) {
  globalUpsideDown = enable;
  upsideDownPrinting(enable ? 1 : 0);  // Apply to printer immediately
  Serial.print("Global upside-down set to: ");
  Serial.println(enable ? "ON" : "OFF");
}

// Arduino setup function - runs once at startup
void setup() {
  // Configure button input pins with internal pullup resistors
  pinMode(Key1, INPUT_PULLUP);
  pinMode(Key2, INPUT_PULLUP);

  // Initialize serial communications
  Serial.begin(115200);                               // Debug monitor over native USB
  while (!Serial && millis() < 3000) delay(10);       // Wait briefly for the USB serial monitor
  printerSerial.begin(PRINTER_BAUD, SERIAL_8N1, PRINTER_RX, PRINTER_TX);

  delay(1000);  // Give printer time to initialize

  // Set initial printer alignment to center
  align(1);

  // Print memory status and set initial upside-down state
  printMemoryStats("setup done");
  setGlobalUpsideDown(true);  // Default: enable upside-down printing
}

// Main Arduino loop - handles serial commands and button presses
void loop() {
  // Handle serial command input with static buffer for efficiency
  static String inputBuffer = "";
  inputBuffer.reserve(64);  // Pre-allocate string memory to reduce fragmentation
  
  // Process incoming serial data character by character
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      // End of command - process if buffer has content
      if (inputBuffer.length() > 0) {
        handleSerialCommand(inputBuffer);
        inputBuffer = "";  // Clear buffer for next command
      }
    } else {
      inputBuffer += c;
    }
  }

  // Button handling with debouncing
  // Key1: Print next image in sequence
  if (digitalRead(Key1) == LOW) {
    if (millis() - lastKey1Press > debounceDelay) {
      demoPrintNextImage();
      lastKey1Press = millis();
    }
  }
  
  // Key2: Print comprehensive demo page
  if (digitalRead(Key2) == LOW) {
    if (millis() - lastKey2Press > debounceDelay) {
      demoPrintAllFormats();
      lastKey2Press = millis();
    }
  }
}

// Parse and execute serial commands
void handleSerialCommand(String cmd) {
  cmd.trim();  // Remove leading/trailing whitespace
  if (cmd.length() == 0) return;

  // Split command and parameters for processing
  int spaceIdx = cmd.indexOf(' ');
  String command = (spaceIdx == -1) ? cmd : cmd.substring(0, spaceIdx);
  String params = (spaceIdx == -1) ? "" : cmd.substring(spaceIdx + 1);
  command.toUpperCase();  // Make command case-insensitive

  // Command processing - organized alphabetically for maintainability
  if (command == "HELP") {
    // Display all available commands and their usage
    Serial.println("Available commands:");
    Serial.println("QRCODE <data>");
    Serial.println("BARCODE <type> <data> (CODE128, UPCA, EAN13)");
    Serial.println("ALIGN <0|1|2>");
    Serial.println("DARKNESS <percent 50-205> <delay_us 0-1750>");
    Serial.print("BITMAP <name> (Available:");
    for (int i = 0; i < numAvailableImages; ++i) {
      Serial.print(" ");
      Serial.print(availableImages[i].name);
      if (i < numAvailableImages - 1) Serial.print(",");
    }
    Serial.println(")");
    Serial.println("TESTPATTERN <pattern 0-3>");
    Serial.println("TEXT <text> (bitmap text)");
    Serial.println("TEXTMODE <text> (native text)");
    Serial.println("UPSIDEDOWN <0|1>");
    Serial.println("UNDERLINE <0|1|2>");
    Serial.println("INVERSE <0|1>");
    Serial.println("FEED <n>");
    Serial.println("MEM");
    Serial.println("GLOBALUPSIDEDOWN <0|1> (all output upside down)");
    Serial.println("---");
    return;
  }
  
  if (command == "QRCODE") {
    printQRCode(params);
    return;
  }
  
  if (command == "BARCODE") {
    parseBarcode(params);
    return;
  }
  
  if (command == "ALIGN") {
    int n = params.toInt();
    align(n);
    Serial.println("Align set to: " + String(n));
    return;
  }
  
  if (command == "DARKNESS") {
    // Parse darkness parameters: percentage and delay
    int sp = params.indexOf(' ');
    if (sp == -1) {
      Serial.println("Usage: DARKNESS <percent> <delay_us>");
      return;
    }
    int percent = params.substring(0, sp).toInt();
    int delayus = params.substring(sp + 1).toInt();
    setDarknessAndDelay(percent, delayus);
    Serial.println("Darkness set to: " + String(percent) + "% Delay: " + String(delayus) + "us");
    return;
  }
  
  if (command == "BITMAP") {
    if (params.length() == 0) {
      // Show usage and available bitmaps
      Serial.println("Usage: BITMAP <name>");
      Serial.print("Available:");
      for (int i = 0; i < numAvailableImages; ++i) {
        Serial.print(" ");
        Serial.print(availableImages[i].name);
        if (i < numAvailableImages - 1) Serial.print(",");
      }
      Serial.println();
      return;
    }
    printNamedBitmap(params);
    return;
  }
  
  if (command == "TESTPATTERN") {
    int pattern = params.toInt();
    printTestPattern(100, 50, pattern);
    return;
  }
  
  
  if (command == "TEXTMODE") {
    printerSerial.println(params);
    Serial.println("Printed native text: " + params);
    feedLines(2);
    return;
  }
  
  if (command == "UPSIDEDOWN") {
    int n = params.toInt();
    upsideDownPrinting(n);
    Serial.println("UpsideDown: " + String(n));
    printerSerial.println("Sample: UpsideDown mode " + String(n));
    feedLines(2);
    return;
  }
  
  if (command == "UNDERLINE") {
    int n = params.toInt();
    underlineMode(n);
    Serial.println("Underline: " + String(n));
    printerSerial.println("Sample: Underline mode " + String(n));
    feedLines(2);
    return;
  }
  
  if (command == "INVERSE") {
    int n = params.toInt();
    inverseMode(n);
    Serial.println("Inverse: " + String(n));
    printerSerial.println("Sample: Inverse mode " + String(n));
    feedLines(2);
    return;
  }
  
  if (command == "FEED") {
    int n = params.toInt();
    feedLines(n);
    Serial.println("Feed lines: " + String(n));
    return;
  }
  
  if (command == "MEM") {
    printMemoryStats("on demand");
    return;
  }
  
  if (command == "GLOBALUPSIDEDOWN") {
    int n = params.toInt();
    setGlobalUpsideDown(n != 0);
    return;
  }
  
  // Unknown command
  Serial.println("Unknown command. Type HELP for list.");
}

// Set print darkness (% from 50 to 205%) and break delay (µs)
// This controls how dark the print appears and timing between print operations
void setDarknessAndDelay(uint8_t densityPercent, uint16_t breakDelayUs) {
  // Clamp density between 50% and 205% in steps of 5%
  if (densityPercent < 50) densityPercent = 50;
  if (densityPercent > 205) densityPercent = 205;

  // Convert % to value (0–31) where 50% + 5*x = density
  uint8_t densityLevel = (densityPercent - 50) / 5;

  // Clamp delay to nearest 250us step (max = 7 * 250 = 1750us)
  if (breakDelayUs > 1750) breakDelayUs = 1750;
  uint8_t breakTimeLevel = breakDelayUs / 250;

  // Merge both into single byte: D7–D5 = breakTimeLevel, D4–D0 = densityLevel
  uint8_t n = (breakTimeLevel << 5) | (densityLevel & 0x1F);

  // Send command: DC2 '#' n
  printerSerial.write(0x12);  // DC2
  printerSerial.write(0x23);  // '#'
  printerSerial.write(n);
}

// Print QR Code with specified data
// Uses the printer's built-in QR code generation capability
void printQRCode(String data) {
  upsideDownPrinting(globalUpsideDown ? 1 : 0);
  if (data.length() == 0) {
    Serial.println("Usage: QRCODE <data>");
    return;
  }

  // [1] Set QR code model (Model 2 is standard)
  printerSerial.write(0x1D);  // GS
  printerSerial.write(0x28);  // (
  printerSerial.write(0x6B);  // k
  printerSerial.write(0x04);  // pL
  printerSerial.write(0x00);  // pH
  printerSerial.write(0x31);  // cn
  printerSerial.write(0x41);  // fn
  printerSerial.write(0x32);  // Model 2
  printerSerial.write(0x00);  // n

  // [2] Set size of module (3–16 dots per module)
  printerSerial.write(0x1D);
  printerSerial.write(0x28);
  printerSerial.write(0x6B);
  printerSerial.write(0x03);
  printerSerial.write(0x00);
  printerSerial.write(0x31);
  printerSerial.write(0x43);
  printerSerial.write(0x06);  // module size = 6

  // [3] Set error correction level (L=48, M=49, Q=50, H=51)
  printerSerial.write(0x1D);
  printerSerial.write(0x28);
  printerSerial.write(0x6B);
  printerSerial.write(0x03);
  printerSerial.write(0x00);
  printerSerial.write(0x31);
  printerSerial.write(0x45);
  printerSerial.write(0x31);  // 'M' level (medium error correction)

  // [4] Store data in QR code buffer
  int len = data.length() + 3;  // +3 for command overhead
  byte pL = len & 0xFF;         // Low byte of length
  byte pH = (len >> 8) & 0xFF;  // High byte of length

  printerSerial.write(0x1D);
  printerSerial.write(0x28);
  printerSerial.write(0x6B);
  printerSerial.write(pL);
  printerSerial.write(pH);
  printerSerial.write(0x31);
  printerSerial.write(0x50);
  printerSerial.write(0x30);
  printerSerial.print(data);  // Actual QR code data

  // [5] Print the stored QR code
  printerSerial.write(0x1D);
  printerSerial.write(0x28);
  printerSerial.write(0x6B);
  printerSerial.write(0x03);
  printerSerial.write(0x00);
  printerSerial.write(0x31);
  printerSerial.write(0x51);
  printerSerial.write(0x30);

  Serial.println("Printed QR Code: " + data);
  feedLines(3);  // Add some space after QR code
}

// Parse barcode command and call appropriate printing function
void parseBarcode(String params) {
  int spaceIndex = params.indexOf(' ');
  if (spaceIndex == -1) {
    Serial.println("Usage: BARCODE <type> <data>");
    Serial.println("Types: CODE128, UPCA, EAN13");
    return;
  }

  String type = params.substring(0, spaceIndex);
  String data = params.substring(spaceIndex + 1);

  // Route to appropriate barcode printing function
  if (type == "CODE128") {
    printBarcode_CODE128(data);
  } else if (type == "UPCA") {
    printBarcode_UPCA(data);
  } else if (type == "EAN13") {
    printBarcode_EAN13(data);
  } else {
    Serial.println("Unknown barcode type: " + type);
  }
}

// Print CODE128 barcode - variable length alphanumeric
void printBarcode_CODE128(String data) {
  upsideDownPrinting(globalUpsideDown ? 1 : 0);
  
  // Set barcode height (in dots)
  printerSerial.write(0x1D);  // GS
  printerSerial.write(0x68);  // h
  printerSerial.write(0x50);  // Height = 80 dots
  delay(100);

  // Set barcode width (module width)
  printerSerial.write(0x1D);  // GS
  printerSerial.write(0x77);  // w
  printerSerial.write(0x02);  // Width = 2 dots per module
  delay(100);

  // Set HRI (Human Readable Interpretation) position
  printerSerial.write(0x1D);  // GS
  printerSerial.write(0x48);  // H
  printerSerial.write(0x02);  // HRI below barcode
  delay(100);

  // Print the barcode
  printerSerial.write(0x1D);           // GS
  printerSerial.write(0x6B);           // k
  printerSerial.write(0x49);           // CODE128 type
  printerSerial.write(data.length());  // Data length
  printerSerial.print(data);           // Barcode data

  Serial.println("Printed CODE128: " + data);
  feedLines(2);
}

// Print UPC-A barcode - requires exactly 11 digits
void printBarcode_UPCA(String data) {
  upsideDownPrinting(globalUpsideDown ? 1 : 0);
  
  // Validate data length for UPC-A
  if (data.length() != 11) {
    Serial.println("UPC-A requires 11 digits");
    return;
  }

  // Set barcode parameters (same as CODE128)
  printerSerial.write(0x1D);  // GS
  printerSerial.write(0x68);  // h
  printerSerial.write(0x50);  // Height = 80 dots
  delay(100);

  printerSerial.write(0x1D);  // GS
  printerSerial.write(0x77);  // w
  printerSerial.write(0x02);  // Width = 2
  delay(100);

  printerSerial.write(0x1D);  // GS
  printerSerial.write(0x48);  // H
  printerSerial.write(0x02);  // HRI below
  delay(100);

  // Print UPC-A barcode
  printerSerial.write(0x1D);           // GS
  printerSerial.write(0x6B);           // k
  printerSerial.write(0x41);           // UPC-A type
  printerSerial.write(data.length());  // Data length
  printerSerial.print(data);

  Serial.println("Printed UPC-A: " + data);
  feedLines(2);
}

// Print EAN13 barcode - requires exactly 12 digits
void printBarcode_EAN13(String data) {
  upsideDownPrinting(globalUpsideDown ? 1 : 0);
  
  // Validate data length for EAN13
  if (data.length() != 12) {
    Serial.println("EAN13 requires 12 digits");
    return;
  }

  // Set barcode parameters (same as others)
  printerSerial.write(0x1D);  // GS
  printerSerial.write(0x68);  // h
  printerSerial.write(0x50);  // Height = 80 dots
  delay(100);

  printerSerial.write(0x1D);  // GS
  printerSerial.write(0x77);  // w
  printerSerial.write(0x02);  // Width = 2
  delay(100);

  printerSerial.write(0x1D);  // GS
  printerSerial.write(0x48);  // H
  printerSerial.write(0x02);  // HRI below
  delay(100);

  // Print EAN13 barcode
  printerSerial.write(0x1D);           // GS
  printerSerial.write(0x6B);           // k
  printerSerial.write(0x43);           // EAN13 type
  printerSerial.write(data.length());  // Data length
  printerSerial.print(data);

  Serial.println("Printed EAN13: " + data);
  feedLines(2);
}

// Printer control functions - these send ESC/POS commands to control printer behavior

// Print and feed n lines - creates vertical spacing
void feedLines(int n) {
  printerSerial.write(0x1B);  // ESC
  printerSerial.write(0x64);  // d
  printerSerial.write(n);     // 0≤n≤255
  delay(500);  // Allow time for paper feed
}

// Turn upside-down printing mode on/off
void upsideDownPrinting(int n) {
  printerSerial.write(0x1B);  // ESC
  printerSerial.write(0x7B);  // {
  printerSerial.write(n);     // 0/1 - OFF/ON
  delay(500);
}

// Set underline mode - 0=off, 1=1dot thick, 2=2dots thick
void underlineMode(int n) {
  printerSerial.write(0x1B);  // ESC
  printerSerial.write(0x2D);  // -
  printerSerial.write(n);     // 0/1/2
  delay(500);
}

// Set text justification - 0=left, 1=center, 2=right
void align(int n) {
  printerSerial.write(0x1B);  // ESC
  printerSerial.write(0x61);  // a
  printerSerial.write(n);     // 0/1/2
  delay(500);
}

// Turn inverse (white on black) mode on/off
void inverseMode(int n) {
  printerSerial.write(0x1D);  // GS
  printerSerial.write(0x42);  // B
  printerSerial.write(n);     // 0/1 - OFF/ON
  delay(500);
}

// Print bitmap image using standard GS v command - optimized for memory usage
void printBitmap(uint8_t* imageData, int width, int height) {
  // Validate dimensions to prevent printer damage or memory issues
  if (width <= 0 || height <= 0 || width > 512 || height > 1662) {
    Serial.println("Invalid image dimensions");
    return;
  }

  // Calculate bytes per line (width must be multiple of 8 for 1-bit images)
  int bytesPerLine = (width + 7) / 8;

  // Use normal mode (m=0) for standard density printing
  uint8_t m = 0;

  // Calculate command parameters for image size
  uint8_t xL = bytesPerLine & 0xFF;        // Low byte of horizontal bytes
  uint8_t xH = (bytesPerLine >> 8) & 0xFF; // High byte of horizontal bytes
  uint8_t yL = height & 0xFF;              // Low byte of vertical dots
  uint8_t yH = (height >> 8) & 0xFF;       // High byte of vertical dots

  // Send GS v command for bitmap printing
  printerSerial.write(0x1D);  // GS
  printerSerial.write(0x76);  // v
  printerSerial.write(0x30);  // 0
  printerSerial.write(m);     // mode
  printerSerial.write(xL);    // xL
  printerSerial.write(xH);    // xH
  printerSerial.write(yL);    // yL
  printerSerial.write(yH);    // yH

  // Send image data efficiently
  int totalBytes = bytesPerLine * height;
  for (int i = 0; i < totalBytes; i++) {
    printerSerial.write(imageData[i]);
  }

  Serial.println("Bitmap printed: " + String(width) + "x" + String(height));
  feedLines(2);
}

// Create test pattern in bitmap buffer - useful for printer testing
void createTestPattern(uint8_t* buffer, int width, int height, int pattern) {
  int bytesPerLine = (width + 7) / 8;

  // Generate different test patterns
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < bytesPerLine; x++) {
      uint8_t byte_val = 0;

      // Process 8 pixels at a time (1 byte)
      for (int bit = 0; bit < 8; bit++) {
        int pixel_x = x * 8 + bit;
        if (pixel_x >= width) break; // Don't exceed image width

        bool pixel = false;
        // Generate different patterns based on pattern parameter
        switch (pattern) {
          case 0:  // Checkerboard pattern
            pixel = ((pixel_x / 8) + (y / 8)) % 2 == 0;
            break;
          case 1:  // Horizontal lines
            pixel = (y % 4) < 2;
            break;
          case 2:  // Vertical lines
            pixel = (pixel_x % 4) < 2;
            break;
          case 3:  // Diagonal lines
            pixel = ((pixel_x + y) % 8) < 4;
            break;
          default: // Default diagonal pattern
            pixel = (pixel_x + y) % 2 == 0;
        }

        // Set bit in byte if pixel should be printed
        if (pixel) {
          byte_val |= (0x80 >> bit);
        }
      }

      buffer[y * bytesPerLine + x] = byte_val;
    }
  }
}

// Print a test pattern - useful for testing printer functionality
void printTestPattern(int width, int height, int pattern) {
  // Limit size to prevent memory issues on ESP32
  if (width > 384 || height > 400) {
    Serial.println("Image too large for ESP32 memory");
    return;
  }

  printMemoryStats("before test pattern alloc");

  int bytesPerLine = (width + 7) / 8;
  int totalBytes = bytesPerLine * height;

  // Allocate memory for image data
  uint8_t* imageData = (uint8_t*)malloc(totalBytes);
  if (imageData == NULL) {
    Serial.println("Failed to allocate memory for image");
    printMemoryStats("alloc fail");
    return;
  }

  // Create the test pattern
  createTestPattern(imageData, width, height, pattern);

  // Print the bitmap
  printBitmap(imageData, width, height);

  printMemoryStats("before free test pattern");
  // Free allocated memory
  free(imageData);
  printMemoryStats("after free test pattern");
}

// Alternative bitmap printing method using DC2 * command
// This method prints line by line to avoid spacing issues
void printBitmapDC2_Method(const unsigned char* progmemData, int width, int height) {
  upsideDownPrinting(globalUpsideDown ? 1 : 0);

  // Set printer to maximum darkness for bitmap printing
  printerSerial.write(0x12);  // DC2
  printerSerial.write(0x23);  // #
  printerSerial.write(0x9F);  // Maximum darkness setting
  delay(50);

  Serial.println("Using DC2 * method (Method 1)...");

  int bytesPerLine = (width + 7) / 8;

  // Print line by line to avoid spacing issues
  for (int line = 0; line < height; line++) {
    // Send DC2 * command for each line
    printerSerial.write(0x12);                 // DC2
    printerSerial.write(0x2A);                 // *
    printerSerial.write(0x01);                 // r = 1 line height
    printerSerial.write(bytesPerLine & 0xFF);  // n = bytes per line

    // Send one line of data from PROGMEM
    for (int i = 0; i < bytesPerLine; i++) {
      uint8_t byte_val = pgm_read_byte(progmemData + (line * bytesPerLine) + i);
      printerSerial.write(byte_val);
    }

    // Small delay between lines for printer processing
    delayMicroseconds(10);
  }

  Serial.println("DC2 * method completed!");
  feedLines(2);
  
  // Reset printer darkness to normal
  printerSerial.write(0x12);  // DC2
  printerSerial.write(0x23);  // #
  printerSerial.write(0x58);  // Normal darkness setting
  delay(50);
}

// Helper function: Rotate a 1bpp bitmap by 180 degrees
// Returns a newly allocated buffer that caller must free()
// Used for upside-down printing when globalUpsideDown is enabled
uint8_t* rotateBitmap1bpp_180(const uint8_t* src, int width, int height) {
  int bytesPerLine = (width + 7) / 8;
  int totalBytes = bytesPerLine * height;
  
  // Allocate destination buffer and clear it
  uint8_t* dst = (uint8_t*)calloc(totalBytes, 1);
  if (!dst) return nullptr;
  
  // Rotate each pixel by 180 degrees
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      // Extract pixel from source
      int srcByte = y * bytesPerLine + (x / 8);
      int srcBit = 7 - (x % 8);
      bool pixel = (src[srcByte] >> srcBit) & 1;
      
      // Calculate rotated position
      int nx = width - 1 - x;
      int ny = height - 1 - y;
      
      // Set pixel in destination
      int dstByte = ny * bytesPerLine + (nx / 8);
      int dstBit = 7 - (nx % 8);
      if (pixel) dst[dstByte] |= (1 << dstBit);
    }
  }
  return dst;
}

// Main bitmap printing method using GS v command with chunking
// This is the primary method for printing large images efficiently
void printBitmapGS_Method(const unsigned char* progmemData, int width, int height) {
  upsideDownPrinting(globalUpsideDown ? 1 : 0);
  align(1);  // Center align for images
  
  // Set printer to maximum darkness for better image quality
  printerSerial.write(0x12);  // DC2
  printerSerial.write(0x23);  // #
  printerSerial.write(0x9F);  // Maximum darkness
  delay(50);

  const unsigned char* dataToPrint = progmemData;
  int w = width, h = height;
  uint8_t* rotated = nullptr;

  // Handle global upside-down printing by rotating the entire image
  if (globalUpsideDown) {
    // Copy from PROGMEM to RAM, then rotate 180°
    int bytesPerLine = (width + 7) / 8;
    int totalBytes = bytesPerLine * height;
    
    uint8_t* ramCopy = (uint8_t*)malloc(totalBytes);
    if (ramCopy) {
      // Copy image data from PROGMEM to RAM for rotation
      for (int i = 0; i < totalBytes; ++i)
        ramCopy[i] = pgm_read_byte(progmemData + i);
      
      // Rotate the image 180 degrees
      rotated = rotateBitmap1bpp_180(ramCopy, width, height);
      free(ramCopy);
      
      if (rotated) {
        dataToPrint = rotated;
      }
    }
  }

  // Validate image dimensions
  if (w <= 0 || h <= 0) {
    Serial.println("Invalid image size");
    if (rotated) free(rotated);
    return;
  }

  int bytesPerLine = (w + 7) / 8;
  int totalChunks = (h + 23) / 24;  // Process in 24-line chunks for memory efficiency

  // Set line spacing to 0 for seamless image printing
  printerSerial.write(0x1B);  // ESC
  printerSerial.write(0x33);  // '3'
  printerSerial.write(0x00);  // 0 dots line spacing

  // Process image in chunks to manage memory usage
  for (int chunk = 0; chunk < totalChunks; chunk++) {
    int linesInChunk = min(24, h - chunk * 24);  // Last chunk may be smaller
    
    // Calculate chunk parameters
    int yL = linesInChunk & 0xFF;
    int yH = (linesInChunk >> 8) & 0xFF;
    int xL = bytesPerLine & 0xFF;
    int xH = (bytesPerLine >> 8) & 0xFF;

    // Send GS v command for this chunk
    printerSerial.write(0x1D);  // GS
    printerSerial.write(0x76);  // 'v'
    printerSerial.write(0x30);  // '0'
    printerSerial.write(0x00);  // m = 0 (normal density)
    printerSerial.write(xL);
    printerSerial.write(xH);
    printerSerial.write(yL);
    printerSerial.write(yH);

    // Send chunk data
    for (int row = 0; row < linesInChunk; row++) {
      int baseIndex = (chunk * 24 + row) * bytesPerLine;
      for (int col = 0; col < bytesPerLine; col++) {
        printerSerial.write(dataToPrint[baseIndex + col]);
      }
    }
    
    delay(30);  // Allow printer to process chunk
  }

  // Feed paper and restore normal settings
  printerSerial.write(0x1B);  // ESC
  printerSerial.write(0x64);  // d
  printerSerial.write(3);     // Feed 3 lines

  Serial.println("Finished printing image in chunks.");
  
  // Reset printer darkness to normal
  printerSerial.write(0x12);  // DC2
  printerSerial.write(0x23);  // #
  printerSerial.write(0x58);  // Normal darkness
  delay(50);
  
  // Clean up rotated image memory if allocated
  if (rotated) free(rotated);
}

// Helper function to print a named bitmap from the availableImages array
void printNamedBitmap(String imageName) {
  imageName.trim();
  imageName.toLowerCase();  // Make search case-insensitive
  
  // Search for matching image name
  for (int i = 0; i < numAvailableImages; ++i) {
    String candidate = String(availableImages[i].name);
    candidate.toLowerCase();
    if (imageName == candidate) {
      printBitmapGS_Method(availableImages[i].data, availableImages[i].width, availableImages[i].height);
      return;
    }
  }
  
  Serial.println("Unknown image name. Type HELP for list.");
}

// Demo function: Print next image in sequence (triggered by Key1)
void demoPrintNextImage() {
  static int currentImageIndex = 0;
  
  const BitmapImage& img = availableImages[currentImageIndex];
  Serial.print("[Key1] Printing image: ");
  Serial.println(img.name);
  
  printBitmapGS_Method(img.data, img.width, img.height);
  
  // Advance to next image, wrap around to beginning
  currentImageIndex = (currentImageIndex + 1) % numAvailableImages;
}

// Demo function: Print comprehensive demo page (triggered by Key2)
// Shows all printer capabilities in one printout
void demoPrintAllFormats() {
  Serial.println("[Key2] Printing all barcodes, QR, and formatted text...");
  
  // Print demo header
  align(1);  // Center align
  printerSerial.println("--- DEMO PAGE ---");
  feedLines(1);

  // Demonstrate different barcode types
  align(0);  // Left align for labels
  printerSerial.println("CODE128: 123456");
  printBarcode_CODE128("123456");
  
  printerSerial.println("UPCA: 12345678901");
  printBarcode_UPCA("12345678901");
  
  printerSerial.println("EAN13: 123456789012");
  printBarcode_EAN13("123456789012");
  feedLines(1);

  // Demonstrate QR Code
  align(1);  // Center align for QR
  printerSerial.println("QR Code: HelloWorld");
  printQRCode("HelloWorld");
  feedLines(1);

  // Demonstrate text alignment
  align(0);
  printerSerial.println("Align Left");
  align(1);
  printerSerial.println("Align Center");
  align(2);
  printerSerial.println("Align Right");
  feedLines(1);

  // Demonstrate underline modes
  underlineMode(1);
  printerSerial.println("Underline 1");
  underlineMode(2);
  printerSerial.println("Underline 2");
  underlineMode(0);  // Turn off underline
  feedLines(1);

  // Demonstrate inverse printing
  inverseMode(1);
  printerSerial.println("Inverse ON");
  inverseMode(0);
  printerSerial.println("Inverse OFF");
  feedLines(1);

  // Demonstrate upside down printing
  upsideDownPrinting(1);
  printerSerial.println("Upside Down ON");
  upsideDownPrinting(0);
  printerSerial.println("Upside Down OFF");
  feedLines(2);

  // Print normal footer text
  printerSerial.println("Normal text");
  feedLines(2);
  
  Serial.println("[Key2] Demo page printed.");
}