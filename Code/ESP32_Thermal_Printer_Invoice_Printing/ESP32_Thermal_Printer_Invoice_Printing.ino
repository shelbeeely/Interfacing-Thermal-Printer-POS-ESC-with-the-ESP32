/*
 * Thermal Printer Invoice System for ESP32
 * 
 * Author: Rithik Krisna M
 * Date: August/2025
 * 
 * Purpose:
 * This code implements a complete invoice printing system for thermal printers using ESP32.
 * It handles invoice creation, formatting, and printing with professional layout including
 * company logo, customer details, itemized products, totals calculations, and payment information.
 * 
 * Features:
 * - Complete invoice generation system
 * - Professional formatting with logo and company info
 * - Itemized product listing with quantities and totals
 * - Automatic tax and discount calculations
 * - QR code and barcode generation for invoice reference
 * - Multiple configuration options
 * - Serial command interface for invoice management
 * - Sample data mode for testing
 * - Memory-efficient bitmap printing
 * 
 * Working:
 * 1. System initializes with default or sample invoice data
 * 2. Provides serial interface for:
 *    - Creating new invoices
 *    - Printing existing invoices
 *    - Loading sample data
 *    - Configuring system settings
 * 3. Formats invoice data professionally with:
 *    - Company header and logo
 *    - Customer information
 *    - Itemized product table
 *    - Calculated totals (subtotal, tax, discount)
 *    - Payment information and terms
 *    - QR code and barcode for reference
 * 4. Optimizes thermal printer output with:
 *    - Proper line spacing
 *    - Font sizing and styling
 *    - Bitmap handling
 *    - Paper cutting commands
 * 
 * Hardware Connections:
 * - Thermal printer connected via UART2 (pins 16/RXD2, 17/TXD2)
 * - Uses standard ESC/POS commands for printer control
 * 
 * Serial Commands:
 * new     - Create new invoice
 * print   - Print current invoice
 * sample  - Load sample invoice data
 * config  - Configure system settings
 * status  - Show current invoice status
 */

#define RXD2 16  // UART2 RX pin for printer communication
#define TXD2 17  // UART2 TX pin for printer communication
HardwareSerial printerSerial(2);  // Use UART2 for printer communication

// Company logo bitmap data (200x56 pixels)
const unsigned char circuitDigestSmall [] PROGMEM = { 
  // [Original bitmap data remains unchanged...]
};

// Invoice configuration structure
struct InvoiceConfig {
  String companyName = "Circuit Digest Components";
  String companyAddress = "123 Electronics Street, Tech City, TC 12345";
  String companyPhone = "Phone: +91 85479 56894";
  String companyEmail = "sales@circuitdigest.com";
  String companyWebsite = "www.circuitdigest.com";
  String invoiceTitle = "INVOICE";
  String currency = "Rs.";
  float taxRate = 0.18;  // 18% tax
  bool printLogo = true;
  bool printQRCode = true;
  bool printBarcode = true;
};

// Product structure
struct Product {
  String name;
  String description;
  int quantity;
  float unitPrice;
  String category;
};

// Customer structure
struct Customer {
  String name;
  String address;
  String phone;
  String email;
  String gstNumber;
};

// Invoice data structure
struct Invoice {
  String invoiceNumber;
  String date;
  String dueDate;
  Customer customer;
  Product products[20];  // Max 20 products
  int productCount;
  float discount;
  String notes;
  String paymentTerms;
};

InvoiceConfig config;
Invoice currentInvoice;

void setup() {
  Serial.begin(115200);
  printerSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  
  delay(1000);
  
  // Initialize printer
  resetPrinter();
  
  Serial.println("=== Invoice Printer Ready ===");
  Serial.println("Commands:");
  Serial.println("1. new - Create new invoice");
  Serial.println("2. print - Print current invoice");
  Serial.println("3. sample - Load sample data");
  Serial.println("4. config - Configure printer settings");
  Serial.println("5. status - Show current invoice");
  Serial.println("==============================");

  delay(10000);

  initializeSampleInvoice();
      Serial.println("Sample invoice loaded.");

       printProfessionalInvoice();
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    if (command == "new") {
      createNewInvoice();
    } else if (command == "print") {
      printProfessionalInvoice();
    } else if (command == "sample") {
      initializeSampleInvoice();
      Serial.println("Sample invoice loaded.");
    } else if (command == "config") {
      configureSettings();
    } else if (command == "status") {
      showInvoiceStatus();
    } else {
      Serial.println("Unknown command. Type 'new', 'print', 'sample', 'config', or 'status'");
    }
  }
}

void createNewInvoice() {
  Serial.println("Creating new invoice...");
  
  // Clear current invoice
  currentInvoice = Invoice();
  
  // Get invoice details
  Serial.print("Invoice Number: ");
  currentInvoice.invoiceNumber = waitForInput();
  
  Serial.print("Date (YYYY-MM-DD): ");
  currentInvoice.date = waitForInput();
  
  Serial.print("Due Date (YYYY-MM-DD): ");
  currentInvoice.dueDate = waitForInput();
  
  Serial.print("Payment Terms: ");
  currentInvoice.paymentTerms = waitForInput();
  
  Serial.print("Discount (%): ");
  currentInvoice.discount = waitForInput().toFloat();
  
  // Get customer details
  Serial.println("\n--- Customer Details ---");
  Serial.print("Customer Name: ");
  currentInvoice.customer.name = waitForInput();
  
  Serial.print("Address: ");
  currentInvoice.customer.address = waitForInput();
  
  Serial.print("Phone: ");
  currentInvoice.customer.phone = waitForInput();
  
  Serial.print("Email: ");
  currentInvoice.customer.email = waitForInput();
  
  Serial.print("GST Number (optional): ");
  currentInvoice.customer.gstNumber = waitForInput();
  
  // Get products
  Serial.println("\n--- Products ---");
  Serial.print("Number of products (max 20): ");
  int productCount = waitForInput().toInt();
  
  if (productCount > 20) productCount = 20;
  if (productCount < 1) productCount = 1;
  
  currentInvoice.productCount = productCount;
  
  for (int i = 0; i < productCount; i++) {
    Serial.println("\nProduct " + String(i + 1) + ":");
    Serial.print("Name: ");
    currentInvoice.products[i].name = waitForInput();
    
    Serial.print("Description: ");
    currentInvoice.products[i].description = waitForInput();
    
    Serial.print("Quantity: ");
    currentInvoice.products[i].quantity = waitForInput().toInt();
    
    Serial.print("Unit Price: ");
    currentInvoice.products[i].unitPrice = waitForInput().toFloat();
    
    Serial.print("Category: ");
    currentInvoice.products[i].category = waitForInput();
  }
  
  Serial.print("\nNotes (optional): ");
  currentInvoice.notes = waitForInput();
  
  Serial.println("\nInvoice created successfully!");
  Serial.println("Type 'print' to print the invoice or 'status' to review details.");
}

void configureSettings() {
  Serial.println("\n--- Configuration ---");
  Serial.println("1. Company Name: " + config.companyName);
  Serial.println("2. Tax Rate: " + String(config.taxRate * 100) + "%");
  Serial.println("3. Currency: " + config.currency);
  Serial.println("4. Print Logo: " + String(config.printLogo ? "Yes" : "No"));
  Serial.println("5. Print QR Code: " + String(config.printQRCode ? "Yes" : "No"));
  Serial.println("6. Print Barcode: " + String(config.printBarcode ? "Yes" : "No"));
  Serial.println("Enter option number (1-6) or 'back' to return:");
  
  String option = waitForInput();
  
  if (option == "1") {
    Serial.print("New Company Name: ");
    config.companyName = waitForInput();
  } else if (option == "2") {
    Serial.print("New Tax Rate (%): ");
    config.taxRate = waitForInput().toFloat() / 100.0;
  } else if (option == "3") {
    Serial.print("New Currency Symbol: ");
    config.currency = waitForInput();
  } else if (option == "4") {
    Serial.print("Print Logo (y/n): ");
    config.printLogo = (waitForInput().charAt(0) == 'y');
  } else if (option == "5") {
    Serial.print("Print QR Code (y/n): ");
    config.printQRCode = (waitForInput().charAt(0) == 'y');
  } else if (option == "6") {
    Serial.print("Print Barcode (y/n): ");
    config.printBarcode = (waitForInput().charAt(0) == 'y');
  }
  
  Serial.println("Configuration updated!");
}

void showInvoiceStatus() {
  if (currentInvoice.invoiceNumber.length() == 0) {
    Serial.println("No invoice created yet. Type 'new' to create one.");
    return;
  }
  
  Serial.println("\n--- Current Invoice Status ---");
  Serial.println("Invoice #: " + currentInvoice.invoiceNumber);
  Serial.println("Date: " + currentInvoice.date);
  Serial.println("Customer: " + currentInvoice.customer.name);
  Serial.println("Products: " + String(currentInvoice.productCount));
  Serial.println("Subtotal: " + config.currency + String(calculateSubtotal(), 2));
  Serial.println("Total: " + config.currency + String(calculateTotal(), 2));
  Serial.println("Status: Ready to print");
}

String waitForInput() {
  while (!Serial.available()) {
    delay(10);
  }
  String input = Serial.readStringUntil('\n');
  input.trim();
  return input;
}

// Initialize sample invoice (for testing)
void initializeSampleInvoice() {
  currentInvoice.invoiceNumber = "INV-2025-001";
  currentInvoice.date = "2025-07-17";
  currentInvoice.dueDate = "2025-08-16";
  currentInvoice.discount = 5.0;
  currentInvoice.notes = "Thank you for your business!";
  currentInvoice.paymentTerms = "Net 30 days";
  
  currentInvoice.customer.name = "Semicon Circuits Pvt Ltd";
  currentInvoice.customer.address = "456 Maker Avenue, Innovation City, IC 67890";
  currentInvoice.customer.phone = "+91 54856 25846";
  currentInvoice.customer.email = "orders@semiconcircuits.com";
  currentInvoice.customer.gstNumber = "GST123456789";
  
  currentInvoice.productCount = 2;
  currentInvoice.products[0] = {"ESP32 DevKit V1", "WiFi + Bluetooth microcontroller", 2, 450.00, "MCU"};
  currentInvoice.products[1] = {"PNP-500 Thermal Printer", "58mm thermal receipt printer", 1, 1800.00, "PRINTER"};
}

// Main invoice printing function
void printProfessionalInvoice() {
  if (currentInvoice.invoiceNumber.length() == 0) {
    Serial.println("No invoice to print. Create an invoice first.");
    return;
  }
  
  Serial.println("Printing invoice...");
  
  resetPrinter();
  
  if (config.printLogo) {
    printCompanyLogo();
  }
  
  printInvoiceHeader();
  printCompanyInfo();
  printCustomerInfo();
  printInvoiceDetails();
  printProductsTable();
  printTotals();
  printFooter();
  
  if (config.printQRCode) {
    printInvoiceQRCode();
  }
  
  if (config.printBarcode) {
    printInvoiceBarcode();
  }
  
  feedAndCut();
  
  Serial.println("Invoice printed successfully!");
}

// Optimized printer functions
void resetPrinter() {
  printerSerial.write(0x1B);  // ESC
  printerSerial.write(0x40);  // @ (Initialize printer)
  delay(100);
  
  setDarknessAndDelay(80, 250);
  setLineSpacing(30);
  align(0);
}

void printCompanyLogo() {
  align(1);
  printerSerial.write(0x12);
  printerSerial.write(0x23);
  printerSerial.write(0x9F);
  delay(50);
  
  printBitmapGS_Method(circuitDigestSmall, 200, 56);
  
  printerSerial.write(0x12);
  printerSerial.write(0x23);
  printerSerial.write(0x58);
  delay(50);
  
  printSeparatorLine();
}

void printInvoiceHeader() {
  align(1);
  printBold(true);
  setFontSize(2, 1);
  printerSerial.println(config.invoiceTitle);
  printBold(false);
  setFontSize(1, 1);
  feedLines(1);
}

void printCompanyInfo() {
  align(0);
  printBold(true);
  printerSerial.println("FROM:");
  printBold(false);
  
  printerSerial.println(config.companyName);
  printerSerial.println(config.companyAddress);
  printerSerial.println(config.companyPhone);
  printerSerial.println(config.companyEmail);
  printerSerial.println(config.companyWebsite);
  feedLines(1);
}

void printCustomerInfo() {
  align(0);
  printBold(true);
  printerSerial.println("BILL TO:");
  printBold(false);
  
  printerSerial.println(currentInvoice.customer.name);
  printerSerial.println(currentInvoice.customer.address);
  printerSerial.println(currentInvoice.customer.phone);
  printerSerial.println(currentInvoice.customer.email);
  if (currentInvoice.customer.gstNumber.length() > 0) {
    printerSerial.println("GST: " + currentInvoice.customer.gstNumber);
  }
  feedLines(1);
}

void printInvoiceDetails() {
  printSeparatorLine();
  printerSerial.println("Invoice #: " + currentInvoice.invoiceNumber);
  printerSerial.println("Date: " + currentInvoice.date);
  printerSerial.println("Due Date: " + currentInvoice.dueDate);
  printerSerial.println("Terms: " + currentInvoice.paymentTerms);
  printSeparatorLine();
}

void printProductsTable() {
  printBold(true);
  align(0);
  printerSerial.println("ITEM           QTY  PRICE  TOTAL");
  printBold(false);
  printerSerial.println("--------------------------------");
  
  for (int i = 0; i < currentInvoice.productCount; i++) {
    Product& prod = currentInvoice.products[i];
    float lineTotal = prod.quantity * prod.unitPrice;
    
    // Format and print product line
    String line = formatProductLine(prod.name, prod.quantity, prod.unitPrice, lineTotal);
    printerSerial.println(line);
    
    if (prod.description.length() > 0) {
      printerSerial.println("  " + prod.description);
    }
    
    printerSerial.println("  Cat: " + prod.category);
    printerSerial.println("");
  }
  
  printerSerial.println("--------------------------------");
}

String formatProductLine(String name, int qty, float price, float total) {
  String line = name;
  if (line.length() > 16) line = line.substring(0, 16);
  while (line.length() < 16) line += " ";
  
  String qtyStr = String(qty);
  while (qtyStr.length() < 3) qtyStr = " " + qtyStr;
  line += qtyStr + " ";
  
  String priceStr = config.currency + String(price, 2);
  while (priceStr.length() < 6) priceStr = " " + priceStr;
  line += priceStr + " ";
  
  String totalStr = config.currency + String(total, 2);
  while (totalStr.length() < 6) totalStr = " " + totalStr;
  line += totalStr;
  
  return line;
}

void printTotals() {
  float subtotal = calculateSubtotal();
  float discountAmount = subtotal * (currentInvoice.discount / 100);
  float taxableAmount = subtotal - discountAmount;
  float taxAmount = taxableAmount * config.taxRate;
  float total = taxableAmount + taxAmount;
  
  align(2);
  printerSerial.println("Subtotal: " + config.currency + String(subtotal, 2));
  
  if (currentInvoice.discount > 0) {
    printerSerial.println("Discount (" + String(currentInvoice.discount, 1) + "%): -" + config.currency + String(discountAmount, 2));
  }
  
  printerSerial.println("Tax (" + String(config.taxRate * 100, 0) + "%): " + config.currency + String(taxAmount, 2));
  printerSerial.println("================================");
  
  printBold(true);
  setFontSize(1, 2);
  printerSerial.println("TOTAL: " + config.currency + String(total, 2));
  setFontSize(1, 1);
  printBold(false);
  
  align(0);
  feedLines(1);
}

void printFooter() {
  align(1);
  printSeparatorLine();
  
  if (currentInvoice.notes.length() > 0) {
    printerSerial.println(currentInvoice.notes);
    feedLines(1);
  }
  
  printerSerial.println("Payment Methods:");
  printerSerial.println("Cash | Card | Bank Transfer");
  feedLines(1);
  
  printerSerial.println("Printed: " + currentInvoice.date);
  align(0);
}

void printInvoiceQRCode() {
  align(1);
  feedLines(1);
  printerSerial.println("Scan for payment:");
  
  String qrData = "INV:" + currentInvoice.invoiceNumber + "|AMT:" + String(calculateTotal(), 2) + "|DUE:" + currentInvoice.dueDate;
  printQRCode(qrData);
}

void printInvoiceBarcode() {
  align(1);
  feedLines(1);
  printerSerial.println("Invoice Reference:");
  
  String barcodeData = currentInvoice.invoiceNumber;
  barcodeData.replace("INV-", "");
  barcodeData.replace("-", "");
  
  while (barcodeData.length() < 12) {
    barcodeData = "0" + barcodeData;
  }
  
  printBarcode_EAN13(barcodeData);
}

// Utility functions
float calculateSubtotal() {
  float subtotal = 0;
  for (int i = 0; i < currentInvoice.productCount; i++) {
    subtotal += currentInvoice.products[i].quantity * currentInvoice.products[i].unitPrice;
  }
  return subtotal;
}

float calculateTotal() {
  float subtotal = calculateSubtotal();
  float discountAmount = subtotal * (currentInvoice.discount / 100);
  float taxableAmount = subtotal - discountAmount;
  float taxAmount = taxableAmount * config.taxRate;
  return taxableAmount + taxAmount;
}

void printSeparatorLine() {
  printerSerial.println("================================");
}

void feedAndCut() {
  feedLines(3);
  printerSerial.write(0x1D);  // GS
  printerSerial.write(0x56);  // V
  printerSerial.write(0x00);  // Full cut
}

// Printer control functions
void printBold(bool enable) {
  printerSerial.write(0x1B);
  printerSerial.write(0x45);
  printerSerial.write(enable ? 1 : 0);
}

void setFontSize(int width, int height) {
  printerSerial.write(0x1D);
  printerSerial.write(0x21);
  printerSerial.write(((width - 1) << 4) | (height - 1));
}

void setLineSpacing(int spacing) {
  printerSerial.write(0x1B);
  printerSerial.write(0x33);
  printerSerial.write(spacing);
}

void setDarknessAndDelay(uint8_t densityPercent, uint16_t breakDelayUs) {
  if (densityPercent < 50) densityPercent = 50;
  if (densityPercent > 205) densityPercent = 205;
  
  uint8_t densityLevel = (densityPercent - 50) / 5;
  
  if (breakDelayUs > 1750) breakDelayUs = 1750;
  uint8_t breakTimeLevel = breakDelayUs / 250;
  
  uint8_t n = (breakTimeLevel << 5) | (densityLevel & 0x1F);
  
  printerSerial.write(0x12);
  printerSerial.write(0x23);
  printerSerial.write(n);
}

void feedLines(int n) {
  printerSerial.write(0x1B);
  printerSerial.write(0x64);
  printerSerial.write(n);
  delay(100);
}

void align(int n) {
  printerSerial.write(0x1B);
  printerSerial.write(0x61);
  printerSerial.write(n);
  delay(50);
}

void printQRCode(String data) {
  if (data.length() == 0) return;
  
  // Set model
  printerSerial.write(0x1D);
  printerSerial.write(0x28);
  printerSerial.write(0x6B);
  printerSerial.write(0x04);
  printerSerial.write(0x00);
  printerSerial.write(0x31);
  printerSerial.write(0x41);
  printerSerial.write(0x32);
  printerSerial.write(0x00);
  
  // Set size
  printerSerial.write(0x1D);
  printerSerial.write(0x28);
  printerSerial.write(0x6B);
  printerSerial.write(0x03);
  printerSerial.write(0x00);
  printerSerial.write(0x31);
  printerSerial.write(0x43);
  printerSerial.write(0x06);
  
  // Set error correction
  printerSerial.write(0x1D);
  printerSerial.write(0x28);
  printerSerial.write(0x6B);
  printerSerial.write(0x03);
  printerSerial.write(0x00);
  printerSerial.write(0x31);
  printerSerial.write(0x45);
  printerSerial.write(0x31);
  
  // Store data
  int len = data.length() + 3;
  byte pL = len & 0xFF;
  byte pH = (len >> 8) & 0xFF;
  
  printerSerial.write(0x1D);
  printerSerial.write(0x28);
  printerSerial.write(0x6B);
  printerSerial.write(pL);
  printerSerial.write(pH);
  printerSerial.write(0x31);
  printerSerial.write(0x50);
  printerSerial.write(0x30);
  printerSerial.print(data);
  
  // Print QR code
  printerSerial.write(0x1D);
  printerSerial.write(0x28);
  printerSerial.write(0x6B);
  printerSerial.write(0x03);
  printerSerial.write(0x00);
  printerSerial.write(0x31);
  printerSerial.write(0x51);
  printerSerial.write(0x30);
  
  feedLines(2);
}

void printBarcode_EAN13(String data) {
  if (data.length() != 12) return;
  
  printerSerial.write(0x1D);
  printerSerial.write(0x68);
  printerSerial.write(0x50);
  delay(50);
  printerSerial.write(0x1D);
  printerSerial.write(0x77);
  printerSerial.write(0x02);
  delay(50);
  printerSerial.write(0x1D);
  printerSerial.write(0x48);
  printerSerial.write(0x02);
  delay(50);
  
  printerSerial.write(0x1D);
  printerSerial.write(0x6B);
  printerSerial.write(0x43);
  printerSerial.write(data.length());
  printerSerial.print(data);
  
  feedLines(2);
}

void printBitmapGS_Method(const unsigned char* progmemData, int width, int height) {
  if (width <= 0 || height <= 0) return;
  
  int bytesPerLine = (width + 7) / 8;
  int totalChunks = (height + 23) / 24;
  
  // Set line spacing to 0
  printerSerial.write(0x1B);
  printerSerial.write(0x33);
  printerSerial.write(0x00);
  
  for (int chunk = 0; chunk < totalChunks; chunk++) {
    int linesInChunk = min(24, height - chunk * 24);
    int yL = linesInChunk & 0xFF;
    int yH = (linesInChunk >> 8) & 0xFF;
    
    int xL = bytesPerLine & 0xFF;
    int xH = (bytesPerLine >> 8) & 0xFF;
    
    // Send command header
    printerSerial.write(0x1D);
    printerSerial.write(0x76);
    printerSerial.write(0x30);
    printerSerial.write(0x00);
    printerSerial.write(xL);
    printerSerial.write(xH);
    printerSerial.write(yL);
    printerSerial.write(yH);
    
    // Send bitmap data for this chunk
    for (int row = 0; row < linesInChunk; row++) {
      int baseIndex = (chunk * 24 + row) * bytesPerLine;
      for (int col = 0; col < bytesPerLine; col++) {
        printerSerial.write(pgm_read_byte(progmemData + baseIndex + col));
      }
    }
  }
}