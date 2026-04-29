# Axis Spotify Connect ACAP (WIP)

> ⚠️ **WORK IN PROGRESS**: This project is currently in active development. It is functional (audio streaming works via PipeWire on Axis OS 12) but has not been extensively tested on all firmware versions or devices. Use at your own risk.

An ACAP (Axis Camera Application Platform) application that turns an Axis network audio device (like the Axis I8116 intercom) into a Spotify Connect receiver.

This application cross-compiles the Rust-based [librespot](https://github.com/librespot-org/librespot) client for the ARM architectures used in Axis cameras and integrates it securely into Axis OS 12 using the PipeWire audio backend.

## Features
- **Spotify Connect integration**: Stream music directly from the Spotify app to your Axis intercom/speaker.
- **Embedded Web Interface**: Modern, dark-themed GUI accessible directly from the camera's IP to view playback status and debug logs.
- **Optimized for Axis OS 12**: Uses the required `pipewire` group and ALSA layer to ensure compatibility with modern Axis firmwares.
- **Lightweight**: Extensively stripped and link-time-optimized binary to fit within the small flash memory of embedded devices (~2.7MB `.eap` package).

## Architectures
Axis cameras run on different processors. This project provides builds for the two primary architectures:
1. **aarch64**: For newer devices with ARTPEC-8 or Ambarella CV25 processors (e.g., Axis I8116).
2. **armv7hf**: For older devices with ARTPEC-6 or ARTPEC-7 processors.

## Installation

You can find the pre-built `.eap` files in the `builds/` directory of this repository.

1. Download the correct `.eap` file for your camera's architecture:
   - `Spotify_Connect_1_0_0_aarch64.eap`
   - `Spotify_Connect_1_0_0_armv7hf.eap`
2. Upload the `.eap` file via the Axis Web interface (`Apps` tab).
3. Start the application and open the App interface to configure the device name and view logs.

## Building from source

You can compile the ACAP packages yourself using the provided Dockerfile.

**For aarch64 (ARTPEC-8 / Ambarella CV25):**
```bash
docker build --build-arg ARCH=aarch64 -t spotify-acap-aarch64 .
docker create --name temp-aarch64 spotify-acap-aarch64
docker cp temp-aarch64:/opt/app/Spotify_Connect_1_0_0_aarch64.eap ./builds/
docker rm temp-aarch64
```

**For armv7hf (ARTPEC-6 / ARTPEC-7):**
```bash
docker build --build-arg ARCH=armv7hf -t spotify-acap-armv7hf .
docker create --name temp-armv7hf spotify-acap-armv7hf
docker cp temp-armv7hf:/opt/app/Spotify_Connect_1_0_0_armv7hf.eap ./builds/
docker rm temp-armv7hf
```

## Technical Details

- Relies on `librespot` for the Spotify Connect protocol.
- Includes a C wrapper that interfaces with the `axparameter` API to manage settings like `DeviceName` and `Bitrate`.
- Employs a custom Bash script and CGI endpoints to poll playback events and serve them to the web UI.

## License
MIT License. See the [LICENSE](LICENSE) file for more information.
