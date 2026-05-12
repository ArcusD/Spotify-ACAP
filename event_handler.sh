#!/bin/sh

# This script is called by librespot on various events.
EVENT=$1

case "$EVENT" in
    "playing")
        syslog -p i -t SpotifyConnect "Playback started via ALSA."
        ;;
    "paused" | "stopped")
        syslog -p i -t SpotifyConnect "Playback stopped."
        ;;
esac

exit 0
