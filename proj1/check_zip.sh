#!/bin/bash
# ===========================================
# Skript: check_zip.sh
# Účel: Ověří správnost názvu a struktury zip archivu
# ===========================================

# --- Kontrola argumentu ---
if [ $# -ne 1 ]; then
    echo "Použití: $0 <soubor.zip>"
    exit 1
fi

ZIPFILE="$1"

# --- Kontrola, zda soubor existuje ---
if [ ! -f "$ZIPFILE" ]; then
    echo "Chyba: Soubor '$ZIPFILE' neexistuje."
    exit 1
fi

# --- Extrakce jména bez přípony ---
BASENAME=$(basename "$ZIPFILE" .zip)

# --- Kontrola názvu souboru (login00.zip) ---
if [[ ! "$BASENAME" =~ ^[a-z0-9]{6,9}$ ]]; then
    echo "VAROVÁNÍ: Název '$BASENAME' nemá správný formát (očekáváno xlogin00.zip s 8–9 znaky)."
fi

# --- Kontrola obsahu zipu ---
# Vytvoření dočasného adresáře
TMPDIR=$(mktemp -d)
unzip -qq "$ZIPFILE" -d "$TMPDIR"

# --- Kontrola, že archiv neobsahuje žádné složky ---
if find "$TMPDIR" -type d | grep -qv "^$TMPDIR$"; then
    echo "VAROVÁNÍ: Archiv obsahuje složky – měly by tam být pouze soubory v kořenovém adresáři."
fi

# --- Seznam očekávaných souborů ---
EXPECTED_FILES=(
    "LineMandelCalculator.cc"
    "LineMandelCalculator.h"
    "BatchMandelCalculator.cc"
    "BatchMandelCalculator.h"
    "eval.png"
    "MB-$BASENAME.txt"
)

# --- Získání skutečného obsahu ---
ACTUAL_FILES=($(find "$TMPDIR" -maxdepth 1 -type f -printf "%f\n"))

# --- Kontrola chybějících souborů ---
for file in "${EXPECTED_FILES[@]}"; do
    if ! printf '%s\n' "${ACTUAL_FILES[@]}" | grep -qx "$file"; then
        echo "VAROVÁNÍ: Chybí soubor '$file'"
    fi
done

# --- Kontrola, zda tam nejsou jiné soubory ---
for file in "${ACTUAL_FILES[@]}"; do
    if ! printf '%s\n' "${EXPECTED_FILES[@]}" | grep -qx "$file"; then
        echo "VAROVÁNÍ: Archiv obsahuje neočekávaný soubor '$file'"
    fi
done

# --- Kontrola shody loginu v názvu MB-xlogin00.txt ---
MBFILE=$(printf '%s\n' "${ACTUAL_FILES[@]}" | grep -E '^MB-.*\.txt$' || true)
if [ -n "$MBFILE" ]; then
    LOGIN_IN_MB=$(echo "$MBFILE" | sed -E 's/^MB-(.*)\.txt$/\1/')
    if [ "$LOGIN_IN_MB" != "$BASENAME" ]; then
        echo "VAROVÁNÍ: Soubor '$MBFILE' neodpovídá loginu v názvu archivu ('$BASENAME')."
    fi
fi

# --- Úklid ---
rm -rf "$TMPDIR"
echo "Kontrola dokončena."
