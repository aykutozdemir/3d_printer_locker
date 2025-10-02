# 3D Printer Door Locker & LED Brightness Controller

This project adds **secure dual‑door locking** and **internal LED brightness control** to a Flashforge Adventurer 5M Pro (or similar) 3D printer. It manages front and top doors with 12 V solenoid locks, mirrors/toggles the printer’s built‑in light, provides a child‑lock for the power button and touchscreen, and offers clear buzzer/LED feedback.

---

## 1) Hardware Overview

**Controller & Power**

* Arduino Nano (5 V)
* 24 V printer rail → **12 V regulator** (for solenoids, LED strip, MOSFET module)
* Buzzer (active preferred)

**Inputs**

* 4‑digit membrane keypad (GND common + 4 signal lines; use internal pull‑ups)
* Yellow single membrane button (toggle + dimming)
* Door switches: 1× front, **2× in series** for top (treated as one input)
* Motherboard LED state input (to mirror stock/phone LED control)
* Device Running Sense input (from motherboard)

**Outputs**

* **3× MOSFET channels** (low‑side):

  * CH1 → **LED strip (12 V, PWM brightness)**
  * CH2 → **Front door solenoid (12 V)**
  * CH3 → **Top door solenoid (12 V)**
* Bi‑color **Red/Green LED** (status)
* **Child‑lock outputs (2)** → *5 V = enabled* / *0 V = disabled*

  * Printer power button enable
  * Touchscreen enable

**Notes**

* Solenoids: **fail‑secure** (no power = locked), **power ~5 s to release**
* All grounds are **common** (Nano, MOSFETs, loads, inputs)

---

## 2) Pin Assignments (Suggested)

### Inputs

| Function                   | Nano Pin | Notes                              |
| -------------------------- | -------- | ---------------------------------- |
| Keypad Line 1              | **D2**   | Internal pull‑up                   |
| Keypad Line 2              | **D3**   | Internal pull‑up                   |
| Keypad Line 3              | **D4**   | Internal pull‑up                   |
| Keypad Line 4              | **D5**   | Internal pull‑up                   |
| Yellow Button              | **D6**   | Internal pull‑up, short/long press |
| Motherboard Light Sense    | **D7**   | Use divider/optocoupler if >5 V    |
| Front Door Switch          | **D8**   | Internal pull‑up                   |
| Top Door Switches (series) | **D9**   | Internal pull‑up                   |
| Device Running Sense       | **A4**   | Internal pull-up, LOW when not running |

### Outputs

| Function                 | Nano Pin | Notes                       |
| ------------------------ | -------- | --------------------------- |
| LED Strip MOSFET (PWM)   | **D10**  | Hardware PWM                |
| Front Solenoid MOSFET    | **D11**  | Digital                     |
| Top Solenoid MOSFET      | **D12**  | Digital                     |
| Child‑Lock: Power Button | **D13**  | Digital (or via NPN driver) |
| Child‑Lock: Touchscreen  | **A0**   | Digital (or via NPN driver) |
| Status LED (Red)         | **A1**   | With series resistor        |
| Status LED (Green)       | **A2**   | With series resistor        |
| Buzzer                   | **A3**   | Active buzzer preferred     |

**PWM capable pins**: D3, D5, D6, D9, **D10**, D11 → D10 is reserved for LED PWM.

---

## 3) Logic & Behavior

### 3.1 Default State (Power‑Up)

* System starts **LOCKED** (solenoids off)
* **Child‑lock active** → power + touchscreen **disabled (0 V)**
* Status LED = **Red**
* LED strip state restored from EEPROM (on/off + brightness)

### 3.2 Internal Lighting (Yellow Button + Motherboard Input)

* **Short press (Yellow)** → toggle LED **on/off**
* **Long press (Yellow)** → start dimming when light is ON:

  * While held → **+10% per second** brightness stepping, ping‑pong between 0% ↔ 100%
  * **Short press while dimming** → stop dimming and **save current brightness to EEPROM**
  * Note: When LED is OFF, long press does not start dimming
* **Motherboard Light Input** mirrors printer/app command when not in manual override:

  * Input ON → LED ON at saved brightness
  * Input OFF → LED OFF
* Pressing Yellow (toggle or dim) enables **manual override**, during which motherboard input is ignored until the next power cycle or explicit toggle from Yellow.

### 3.3 Keypad: Password + Options

* Passwords are **always 4 digits**
* **Factory default password: `1234`**
* **Digit timeout**: 3 s between key presses (resets the entry if exceeded)
* After entering 4 digits:

  * **Correct** → *accept beep* and wait for **option**
  * **Incorrect** → *error beep*, reset

**Options after correct password**

* `+1` → **Top door unlock** (solenoid ~5 s)
* `+2` → **Front door unlock** (solenoid ~5 s)
* `+3` → **Both doors unlock**
* `+4` → **Child‑lock disable** (starts a 1-minute timeout)
* **Short press Yellow Button** → **Password change mode**

**Password Change Mode (Yellow Button)**

* Enter new 4‑digit password **twice**
* Match → store in EEPROM, confirmation beep
* Mismatch → error beep, old password kept

**Child Lock Disabled Mode**

* When child lock is disabled, the keypad has special functions:
    * `1 (long press)` → **Re‑engage child lock immediately**.
    * `4 (short press)` → **Reset the 1‑minute timeout**.

### 3.4 Auto‑Locking

* If a door **was opened and then closed** → **lock immediately** upon closure (magnets re‑engaged)
* Magnets re‑engage **~1.5 s after a door opens** (safety hold‑off) or **~100 ms after a door closes**
* Child lock will be enabled automatically after **1 minute** of being disabled (or sooner via keypad 1 long‑press)

### 3.5 Unauthorized Access

* If any door switch indicates opening **without a valid unlock**:

  * Buzzer sounds **continuous angry alarm** until the door is closed
  * System remains **LOCKED**

### 3.6 Device Running Detection
* The system monitors a `Device Running Sense` input (A4), HIGH = RUNNING, LOW = STOPPED.
* Child‑lock behavior while engaged:
  * RUNNING → both power button and touchscreen are locked.
  * STOPPED → power button is enabled; touchscreen remains locked.
* Other functions (door control, lights, keypad) remain available.

---

## 4) Indicators & Sounds

**Status LED**

* **Red** = LOCKED
* **Green** = UNLOCKED
* **Alternating Red/Green** = Child lock disabled (1‑minute window active)

**Buzzer Patterns (Suggested)**

* Key press → *short beep*
* Correct password → *double short beep*
* Incorrect password → *long low beep*
* Option selected (1–4/0) → *medium beep*
* Password‑change mode entered → *triple short beep*
* Password saved → *long rising tone*
* Unauthorized door open → *fast repeating beep* until closed

---

## 5) Timing Parameters (Effective)

| Event                       | Value       | Notes                         |
| --------------------------- | ----------- | ----------------------------- |
| Magnet re‑engage (after open) | **~1.5 s** | Hold‑off before re‑engage     |
| Magnet re‑engage (after close) | **~100 ms** | Re‑engage after closure      |
| Keypad digit timeout        | **3 s**     | Between digits                |
| Yellow long‑press threshold | **~1 s**    | Start brightness stepping     |
| Brightness step             | **10% / s** | Ping‑pong 0↔100%              |
| Child lock timeout          | **1 min**   | Auto-enables child lock       |

---

## 6) EEPROM Data

* Last LED **brightness** (0–100 %) — address 0
* Last LED **on/off** state (0=OFF,1=ON) — address 1
* Current **password** (factory: `1234`) — addresses 17..20, with magic at 16

---

## 7) Factory Reset

* **Trigger (Serial)**: send `factoryreset`
* **Actions**:

  * Reset password → **`1234`** (and set EEPROM magic)
  * LED brightness → **100 %**; LED state → **OFF**
  * Re‑engage child‑lock, set status LED to **LOCKED**
* Use `password reload` to force a runtime reload of the PIN if needed.

---

## 8) Serial Commands

| Command                  | Description |
| ------------------------ | ----------- |
| `help`                   | Show available commands |
| `status`                 | Show system status summary |
| `led <state>`            | Control LEDs: `locked`, `unlocked`, `to_be_locked` |
| `ledstate <name>`        | Set LED state: `locked`, `unlocked`, `to_be_locked`, `to_be_opened`, `child_unlocked` |
| `light <on|off|toggle>`  | Control internal light |
| `childlock <engage|release|status|reset>` | Engage/release, show status, or reset 1‑min timeout |
| `password <show|reload|set 1234>` | PIN ops; `reload` re‑reads EEPROM; use keypad to change PIN |
| `factoryreset`           | Reset EEPROM to defaults (dangerous) |

---

## 9) Protection & Component Values

### 8.1 Solenoids (12 V)

* **Flyback diode (each coil)**: **SS34** (3 A, 40 V Schottky) or **1N5819** (1 A, 40 V)

  * Cathode → **+12 V**, Anode → **MOSFET/drain**
* *(Optional fast release)* **TVS**: **SMBJ16A** across coil (or RC snubber 100 Ω/1–2 W + 100 nF ≥50 V film)
* Local bulk cap near drivers: **220–470 µF / 25 V**

### 8.2 LED Strip (12 V, PWM)

* If the MOSFET module lacks gate network (discrete build only):

  * Gate series: **100–220 Ω**; Gate pulldown: **100 kΩ**
* **12 V rail TVS**: **SMBJ16A** across 12 V–GND

### 8.3 12 V Supply Rail

* Reverse polarity: **SS54** (5 A Schottky) in series (or ideal‑diode P‑MOSFET stage)
* Bulk: **470–1000 µF / 25 V** electrolytic + **100 nF** ceramics near modules

### 8.4 Child‑Lock Outputs (5 V)

* If high‑impedance inputs (<5 mA): drive from Nano; add **220–330 Ω** series for protection
* If you prefer open‑collector logic: **2N2222** NPN, **1 kΩ** base resistor; add **10 kΩ pull‑up** to 5 V on the line

### 8.5 Status LED (Bi‑Color)

* One resistor **per color**: **330–680 Ω** at 5 V (680 Ω is comfortable)
* Common cathode → GND (pin HIGH = on) or common anode → +5 V (pin LOW = on)

### 8.6 Buzzer

* **Active 5 V buzzer** (<20 mA): direct from Nano (optionally **220 Ω** series)
* **Passive/higher current**: 2N2222 driver + **1 kΩ** base resistor

### 8.7 Keypad & Buttons

* Using internal pull‑ups is sufficient; optional extras:
* External pull‑ups **10kΩ** and/or **100 nF** to GND for hardware debounce
* Series to MCU pins **100–220 Ω** for ESD protection

### 8.8 Motherboard Light‑Sense Input

* If it’s an **open‑collector to GND**: add **10 kΩ pull‑up to 5 V** + **220 Ω** series to the pin
* If it can be **24 V**: use a divider **100 kΩ : 27 kΩ** → ~4.8 V at the pin (optional 5.1 V zener/TVS clamp), or use **PC817 optocoupler** (24 V side ~2.2 kΩ series)

---

## 10) Minimal BOM (Core Protection)

| Use                     | Part                       | Qty       |
| ----------------------- | -------------------------- | --------- |
| Solenoid flyback        | **SS34** (3 A, 40 V)       | 2         |
| (Optional) coil TVS     | **SMBJ16A**                | 2         |
| 12 V rail TVS           | **SMBJ16A**                | 1         |
| Reverse‑polarity        | **SS54** (5 A Schottky)    | 1         |
| Bulk electrolytic       | **470–1000 µF / 25 V**     | 2–3       |
| Decoupling              | **100 nF ceramics**        | 10+       |
| LED resistors           | **680 Ω**                  | 2         |
| Child‑lock pull‑up      | **10 kΩ**                  | 2 (if OC) |
| NPN drivers             | **2N2222** + **1 kΩ base** | as needed |
| Divider for Light‑Sense | **100 kΩ + 27 kΩ**         | 1 set     |
| Optocoupler (alt)       | **PC817** + **2.2 kΩ**     | 1         |

---

## 11) Wiring & Layout Tips

* Keep **solenoid current loops short**; place flyback diodes **at the coil**
* Use **star ground**: separate high‑current returns and meet at one point
* Keep PWM gate traces short; avoid parallel runs with sensitive inputs
* Debounce in **hardware + software** for rock‑solid behavior

---

## 12) Versioning & Defaults

* Default password: **1234**
* Default timings: **5 s unlock**, **10 s auto‑lock**, **3 s keypad timeout**, **10%/s dim step**
* Factory reset: **Hold Yellow ≥5 s on power‑up**

---

## 13) License

Add your preferred open‑source license here (e.g., MIT) if you plan to publish.