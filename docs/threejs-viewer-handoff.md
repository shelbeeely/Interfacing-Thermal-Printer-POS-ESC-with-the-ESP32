# Handoff prompt: three.js viewer for the gesture sticker camera

Copy everything below the line into a new session.

---

## Task

Build an interactive **three.js 3D viewer** of a hardware build I'm about to assemble: a **gesture-triggered thermal sticker camera**. It should show every component at true scale, how they connect, and a rough enclosure. I'll use it to plan the physical layout before buying and cutting anything.

Deliver a **single self-contained `index.html`**. Load three.js and its addons (`OrbitControls`, `CSS2DRenderer`) as ES modules from `cdn.jsdelivr.net/npm/three@<latest>` through an import map. Don't use a build step. Use **1 scene unit = 1 mm**.

## What the device does (for context)

1. An **Arducam IMX500 AI camera** runs a hand-pose model on the sensor itself and sends keypoints to the main board over SPI.
2. A **Waveshare ESP32-P4-WIFI6** board recognizes gestures. A ✌️ peace sign starts a countdown and takes a photo, 👍 prints it, and ✋ discards it. The board receives the photo over MIPI-CSI and shows a preview on a **DSI touchscreen**.
3. The P4 dithers the photo to 1-bit at **384 px wide** and sends it over TTL serial to a **DFRobot DFR0503 thermal printer** loaded with **57 mm sticker paper**.

## Components

Put all dimensions in one `PARTS` config object at the top of the file. Each part gets a `verified: true|false` flag. Draw unverified parts with a dashed outline edge and an "unverified size" badge in the info panel, so I know what to measure. Each part also gets an optional `modelUrl`: if set, load that GLB with `GLTFLoader` in place of the simple shapes, so I can drop in real CAD later.

| id | Part | Dimensions (mm) | Verified | Modeling notes |
|---|---|---|---|---|
| `printer` | DFRobot Embedded Thermal Printer V2.0 (DFR0503-EN) | Overall **82 × 58 × 44**; panel/install body **77 × 53 × 42** | ✅ (DFRobot spec) | Rectangular body with a front bezel flange (82 × 58) and a body behind it (77 × 53 × 42). Hinged paper door on top. Paper slot on the front face, 58 mm wide. Inside: a paper roll as a cylinder **57 mm long, ≤ 30 mm diameter**. Print area 48 mm, centered. Connectors (power + TTL) on the back face; exact position unverified. |
| `paper` | 57 × 30 mm thermal sticker roll | 57 wide × 30 Ø | ✅ | Paper strip exits the front slot. Animate it (see Features). |
| `p4` | Waveshare ESP32-P4-WIFI6 | **≈ 85 × 56 × 1.6** PCB, Raspberry Pi HAT–style layout | ❌ (assumed from its Pi-compatible 40-pin header; confirm against Waveshare's dimension drawing) | Green PCB. Components: 2×20 pin header along one long edge; USB-C (power/program); USB OTG HS 4-pin header; MIPI-CSI FPC connector; MIPI-DSI FPC connector; TF card slot; ESP32-C6-MINI-1 module (≈ 13 × 16.6 mm) as the Wi-Fi co-processor; MX1.25 speaker header. Four M2.5 mounting holes, Pi-style (58 × 49 mm pattern), assumed. |
| `camera` | Arducam IMX500 AI Camera for MCU (B0642) | Board **34 × 34**, depth **24.5** excluding lens | ✅ (Arducam datasheet) | Square stacked board. Front **M12 lens**: cylinder ≈ 14 Ø × 15 long (lens size unverified). Back: 22-pin MIPI FPC connector, SPI & I2C connector, USB-C. 3.3 V, 0.5 W, 8.4 g. FOV 82° D / 72° H / 56° V: draw the view cone as a toggleable translucent frustum. |
| `display` | Waveshare MIPI-DSI touchscreen | Parameter: **5", 7" or 10.1"** (dropdown) | ❌ except 10.1": outline **147 × 239**, active area **135.36 × 216.58**, 800 × 1280 | Thin slab with a bezel and active area. Default to **5"** (outline ≈ 121 × 76 × 5, unverified). Note: Waveshare's kit B may ship the 10.1", which is far too big for a handheld build. |
| `psu` | 12 V ≥ 2 A DC barrel input | Panel jack: 12 Ø × 15 | ❌ | Printer power input. Mount the jack on the enclosure back. |
| `buttons` | 2 × 12 mm tactile buttons (manual fallback: shutter, print) | 12 × 12 × 7 | ❌ | Optional; toggle in the UI. |

## Wiring (draw as colored tube curves)

Use `CatmullRomCurve3` + `TubeGeometry`, about 1 mm diameter for wires and flat ribbons for FFCs. Color by function, add a legend, and toggle each group.

| From | To | Type | Color |
|---|---|---|---|
| P4 40-pin header UART TX (GPIO TBD) | Printer TTL **RX** | wire | yellow |
| P4 GND | Printer TTL GND **and** PSU GND (common ground) | wire | black |
| Printer TTL TX | *leave unconnected*; show as a dangling stub labeled "not used: printer TX may be 5 V, ESP32 pins aren't 5 V tolerant" | wire | gray dashed |
| Camera 22-pin MIPI | P4 MIPI-CSI (15-pin) | **15–22-pin FFC, 15 cm**, which Arducam includes | ribbon, orange |
| Camera SPI (SCLK, MOSI, MISO, CS) + I2C (SDA, SCL) + 3V3 + GND | P4 40-pin header (GPIOs TBD from Arducam's `imx500-mcu-sdk` ESP32-P4 README) | wire bundle | blue (SPI), green (I2C), red (3V3) |
| Display | P4 MIPI-DSI | FFC ribbon | orange |
| 12 V PSU | Printer power input | wire pair | red / black |
| USB-C 5 V | P4 USB-C | cable stub | gray |

Rule to show in the info panel: **never power the printer from the P4.** It needs its own 12 V supply because it draws 0.5–2.5 A peaks.

## Default layout (adjustable)

A handheld or desk "instant camera" box:
- Display on the **back face**, facing the user.
- Camera on the **front face**, upper center, lens poking through.
- Printer in the **lower half**, with its paper slot on the **top face** so stickers come out the top like an instant camera. Rotate the printer so its front slot faces up.
- P4 board sandwiched behind the display.
- 12 V jack and USB-C on a side face.

Store each part's position and rotation in a `LAYOUT` object so I can tweak it. Add a **"Layout B: desk unit"** preset: printer flat on the bottom with the paper out the front, display angled on top, camera on top of the display.

## Enclosure

- Auto-size a rounded box around all parts, with a configurable wall thickness (default 2.5 mm) and an internal clearance (default 3 mm).
- Show the **outer dimensions live** in the UI, so I see the device size.
- Material: translucent (`MeshPhysicalMaterial`, transmission/opacity slider), with a toggle between solid, translucent and hidden.
- Cut-outs, shown as outlined openings: display window, camera lens hole, paper exit slot (60 × 3 mm), USB-C, DC jack, button holes.
- Collision check: highlight any part that intersects another part or the enclosure wall in red, using `Box3` intersection.

## Features

1. **OrbitControls** with damping. Camera presets: Front, Back, Top, Iso, and "User's view" (looking at the display).
2. **Exploded-view slider** (0–100%): parts move outward along their layout offset vectors, and the wires stretch with them (recompute the tube curves).
3. **Click to select:** highlight the part with an outline/emissive tint and open a side panel with its name, dimensions, verified badge, notes and source link.
4. **Hover labels** using `CSS2DRenderer`.
5. **Dimension mode:** draw measurement lines with mm labels on the selected part's bounding box and on the overall enclosure.
6. **Print animation button:**
   - Generate a 384 × 384 test image on a 2D canvas (a simple face or a gradient).
   - Apply **Floyd–Steinberg dithering** in JS.
   - Use the result as a `CanvasTexture`, so the texture is 1-bit black/white with nearest-neighbor filtering.
   - Feed a paper plane out of the printer slot over about 2 s at true scale. 384 px = **48 mm** printed width on 57 mm paper, at 8 px/mm, so a 384-px-tall image is 48 mm tall.
   - Optional: let me drop or upload my own image to dither and "print".
7. **Gesture demo strip:** three buttons (✌️ 👍 ✋) that run the state machine visually. ✌️ shows a 3-2-1 countdown on the display mesh (CanvasTexture), then a "flash"; 👍 runs the print animation; ✋ clears the display.
8. **Camera frustum toggle** showing the IMX500's FOV (72° H × 56° V) out to about 1 m, so I can judge framing for selfie-distance gestures.
9. **Export:** a button to download the current `LAYOUT` + `PARTS` as JSON, and one to export the scene as **GLB** (`GLTFExporter`) for importing into CAD or slicer tools.

## Visual style

- Clean product-render look: `RoomEnvironment` + PMREM for reflections, soft shadows on a ground plane, subtle grid with 10 mm cells.
- Colors:
  - PCBs: green, or black for the camera board.
  - Printer: dark gray plastic.
  - Paper: off-white.
  - Lens: black glass.
- Light and dark UI themes that follow `prefers-color-scheme`. The UI panel is overlaid, collapsible, and usable on a phone. Touch orbit/pinch must work.

## Acceptance criteria

- [ ] Opens from a local file or static host with no build step and no console errors.
- [ ] All parts at true mm scale; unverified dimensions visibly flagged.
- [ ] Changing the display size or layout preset updates the enclosure size and the dimension readout.
- [ ] Exploded view keeps the wires attached.
- [ ] The print animation shows a properly dithered 1-bit image at 48 mm width.
- [ ] Collision highlighting works when I move a part into another.
- [ ] GLB and JSON export both download.

## Out of scope

Real firmware, exact connector pinouts and photorealistic CAD. Primitives are fine; the `modelUrl` swap is the path to real models later.

## Sources for the verified numbers

- DFR0503: https://www.dfrobot.com/product-1799.html and https://wiki.dfrobot.com/dfr0503-en
- Arducam IMX500 for MCU (B0642) datasheet: https://cdn.arducam.com/wp-content/uploads/2026/04/Arducam_B0642_IMX500_AI_Camera_for_MCU_Datasheet.pdf
- Waveshare ESP32-P4-WIFI6: https://www.waveshare.com/esp32-p4-wifi6.htm and https://docs.waveshare.com/ESP32-P4-WIFI6
- Arducam SDK (for SPI/I2C GPIO numbers): https://github.com/ArduCAM/imx500-mcu-sdk
