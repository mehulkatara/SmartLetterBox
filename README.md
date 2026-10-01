# Smart Letterbox

An ESP8266-based mailbox notifier. An infrared (IR) sensor detects a mail drop, an SSD1306 OLED displays the mail count and animations, and a Sinric Pro contact sensor event can trigger an Alexa Routine.

## How It Works

1. The ESP8266 connects to the configured Wi-Fi network and starts Sinric Pro.
2. The IR sensor's digital output is monitored on pin `D5`. A `HIGH` to `LOW` transition is treated as one mail drop.
3. The sketch increments the count, sends an open contact event to Sinric Pro, waits one second, and sends a closed event.
4. The OLED shows a new-mail animation and then returns to the animated count dashboard.
5. Pressing the reset button on `D6` sets the count to zero.

The count is stored only in RAM. It resets to zero every time the ESP8266 restarts, including after a power loss. This sketch does not save the count to flash or another persistent store.

## Parts Required

- ESP8266 development board with the `D` pin labels, such as NodeMCU or Wemos D1 mini
- Digital-output IR sensor module that produces a `LOW` output when it detects mail
- 128 x 64 SSD1306 I2C OLED display, configured for I2C address `0x3C`
- Momentary push button
- Breadboard and jumper wires
- USB cable and a computer for programming the board
- Wi-Fi network with internet access
- Sinric Pro account and an Alexa-enabled device/account for voice announcements

Check the pin labels and voltage requirements for your particular board, sensor, and display before wiring. ESP8266 GPIO pins are 3.3 V logic and are not 5 V tolerant.

## Wiring

Disconnect USB power while wiring. The pin names below use the common NodeMCU/Wemos ESP8266 mapping; check your board's pinout if you have a different model.

| Part | Part pin | ESP8266 connection | Notes |
| --- | --- | --- | --- |
| IR sensor | Digital signal / OUT | `D5` (GPIO14) | Must idle `HIGH` and go `LOW` when mail is detected. |
| IR sensor | VCC | Suitable board supply | Use the sensor's rated supply voltage. Ensure its OUT signal never exceeds 3.3 V. |
| IR sensor | GND | GND | Share ground with the ESP8266. |
| Push button | One terminal | `D6` (GPIO12) | The sketch enables the internal pull-up. |
| Push button | Other terminal | GND | Pressing the button pulls `D6` LOW and resets the count. |
| OLED | SDA | `D2` (GPIO4) | I2C data on common NodeMCU/Wemos boards. |
| OLED | SCL | `D1` (GPIO5) | I2C clock on common NodeMCU/Wemos boards. |
| OLED | VCC | 3V3, if supported by the display | Check the display module's voltage specification. |
| OLED | GND | GND | Common ground. |

The sketch initializes the OLED at I2C address `0x3C`. If your display uses another address, the address in `display.begin(...)` in `SmartLetterBox.ino` must be changed to match it.

## Prepare Arduino IDE

1. Install and open the Arduino IDE from [arduino.cc/en/software](https://www.arduino.cc/en/software).
2. Add ESP8266 board support. In Arduino IDE, open **File > Preferences** and add this URL to **Additional Boards Manager URLs**:

   ```text
   https://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```

   If other board URLs are already present, separate entries with commas.
3. Open **Tools > Board > Boards Manager**, search for `esp8266`, and install the ESP8266 platform published by the ESP8266 Community.
4. Install the libraries from **Sketch > Include Library > Manage Libraries**. Search for and install:
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
   - `SinricPro`
5. `ESP8266WiFi` and `Wire` are included with the ESP8266 board platform; do not install separate copies for this sketch.
6. Open `SmartLetterBox.ino`. Select the board matching your hardware under **Tools > Board**, connect the ESP8266 by USB, and select its serial port under **Tools > Port**.

## Configure Wi-Fi and Sinric Pro

The sketch has five blank configuration values near the top of `SmartLetterBox.ino`:

```cpp
#define WIFI_SSID         ""
#define WIFI_PASS         ""
#define APP_KEY           ""
#define APP_SECRET        ""
#define CONTACT_SENSOR_ID ""
```

Fill them in before uploading:

1. Set `WIFI_SSID` to the exact name of your Wi-Fi network and `WIFI_PASS` to its password. The ESP8266 supports 2.4 GHz Wi-Fi; a 5 GHz-only network will not work.
2. Sign in to Sinric Pro and create or select an account/device configuration for an ESP8266 contact sensor. Follow Sinric Pro's current device setup instructions.
3. Copy the account's `APP_KEY` and `APP_SECRET` into their matching definitions.
4. Copy the contact sensor's device ID into `CONTACT_SENSOR_ID`.
5. Keep the quotation marks around each value. Do not add spaces inside the credentials unless they are part of the value.
6. Do not publish Wi-Fi passwords, app secrets, or device credentials in a public repository. These values are directly in the sketch, so remove them before committing or move them to a private local configuration file and update the sketch to include that file.

For Alexa announcements, link/enable the Sinric Pro skill in the Alexa app, discover the contact sensor, then create an Alexa Routine whose trigger is the sensor opening. Choose the desired announcement/action. The sketch reports the sensor open for about one second and then reports it closed. Alexa and Sinric Pro menu names can change, so use their current setup instructions if the labels differ.

## Upload and First Test

1. Verify the wiring and make sure no ESP8266 input can receive a 5 V signal.
2. Enter the Wi-Fi and Sinric Pro values, then save the sketch.
3. In Arduino IDE, select the correct ESP8266 board and serial port.
4. Click **Verify** (the checkmark) and resolve any missing-library or compile errors.
5. Click **Upload** (the right-arrow) and wait for the IDE to report completion.
6. Open **Tools > Serial Monitor**, set its speed to `115200` baud, and reset the board if needed. The sketch prints dots while connecting to Wi-Fi and `WiFi Connected!` after connection.
7. Confirm the OLED starts and displays the mailbox dashboard after the reset message.
8. Trigger the IR sensor once. The serial monitor should report `New mail detected! Total: 1`, the OLED should show the new-mail animation, and the count should increment.
9. Press the reset button. The OLED should briefly show `MAIL COUNT RESET!` and then show a count of zero.
10. If Alexa is configured, test the open-contact Routine after confirming the device is online in Sinric Pro.

## Using the Mailbox

- Mount and aim the IR sensor so a letter reliably changes its output from `HIGH` to `LOW` as it passes. Secure it so ordinary movement or ambient light does not cause false triggers.
- One `HIGH` to `LOW` transition counts as one mail drop. The sensor must return to `HIGH` before another drop can count.
- The dashboard shows the count, an animated envelope, and a moving scan line.
- The reset button clears the current count. Power cycling also clears it.
- The alert and animations contain delays. While those routines run, the sketch does not continuously sample the sensor or handle Sinric Pro traffic. Very fast repeated drops or connection events during those pauses may be missed or delayed.

## Troubleshooting

| Symptom | Things to check |
| --- | --- |
| Upload fails or board is not listed | Confirm ESP8266 board support is installed, the correct board and port are selected, and the USB cable supports data. |
| `WiFi Connected!` never appears | Check the SSID/password, use a 2.4 GHz network, confirm signal/internet availability, and check that the board has adequate power. The current sketch waits indefinitely for Wi-Fi during startup. |
| OLED is blank | Check SDA/SCL, power, and ground; confirm the display is I2C and 128 x 64; verify address `0x3C`. The current sketch prints an allocation error to Serial if display initialization fails but continues running. |
| IR sensor never counts | Check that its OUT pin is connected to `D5`, share ground, and use a sensor whose output goes LOW on detection. Check the sensor's output voltage and adjust its sensitivity/position. |
| Counts happen without mail | Reposition or shield the sensor from ambient light/movement and adjust its sensitivity. Ensure it returns HIGH between mail drops. |
| Reset button does not work | Check that the button connects `D6` to GND when pressed and is wired to the correct button terminals. |
| Sinric Pro or Alexa does not respond | Check the app key, app secret, contact sensor ID, internet connection, device online status, Alexa skill/device discovery, and that the Routine triggers when the contact sensor opens. |
| Count disappears after restart | This is expected: the sketch resets the count to zero on every boot and does not persist it. |

## Project Files

```text
SmartLetterBox/
├── SmartLetterBox.ino   # ESP8266 firmware
└── README.md            # Setup, wiring, and operating instructions
```