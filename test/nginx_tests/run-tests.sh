#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
WWW="$SCRIPT_DIR/www"
OUTPUT="$SCRIPT_DIR/output"

mkdir -p "$OUTPUT"

# 8 configs dans l'ordre des colonnes du tableau : "config_name idx_state"
CONFIGS=(
    "dav-off_ai-off minus"
    "dav-off_ai-off plus"
    "dav-off_ai-on  minus"
    "dav-off_ai-on  plus"
    "dav-on_ai-off  minus"
    "dav-on_ai-off  plus"
    "dav-on_ai-on   minus"
    "dav-on_ai-on   plus"
)

# Configs cfpp-off pour le test PUT nested sans create_full_put_path
CONFIGS_CFPP_OFF=(
    "dav-off_ai-off       minus"
    "dav-off_ai-off       plus"
    "dav-off_ai-on        minus"
    "dav-off_ai-on        plus"
    "dav-on_ai-off_cfpp-off minus"
    "dav-on_ai-off_cfpp-off plus"
    "dav-on_ai-on_cfpp-off  minus"
    "dav-on_ai-on_cfpp-off  plus"
)

apply_config() {
    local cfg="$1"
    local idx="$2"
    cp "$SCRIPT_DIR/configs/${cfg}.conf" "$SCRIPT_DIR/nginx.conf"
    local pidfile="/opt/homebrew/var/run/nginx.pid"
    if [ -f "$pidfile" ] && kill -0 "$(cat "$pidfile")" 2>/dev/null; then
        kill -HUP "$(cat "$pidfile")"
    else
        nginx -c "$SCRIPT_DIR/nginx.conf" 2>/dev/null
    fi
    sleep 1
    # idx
    if [ "$idx" = "plus" ]; then
        echo "ok" > "$WWW/test/index.html"
    else
        rm -f "$WWW/test/index.html"
    fi
}

run_test() {
    local outfile="$OUTPUT/$1"
    local label="$2"
    local method="$3"
    local url="$4"
    local setup="$5"
    local cleanup="$6"
    local use_cfpp_off="${7:-}"

    echo "=== TEST: $label ===" > "$outfile"
    echo "" >> "$outfile"

    local i=0
    for entry in "${CONFIGS[@]}"; do
        local cfg idx
        cfg=$(echo "$entry" | awk '{print $1}')
        idx=$(echo "$entry" | awk '{print $2}')

        # Pour cfpp-off, remplacer les configs DAV+ par leurs variantes cfpp-off
        if [ -n "$use_cfpp_off" ] && [ $i -ge 4 ]; then
            cfg=$(echo "${CONFIGS_CFPP_OFF[$i]}" | awk '{print $1}')
        fi

        apply_config "$cfg" "$idx"

        # Setup précondition
        [ -n "$setup" ] && eval "$setup" 2>/dev/null

        echo "--- CONFIG: $cfg | idx: $idx ---" >> "$outfile"
        if [ "$method" = "DELETE" ]; then
            curl -sv -X "$method" "localhost$url" >> "$outfile" 2>&1
        else
            curl -sv -X "$method" "localhost$url" -d "test" >> "$outfile" 2>&1
        fi
        echo "" >> "$outfile"

        # Cleanup
        [ -n "$cleanup" ] && eval "$cleanup" 2>/dev/null
        i=$((i + 1))
    done

    echo "Terminé : $1"
}

# ── POST ──────────────────────────────────────────────────────────────────────

run_test "POST_root.txt" \
    "POST /" POST "/" \
    "" ""

run_test "POST_test_slash.txt" \
    "POST /test/" POST "/test/" \
    "" ""

run_test "POST_test.txt" \
    "POST /test" POST "/test" \
    "" ""

run_test "POST_fichier_absent.txt" \
    "POST /fichier.txt (absent)" POST "/fichier.txt" \
    "rm -f '$WWW/fichier.txt'" ""

run_test "POST_fichier_present.txt" \
    "POST /fichier.txt (présent)" POST "/fichier.txt" \
    "echo ok > '$WWW/fichier.txt'" "rm -f '$WWW/fichier.txt'"

run_test "POST_test_fichier_absent.txt" \
    "POST /test/fichier.txt (absent)" POST "/test/fichier.txt" \
    "rm -f '$WWW/test/fichier.txt'" ""

run_test "POST_test_fichier_present.txt" \
    "POST /test/fichier.txt (présent)" POST "/test/fichier.txt" \
    "echo ok > '$WWW/test/fichier.txt'" "rm -f '$WWW/test/fichier.txt'"

# ── PUT répertoires ───────────────────────────────────────────────────────────

run_test "PUT_root.txt" \
    "PUT /" PUT "/" \
    "" ""

run_test "PUT_test_slash.txt" \
    "PUT /test/" PUT "/test/" \
    "" ""

run_test "PUT_test.txt" \
    "PUT /test" PUT "/test" \
    "" ""

# ── PUT fichiers ──────────────────────────────────────────────────────────────

run_test "PUT_nouveau_absent.txt" \
    "PUT /nouveau.txt (absent)" PUT "/nouveau.txt" \
    "rm -f '$WWW/nouveau.txt'" "rm -f '$WWW/nouveau.txt'"

run_test "PUT_nouveau_present.txt" \
    "PUT /nouveau.txt (présent)" PUT "/nouveau.txt" \
    "echo ok > '$WWW/nouveau.txt'" "rm -f '$WWW/nouveau.txt'"

run_test "PUT_test_nouveau_absent.txt" \
    "PUT /test/nouveau.txt (absent)" PUT "/test/nouveau.txt" \
    "rm -f '$WWW/test/nouveau.txt'" "rm -f '$WWW/test/nouveau.txt'"

run_test "PUT_test_nouveau_present.txt" \
    "PUT /test/nouveau.txt (présent)" PUT "/test/nouveau.txt" \
    "echo ok > '$WWW/test/nouveau.txt'" "rm -f '$WWW/test/nouveau.txt'"

run_test "PUT_test_nested_cfpp_on.txt" \
    "PUT /test/a/b/c.txt (create_full_put_path on)" PUT "/test/a/b/c.txt" \
    "rm -rf '$WWW/test/a'" "rm -rf '$WWW/test/a'"

run_test "PUT_test_nested_cfpp_off.txt" \
    "PUT /test/a/b/c.txt (create_full_put_path off)" PUT "/test/a/b/c.txt" \
    "rm -rf '$WWW/test/a'" "rm -rf '$WWW/test/a'" \
    "CONFIGS_CFPP_OFF"

# ── DELETE ────────────────────────────────────────────────────────────────────

run_test "DELETE_nouveau_present.txt" \
    "DELETE /nouveau.txt (présent)" DELETE "/nouveau.txt" \
    "echo ok > '$WWW/nouveau.txt'" "rm -f '$WWW/nouveau.txt'"

run_test "DELETE_nouveau_absent.txt" \
    "DELETE /nouveau.txt (absent)" DELETE "/nouveau.txt" \
    "rm -f '$WWW/nouveau.txt'" ""

run_test "DELETE_test_nouveau_present.txt" \
    "DELETE /test/nouveau.txt (présent)" DELETE "/test/nouveau.txt" \
    "echo ok > '$WWW/test/nouveau.txt'" "rm -f '$WWW/test/nouveau.txt'"

run_test "DELETE_test_nouveau_absent.txt" \
    "DELETE /test/nouveau.txt (absent)" DELETE "/test/nouveau.txt" \
    "rm -f '$WWW/test/nouveau.txt'" ""

run_test "DELETE_root.txt" \
    "DELETE /" DELETE "/" \
    "" ""

run_test "DELETE_test_slash_vide.txt" \
    "DELETE /test/ (répertoire vide)" DELETE "/test/" \
    "rm -rf '$WWW/test' && mkdir -p '$WWW/test'" "mkdir -p '$WWW/test'"

run_test "DELETE_test_slash_nonvide.txt" \
    "DELETE /test/ (répertoire non vide)" DELETE "/test/" \
    "mkdir -p '$WWW/test' && echo ok > '$WWW/test/dummy.txt'" \
    "rm -f '$WWW/test/dummy.txt' && mkdir -p '$WWW/test'"

echo ""
echo "✓ Tous les tests terminés. Résultats dans : $OUTPUT/"
echo "  $(ls "$OUTPUT" | wc -l | tr -d ' ') fichiers générés."
