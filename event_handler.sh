#!/bin/sh

# This script is called by librespot on various events.
EVENT=$1

case "$EVENT" in
    "playing")
        syslog -p i -t SpotifyConnect "Playback started via ALSA."
        echo "{\"event\":\"playing\",\"track_id\":\"$TRACK_ID\"}" > localdata/state.json
        ;;
    "paused" | "stopped")
        syslog -p i -t SpotifyConnect "Playback stopped."
        echo "{\"event\":\"$EVENT\"}" > localdata/state.json
        ;;
    *)
        echo "{\"event\":\"$EVENT\"}" > localdata/state.json
        ;;
esac

exit 0
