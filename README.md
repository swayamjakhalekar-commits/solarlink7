# SolarLink7

Automated solar-tracker prototypes at **THM Friedberg (DE)** and **CESI Arras (FR)**,
networked over MQTT into a shared **virtual power plant (VPP)** dashboard. Both sites run
identical prototypes that can be monitored and remote-controlled across one broker.
Built for **COIL 2026 · Course M1202 (Project Management)**.

## Status
Prototypes built and demonstrated at both sites during the project week (27–29 May 2026).
Cross-site MQTT control and the Node-RED VPP dashboard are functioning.
Final presentation: **12 June 2026**.

## Team
| Site | Members |
|------|---------|
| THM Friedberg | Swayam Jakhalekar (communicator), Shanmugaraja Senthil (organizer), Shantanu Shende |
| CESI Arras | Gabriel Lenoir (communicator), Willyam Okondza (organizer), Edouard Cossin |
| Supervisors | Prof. Dr. M. Arndt (THM), Prof. L. Costelle (CESI) |

## What it does
Each node reads panel voltage/current via an INA219 sensor, positions the panel with two
servos (automatic maximum-power search, or manual override from the dashboard), shows live values
on a local LCD (THM), and publishes telemetry over MQTT. The Node-RED dashboard shows both
sites side by side and can command either one — that two-site, remote-controllable view is
the VPP demo.

## Repository layout
```
SolarLink7/
├── Arduino Codes/
│   ├── Team7_Friedberg_FinalCode/   # THM firmware
│   ├── Team7_Arras_FinalCode/       # CESI firmware
│   ├── Servos_ZeroPos/              # servo zero/home reference helper
│   └── Servos_MovIndetifier/        # servo direction-of-motion helper
├── 3D_PrintingParts/                # STL parts (base, arm, panel supports, ...)
├── Electronics/                     # Circuit Wiring.png (INA219 + Arduino)
├── Node-Red/                        # Team7_FinalFlow.json + MQTT/Node-RED setup PDF
├── Project Planning and Report/     # WBS, Gantt, competence analysis (PDF)
├── Team and Project/                # brief, poster, team roles/rules/comms plan
├── Measurement/                     # I-V curves, INA219 logs (to be added)
├── Media/                           # photos/video — see Google Drive (not committed)
└── Reflections/                     # weekly reflection journals (to be added)
```

## How tracking works
Tracking is a **two-axis maximum-power search** — measurement-based, not astronomical and
not LDR-based. The panel physically moves and the INA219 power reading (voltage × current)
decides where it settles.

- **Startup:** both servos home, then a full sweep — horizontal 0–180° in 5° steps, then
  vertical, settling 700 ms at each step and recording power. The panel moves to the angle
  that produced the most power. Horizontal is searched first, then vertical.
- **Auto mode:** every 2 minutes the node re-scans only a narrow window around the current
  best (horizontal ±30°, vertical ±20°) rather than a full sweep, so it follows the sun's
  drift without a costly full search each cycle.
- **Manual mode:** a dashboard `control/angle-h|v` command switches the node to manual and
  drives to the requested angle; `control/mode` toggles auto/manual.

Both sites run the identical algorithm; the THM node additionally shows live values on a
16×2 I2C LCD.

## System & MQTT
- **Broker:** `broker.hivemq.com:1883` (public, no auth)
- **Topic base:** `cesithmcoil2025/Team7/<SITE>/` where `<SITE>` is `Friedberg` or `Arras`
- **Telemetry:** `panel-voltage`, `panel-current`, `panel-power`, `position/angle-h`,
  `position/angle-v`, `status`
- **Control:** `control/mode`, `control/angle-h`, `control/angle-v`

The dashboard subscribes to each site's `…/#` wildcard, splits the stream per site, and
routes each field to a widget; control widgets publish back on the `control/...` topics.
See `Node-Red/` for the flow export and setup walkthrough.

## Stack
- **Hardware:** Arduino UNO R4 WiFi · INA219 · 2× SG90 servos · 16×2 I2C LCD (THM) · panel 6.4 V / 360 mA
- **Tracking:** two-axis maximum-power search (INA219 power feedback) — no LDRs, no lat/lon
- **Dashboard:** Node-RED
- **CAD:** TinkerCAD → STL

## Links
- Google Drive (deliverables, media): _add link_
- Trello (task board): _add link_

## License / use
Coursework artifact for M1202. Not for redistribution outside the course.