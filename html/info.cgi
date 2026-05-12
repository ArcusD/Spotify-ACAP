#!/bin/sh
echo "Content-Type: application/json"
echo ""
echo '{"url": "http://127.0.0.1:8000/stream.wav", "name": "Spotify Connect", "type": "acap", "status": "playing"}'
