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

# --- Kontrola názvu souboru (xlogin00.zip) ---
if [[ ! "$BASENAME" =~ ^[a-z0-9]{6,9}$ ]]; then
    echo "VAROVÁNÍ: Název '$BASENAME' nemá správný formát (očekáváno xlogin00.zip s 6–9 znaky)."
fi

# --- Extrakce zipu do dočasného adresáře ---
TMPDIR=$(mktemp -d)
unzip -qq "$ZIPFILE" -d "$TMPDIR"

# --- Kontrola složek ---
if find "$TMPDIR" -type d | grep -qv "^$TMPDIR$"; then
    echo "INFO: Archiv obsahuje složky (některé soubory očekáváme ve složce parallel_builder)."
fi

# --- Očekávané soubory ---
EXPECTED_IN_PARALLEL_BUILDER=(
    "loop_mesh_builder.h"
    "loop_mesh_builder.cpp"
    "tree_mesh_builder.h"
    "tree_mesh_builder.cpp"
)

EXPECTED_IN_ROOT=(
    "1_2.txt"
    "2_1.txt"
    "3_4.txt"
    "4_1.txt"
    "4_2.txt"
    "4_3.txt"
    "PMC-$BASENAME.txt"
)

# --- Kontrola souborů ve složce parallel_builder ---
for file in "${EXPECTED_IN_PARALLEL_BUILDER[@]}"; do
    if [ ! -f "$TMPDIR/parallel_builder/$file" ]; then
        echo "VAROVÁNÍ: Chybí soubor '$file' ve složce parallel_builder"
    fi
done

# --- Kontrola souborů v kořenovém adresáři ---
for file in "${EXPECTED_IN_ROOT[@]}"; do
    if [ ! -f "$TMPDIR/$file" ]; then
        echo "VAROVÁNÍ: Chybí soubor '$file' v kořenovém adresáři"
    fi
done

# --- Kontrola neočekávaných souborů ---
ALL_EXPECTED=("${EXPECTED_IN_PARALLEL_BUILDER[@]/#/parallel_builder/}" "${EXPECTED_IN_ROOT[@]}")
ACTUAL_FILES=($(find "$TMPDIR" -type f -printf "%P\n"))

for file in "${ACTUAL_FILES[@]}"; do
    if ! printf '%s\n' "${ALL_EXPECTED[@]}" | grep -qx "$file"; then
        echo "VAROVÁNÍ: Archiv obsahuje neočekávaný soubor '$file'"
    fi
done

# --- Kontrola shody loginu v názvu PMC-xlogin00.txt ---
PMCFILE=$(printf '%s\n' "${ACTUAL_FILES[@]}" | grep -E '^PMC-.*\.txt$' || true)
if [ -n "$PMCFILE" ]; then
    LOGIN_IN_PMC=$(echo "$PMCFILE" | sed -E 's/^PMC-(.*)\.txt$/\1/')
    if [ "$LOGIN_IN_PMC" != "$BASENAME" ]; then
        echo "VAROVÁNÍ: Soubor '$PMCFILE' neodpovídá loginu v názvu archivu ('$BASENAME')."
    fi
fi

# --- Úklid ---
rm -rf "$TMPDIR"
echo "Kontrola dokončena."

