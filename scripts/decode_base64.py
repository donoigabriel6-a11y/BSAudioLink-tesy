#!/usr/bin/env python3
"""Decode a base64-transported binary asset into the build directory."""
import base64
import pathlib
import sys

if len(sys.argv) != 3:
    raise SystemExit("usage: decode_base64.py INPUT.base64 OUTPUT")

source = pathlib.Path(sys.argv[1])
destination = pathlib.Path(sys.argv[2])
encoded = b"".join(source.read_bytes().split())
try:
    decoded = base64.b64decode(encoded, validate=True)
except Exception as exc:
    raise SystemExit(f"Failed to decode {source}: {exc}") from exc

destination.parent.mkdir(parents=True, exist_ok=True)
destination.write_bytes(decoded)
print(f"Decoded {source.name}: {len(decoded)} bytes -> {destination}")
