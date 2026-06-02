#!/usr/bin/env bash
set -euo pipefail

SRC="$HOME/PhD/Datafiles/dna.200MB"
OUT="$HOME/PhD/rlz_cache_experiments"
RLZ_ROOT="$HOME/PhD/rlz"

mkdir -p "$OUT/refs" "$OUT/inputs" "$OUT/warm_inputs" "$OUT/results"

echo "Creating RLZ experiment files from: $SRC"
echo "Output folder: $OUT"

python3 - << 'PY'
from pathlib import Path

SRC = Path.home() / "PhD/Datafiles/dna.200MB"
OUT = Path.home() / "PhD/rlz_cache_experiments"
MB = 1024 * 1024

data = SRC.read_bytes()
n = len(data)

sentinel = None
present = set(data)
for b in range(256):
    if b not in present:
        sentinel = bytes([b])
        break

if sentinel is None:
    raise RuntimeError("No unused byte found for sentinel.")

print(f"source bytes   : {n}")
print(f"sentinel byte : {sentinel[0]}")

def write_ref(name, start_mb, size_mb):
    start = start_mb * MB
    size = size_mb * MB
    chunk = data[start:start + size]
    if len(chunk) != size:
        raise RuntimeError(f"Not enough data for {name}")
    path = OUT / "refs" / name
    path.write_bytes(chunk + sentinel)
    print(f"REF   {path}  {len(chunk) + 1} bytes")

def write_input(folder, name, start_mb, size_mb):
    start = start_mb * MB
    size = size_mb * MB
    chunk = data[start:start + size]
    if len(chunk) != size:
        raise RuntimeError(f"Not enough data for {name}")
    path = OUT / folder / name
    path.write_bytes(chunk)
    print(f"INPUT {path}  {len(chunk)} bytes")

# References with sentinel appended.
write_ref("dna.ref.25MB.sentinel", 0, 25)
write_ref("dna.ref.50MB.sentinel", 0, 50)
write_ref("dna.ref.100MB.sentinel", 0, 100)

# Single-input experiments.
write_input("inputs", "dna.input.after25.25MB", 25, 25)
write_input("inputs", "dna.input.after25.50MB", 25, 50)
write_input("inputs", "dna.input.after50.25MB", 50, 25)
write_input("inputs", "dna.input.after50.50MB", 50, 50)
write_input("inputs", "dna.input.after50.100MB", 50, 100)
write_input("inputs", "dna.input.after100.50MB", 100, 50)

# Warm-cache multi-input experiments.
for k, start_mb in enumerate(range(50, 150, 10), start=1):
    write_input("warm_inputs", f"dna.warm.{k:02d}.10MB", start_mb, 10)
PY

echo
echo "Building suffix arrays for reference files..."

cd "$RLZ_ROOT/sa"

if [ ! -x "./build/sa" ]; then
    echo "SA binary not found. Building SA tool..."
    cmake -S . -B build
    cmake --build build -j
fi

for ref in "$OUT"/refs/*.sentinel; do
    echo "Building SA: $ref.sa"
    ./build/sa "$ref" "$ref.sa"
done

echo
echo "Done."
echo "Experiment files are in: $OUT"
echo
echo "Example run:"
echo "cd $RLZ_ROOT/parser"
echo "./compare_rlz_cache \\"
echo "  $OUT/refs/dna.ref.50MB.sentinel \\"
echo "  $OUT/refs/dna.ref.50MB.sentinel.sa \\"
echo "  $OUT/inputs/dna.input.after50.50MB \\"
echo "  256 3"