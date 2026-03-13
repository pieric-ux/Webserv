#!/bin/bash
# Usage: ./setup-idx.sh <plus|minus>

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
INDEX="$SCRIPT_DIR/www/test/index.html"

case "$1" in
    plus)
        echo "ok" > "$INDEX"
        echo "index.html créé : $INDEX"
        ;;
    minus)
        rm -f "$INDEX"
        echo "index.html supprimé"
        ;;
    *)
        echo "Usage: $0 <plus|minus>"
        exit 1
        ;;
esac
