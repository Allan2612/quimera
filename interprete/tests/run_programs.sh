#!/usr/bin/env bash
# Tests de programas completos: cada tests/programs/<n>.qmr se ejecuta con bin/Quimera y su
# salida (stdout + "exit=<código>") se compara con <n>.expected. Si existe <n>.in, se usa
# como entrada estándar (para Read).
set -u
BIN="${1:-bin/Quimera}"
DIR="$(dirname "$0")/programs"
fails=0; total=0
for src in "$DIR"/*.qmr; do
  name="${src%.qmr}"; total=$((total + 1))
  input=/dev/null; [ -f "$name.in" ] && input="$name.in"
  actual="$(timeout 5 "$BIN" "$src" < "$input" 2>/dev/null; echo "exit=$?")"
  if [ "$actual" == "$(cat "$name.expected")" ]; then
    echo "PASS $(basename "$name")"
  else
    echo "FAIL $(basename "$name")"
    diff <(cat "$name.expected") <(echo "$actual") | sed 's/^/    /'
    fails=$((fails + 1))
  fi
done
echo "$total programas, $fails fallas"
[ "$fails" -eq 0 ]
