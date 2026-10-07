# Handoff prompt: three.js viewer for the X4 sticker printer

Copy everything below the line into a new session.

---

## Task

Build an interactive **three.js 3D viewer** of a hardware device I'm about to build: a **single handheld enclosure that combines an Xteink X4 e-paper reader with a thermal sticker printer**.
- The X4's e-paper screen is the user interface. It browses **preset stickers stored on its microSD card**, previews them, and sends the chosen one to print.
- A small controller board inside the case receives the image wirelessly and drives the printer.

I'll use the viewer to plan the case layout before 3D-printing it.

Deliver a **single self-contained `index.html`**. Load three.js and its addons (`OrbitControls`, `CSS2DRenderer`, `GLTFExporter`, `GLTFLoader`, `RoomEnvironment`) as ES modules from `cdn.jsdelivr.net/npm/three@<latest>` through an import map. Don't use a build step. Use **1 scene unit = 1 mm**.

## How the device works (for context)

1. **Xteink X4** (ESP32-C3, 4.3" 480×800 e-paper, 220 PPI) runs custom firmware based on the open-source CrossPoint Reader. A "Stickers" app:
   - lists `/stickers/**/*.bmp` on the X4's own microSD card. These are 1-bit, pre-dithered, ≤ 384 px wide.
   - shows a pixel-for-pixel preview (384 px ≈ 44 mm on screen vs 48 mm printed).
   - on the print button, sends the 1-bit image over **ESP-NOW** to the controller. 384 × 800 px is ≈ 38 KB, which takes about a second.
2. **Seeed XIAO ESP32S3 Sense** inside the case receives the image and prints it over TTL serial using the ESC/POS raster command `GS v 0`. Its Sense camera is optional, for photo stickers previewed on the X4.
3. **DFRobot DFR0503 V2.0 thermal printer** prints on **57 mm × 30 mm Ø sticker rolls**, at 384 dots = 48 mm print width.
4. **Power:** a USB-C PD trigger board set to **15 V** feeds the printer (rated 9–24 V, 0.5–2.5 A). A small buck converter gives 5 V to the XIAO. The X4 keeps its own 650 mAh battery and charges through its own USB-C port.

## Components

Put all dimensions in one `PARTS` config object at the top of the file. Each part gets a `verified: true|false` flag. Draw unverified parts with a dashed outline edge and an "unverified size" badge in the info panel. Each part also gets an optional `modelUrl`: if set, load that GLB in place of the simple shapes.

| id | Part | Dimensions (mm) | Verified | Modeling notes |
|---|---|---|---|---|
| `x4` | Xteink X4 e-reader | **114 × 69 × 5.9**, 77 g | ✅ (published spec) | Slab with rounded corners (radius ≈ 6, unverified). Screen: 4.3" active area ≈ 56 × 94 (unverified; derive from 480×800 at 220 PPI: 55.4 × 92.4). Show the screen as a CanvasTexture: the e-paper UI and the sticker preview. Features to keep reachable (positions **unverified**; mark them as "measure your unit"): USB-C port, microSD slot (deeply recessed, needs a pin to eject), power button, page-turn buttons. |
| `printer` | DFRobot Embedded Thermal Printer V2.0 (DFR0503-EN) | Overall **82 × 58 × 44**; panel/install body **77 × 53 × 42** | ✅ (DFRobot spec) | Front bezel flange 82 × 58 with the body behind it. Paper slot in the front face, 58 wide. Hinged paper door. Inside: a paper roll cylinder **57 long × 30 Ø**. Power and TTL connectors on the back face (position unverified). |
| `xiao` | Seeed XIAO ESP32S3 Sense | **21 × 17.8** PCB; ≈ 15 tall with the Sense camera board stacked | ✅ footprint / ❌ stack height | Tiny PCB, USB-C on one short edge. Optional camera module on top: toggle it, and give it a lens hole in the case. |
| `pd` | USB-C PD trigger board (15 V) | ≈ 25 × 12 × 6 | ❌ | Its USB-C port is the device's main power input; give it a case cutout. |
| `buck` | 15 V → 5 V buck module (Mini-360 class) | ≈ 22 × 17 × 4 | ❌ | |
| `paper` | Sticker strip | 57 wide × 0.1 | ✅ | Animated (see Features). |
| `button` | Optional 12 mm tactile "feed" button | 12 × 12 × 7 | ❌ | Toggle. |

**Hard rule to show in the UI:** check the printer's label before wiring. A **V1 board is rated 5–9 V, and 15 V would destroy it**. Only the V2.0 (9–24 V) takes the 15 V PD input.

## Wiring (draw as colored tube curves)

Use `CatmullRomCurve3` + `TubeGeometry`, about 1 mm diameter. Color by function, add a legend, and toggle each group.

| From | To | Color |
|---|---|---|
| PD board V+ (15 V) | Printer power + **and** buck input | red |
| PD board GND | Printer power GND, buck GND, XIAO GND (common ground) | black |
| Buck 5 V out | XIAO 5V pin | orange |
| XIAO D6 / GPIO43 (TX) | Printer TTL RX | yellow |
| Printer TTL TX | **leave unconnected**; dangling stub labeled "printer TX may be 5 V, ESP32 pins aren't 5 V tolerant" | gray dashed |
| XIAO D0 / GPIO1 | Feed button → GND | green |
| X4 ⇄ XIAO | **No wire:** draw an animated dotted "ESP-NOW" arc between them | cyan dotted |

## Layout presets (store in a `LAYOUT` object, switchable in the UI)

**A — "Instant camera" (portrait).** X4 in portrait on the front face. The printer sits behind the X4's top half, rotated so its paper slot points **up** and stickers come out the top edge. The XIAO, PD board and buck sit in the space below the printer. Target outer size ≈ **92 × 75 × 125 mm** (W × D × H).

**B — "Label maker" (landscape).** X4 in landscape on the front face. The printer sits behind it with the paper slot pointing **up**. Electronics go in the side bay next to the printer, using the X4's extra length (114 vs 82). Target outer size ≈ **120 × 72 × 78 mm**.

For both presets:
- The X4 sits in a **removable dock pocket**. It slides in from the top or side and is held by a 1.5 mm lip around the bezel, so it can still be used as an e-reader.
- Leave its USB-C port, microSD slot and buttons reachable through cutouts.
- Leave a 1 mm gap around the X4 for print tolerance.

## Enclosure

- Auto-size a rounded box around all parts, with a configurable wall thickness (default 2.5 mm), internal clearance (default 2 mm) and corner radius (default 6 mm).
- Show **live outer dimensions** in the UI.
- Split the box into a **front shell** (holding the X4 pocket) and a **back shell** (printer cradle and electronics bay). An exploded view separates them.
- Material: translucent `MeshPhysicalMaterial`, with toggles for solid, translucent and hidden. The X4 pocket is a separately colored part.
- Cutouts, shown as outlined openings:
  - X4 screen window (shows the full active area)
  - paper exit slot (**60 × 3 mm**) with a **tear bar**: a thin serrated edge just outside the slot
  - printer paper door access: a hinged lid on the back shell, so the 30 mm roll can be swapped
  - PD USB-C port, X4 USB-C, X4 microSD, X4 buttons
  - optional camera lens hole and feed button
- Collision check: highlight any part that intersects another part or a shell wall in red, using `Box3`.

## Features

1. **OrbitControls** with damping. Camera presets: Front, Back, Top, Iso, "In hand" (slight tilt, as if held).
2. **Exploded-view slider** (0–100%): the shells and parts move apart along their layout offset vectors, and the wires re-route with them.
3. **Click to select** a part: outline highlight plus a side panel with its name, dimensions, verified badge, notes and source link.
4. **Hover labels** using `CSS2DRenderer`.
5. **Dimension mode:** mm measurement lines on the selected part and the overall enclosure.
6. **Simulated X4 sticker UI:** draw on the X4 screen texture with canvas 2D, in pure black/white with nearest-neighbor filtering.
   - A sticker list with a few generated presets: "Date stamp", "Habit grid 7×5", "Mood: ☐☐☐☐☐", "Today I…" with ruled lines, a heart doodle.
   - **Up / Down / Print** buttons in the UI, standing in for the X4's page buttons.
   - The preview shows the selected sticker at true scale (384 px over 44 mm on screen).
   - Optional: drop in your own image, then resize to 384 wide, **Floyd–Steinberg dither** in JS, and add it to the list.
7. **Print animation:** on Print, the ESP-NOW arc pulses, then a paper strip with the same 1-bit texture feeds out of the top slot over about 2 s, at true scale (8 px/mm, so 384 px = 48 mm).
8. **Export:**
   - `LAYOUT` + `PARTS` as JSON
   - the whole scene as **GLB** (`GLTFExporter`)
   - **each enclosure shell separately as GLB/STL** (`STLExporter`), so I can start the 3D-printed case from them

## Visual style

- Clean product-render look: `RoomEnvironment` + PMREM, soft shadows on a ground plane, 10 mm grid.
- Colors:
  - X4 body: white or black (toggle).
  - Screen: paper-white e-ink texture.
  - Printer: dark gray.
  - PCBs: green or black.
  - Sticker paper: off-white.
- Light and dark UI themes that follow `prefers-color-scheme`. A collapsible overlay panel. Works on a phone with touch orbit/pinch.

## Acceptance criteria

- [ ] Opens from a local file or static host with no build step and no console errors.
- [ ] All parts at true mm scale; unverified dimensions visibly flagged.
- [ ] Switching layout preset A/B updates the enclosure and its dimension readout.
- [ ] Exploded view keeps the wires attached and separates the front and back shells.
- [ ] The simulated X4 UI scrolls the presets, and Print produces a correctly dithered 48 mm-wide strip from the top slot.
- [ ] Collision highlighting works.
- [ ] JSON, scene GLB and per-shell STL exports all download.

## Out of scope

The real X4 firmware, the ESP-NOW protocol implementation, exact connector pinouts and photorealistic CAD.

## Sources

- Xteink X4 specs (114 × 69 × 5.9 mm, 77 g, 650 mAh, microSD, USB-C): https://pocketink.io/devices/x4/
- CrossPoint Reader firmware: https://github.com/ivancernja/crosspoint-reader
- DFR0503: https://www.dfrobot.com/product-1799.html and https://wiki.dfrobot.com/dfr0503-en
- XIAO ESP32S3 Sense: https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/
