#!/bin/sh

printf "Content-type: text/plain\r\n\r\n"

if [ -f "localdata/librespot.log" ]; then
    cat localdata/librespot.log
else
    echo "No log file found."
fi
