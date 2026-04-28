#!/bin/sh

# Output HTTP headers
printf "Content-type: application/json\r\n\r\n"

# Read and output the state file, or a default if it doesn't exist
if [ -f "localdata/state.json" ]; then
    cat localdata/state.json
else
    echo "{\"event\": \"stopped\", \"track_id\": \"\"}"
fi
