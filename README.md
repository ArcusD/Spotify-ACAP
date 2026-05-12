# Spotify Connect ACAP for Axis Speakers (Audio Manager Edge)

This project provides a robust Spotify Connect receiver tailored for Axis network speakers running **Audio Manager Edge** (e.g., the Axis C1210-E). It seamlessly integrates into the Axis ecosystem by registering via D-Bus, allowing you to manage Spotify audio just like any other native audio source (paging, SIP, etc.) with proper priority and zone routing.

## Features
- **Native Audio Manager Edge Integration**: Dynamically registers as an eligible audio source via D-Bus (`com.axis.AudioConf.Application`).
- **Dynamic ALSA Routing**: Automatically discovers the correct virtual audio loopback (e.g., `loopbacksink_6`) created by Axis and pipes audio directly into it, fully bypassing hardware strictness and sample rate mismatches.
- **Robust Buffer Negotiation**: Uses `aplay` under the hood to ensure perfect communication with Axis ALSA loopbacks, completely preventing the notorious `Invalid argument (22)` ALSA crashes.
- **Hardware Agnostic**: Because the ACAP requests its ALSA sink dynamically from the speaker, this application should theoretically run flawlessly on any Axis speaker supporting Audio Manager Edge.

## Prerequisites
- **Hardware**: An Axis network speaker (e.g., C1210-E) running Audio Manager Edge.
- **Software**: Docker Desktop installed on your build machine.

## Building the ACAP (.eap file)

The project includes a `Dockerfile` that cross-compiles `librespot` (the core Spotify Connect client) for the ARM architecture of your Axis speaker using the official Axis ACAP SDK.

1. Build the Docker image for your speaker's architecture. For the C1210-E, use `armv7hf` (ARTPEC-7):
   ```bash
   docker build --build-arg ARCH=armv7hf -t spotify-connect-builder-armv7hf .
   ```
   *(Note: For ARTPEC-8 devices like the I8116-E, use `ARCH=aarch64`)*

2. Extract the compiled `.eap` package from the container:
   ```powershell
   $id = docker create spotify-connect-builder-armv7hf
   docker cp "$($id):/opt/app/Spotify_Connect_1_0_0_armv7hf.eap" "./Spotify_Connect_armv7hf.eap"
   docker rm $id
   ```

## Installation & Configuration

1. **Install the App**: Log into the web interface of your Axis speaker. Navigate to **Apps** and upload the `.eap` file you just built.
2. **Configure**: Click on the Spotify Connect app in the list. Here you can configure:
   - **DeviceName**: The name that will show up in your Spotify app (e.g., "Kantine Speaker").
   - **Bitrate**: Default is 320 kbps.
   - **AudioSourceName**: The internal name used by Audio Manager Edge (leave default).
3. **Start**: Start the app. 
4. **Audio Manager Edge**: Go to Audio -> Audio Manager Edge. You will now see your Spotify Connect app listed as an eligible audio source!
5. **Play**: Open Spotify on your phone or PC, select your Axis speaker from the Devices menu, and hit play!

## Troubleshooting

- **No Sound?**: Ensure the app is actually started in the Apps menu. Verify in Audio Manager Edge that the volume is not muted and the app has the correct priority.
- **Forbidden - Blocked due to brute force protection**: This is a standard Axis security feature. If you refresh the Axis web UI too fast, you will be temporarily blocked. Wait 5 minutes or physically reboot the speaker (PoE unplug).
- **Checking Logs**: You can view the internal logs by SSH'ing into the camera or via the App's configuration page if exposed: `cat localdata/librespot.log`.

## Acknowledgments
Powered by [librespot-org/librespot](https://github.com/librespot-org/librespot).
