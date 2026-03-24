#!/bin/bash
# Usage: ./use-config.sh <dav-off_ai-off|dav-off_ai-on|dav-on_ai-off|dav-on_ai-on>

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CONFIG="$SCRIPT_DIR/configs/$1.conf"

if [ -z "$1" ] || [ ! -f "$CONFIG" ]; then
    echo "Usage: $0 <dav-off_ai-off|dav-off_ai-on|dav-on_ai-off|dav-on_ai-on>"
    exit 1
fi

cp "$CONFIG" "$SCRIPT_DIR/nginx.conf"
nginx -c "$SCRIPT_DIR/nginx.conf" -s reload
echo "Config appliquée : $1"
