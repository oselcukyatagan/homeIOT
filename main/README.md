# homeIOT
IOT applications for my home

The current system features:

* **Time-Based Position Control:** Maps  travel time to a 0–100% scale without physical limit switches.
* **Relay Interlocking:** Enforces a 1-second delay during direction changes to prevent back-EMF spikes and contact welding.
* **Mechanical Settling Delay:** Includes a 50ms pause after direction selection to ensure relay armatures are seated before applying power.
* **Boundary Protection:** Hard-coded limits prevent the position counter from exceeding 100 or falling below 0.
* **SinricPro Integration:** Provides remote control for blinds, RGB color selection, and power state via WebSocket.
* **Inverted RGB Logic:** Specifically configured for common-anode LED strips ().
* **Dual Mode Control:** Seamlessly switches between static color control and audio-reactive Music Mode.
* **Advanced Audio DSP:** * **Dynamic DC Offset:** Real-time tracking of microphone bias to eliminate thermal drift.
* **Envelope Follower:** Asymmetric attack/decay algorithm to distinguish sustained audio from electrical EMI.
* **IIR Low-Pass Filter:** Smooths raw microphone data for fluid LED transitions.
* **Auto-Off Timer:** Polls audio for 5 seconds every minute; shuts down LEDs after 15 minutes of silence.
* **Web Dashboard:** Local HTTP server provides secondary control for blinds and music mode selection.

<img width="1916" height="878" alt="image" src="https://github.com/user-attachments/assets/3c140190-5b17-4957-866a-ddc58cd7e458" />

<img width="1418" height="551" alt="image" src="https://github.com/user-attachments/assets/aee32597-78cc-4ff5-b98e-c95eec871d36" />
