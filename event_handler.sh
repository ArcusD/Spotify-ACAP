#!/bin/sh
# This script is called by librespot on playback events.
# It receives environment variables like PLAYER_EVENT and TRACK_ID.

# Write the state to a JSON file in the writable localdata directory
echo "{\"event\": \"$PLAYER_EVENT\", \"track_id\": \"$TRACK_ID\"}" > localdata/state.json
