#!/usr/bin/env python3
"""Replace one mood sprite in src/sprites.h directly from a PNG.

Usage:
  python tools/add_sprite.py my_face.png happy
  python tools/add_sprite.py crying.png  sad

Mood names: happy  okay  sad  dead

The script converts the image to 120x120 RGB565 and patches sprites.h
in-place — no manual copy-pasting needed.
"""

import sys
import re
import argparse
from pathlib import Path
from PIL import Image

# Works whether the script lives in the repo root or in a tools/ subfolder
_here = Path(__file__).parent
SPRITES_H = (
    _here / "src" / "sprites.h"
    if (_here / "src" / "sprites.h").exists()
    else _here.parent / "src" / "sprites.h"
)
VALID_MOODS = {"happy", "okay", "sad", "dead"}


def to_rgb565_bytes(input_path, width=120, height=120):
    img = Image.open(input_path).convert("RGB").resize(
        (width, height), Image.Resampling.NEAREST
    )
    buf = bytearray()
    for r, g, b in img.getdata():
        color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
        buf.append(color >> 8)
        buf.append(color & 0xFF)
    return buf


def array_body(buf):
    lines = []
    row = []
    for byte in buf:
        row.append(f"0x{byte:02X}")
        if len(row) == 16:
            lines.append("    " + ", ".join(row) + ",")
            row = []
    if row:
        lines.append("    " + ", ".join(row) + ",")
    return "\n".join(lines)


def patch_sprites_h(name_upper, buf):
    if not SPRITES_H.exists():
        print(f"Error: sprites.h not found at {SPRITES_H}")
        print("Run this script from inside the project folder.")
        sys.exit(1)

    content = SPRITES_H.read_text()

    new_array = (
        f"const uint8_t SPRITE_{name_upper}[] PROGMEM = {{\n"
        f"{array_body(buf)}\n"
        f"}};"
    )

    # Match from the array declaration to the closing };
    pattern = (
        r"const uint8_t SPRITE_" + name_upper +
        r"\[\] PROGMEM = \{[^}]*\};"
    )
    updated, count = re.subn(pattern, new_array, content, flags=re.DOTALL)

    if count == 0:
        print(f"Error: SPRITE_{name_upper} not found in sprites.h")
        print("Make sure sprites.h contains the standard placeholder arrays.")
        sys.exit(1)

    SPRITES_H.write_text(updated)
    print(f"SPRITE_{name_upper} updated in {SPRITES_H.relative_to(Path.cwd())}")
    print("Re-upload your firmware to see the new sprite on screen!")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("input", help="PNG file to use as sprite")
    parser.add_argument("mood",  choices=sorted(VALID_MOODS),
                        help="Which mood to replace (happy / okay / sad / dead)")
    args = parser.parse_args()

    try:
        buf = to_rgb565_bytes(args.input)
    except FileNotFoundError:
        print(f"Error: file not found: {args.input}")
        sys.exit(1)
    except Exception as e:
        print(f"Error reading image: {e}")
        sys.exit(1)

    patch_sprites_h(args.mood.upper(), buf)
