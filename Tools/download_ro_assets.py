#!/usr/bin/env python3
"""
Ragnarok Online Asset Downloader for Ragna-TH Project

Downloads the COMPLETE set of item icons, skill icons, and monster sprites
from publicly available Ragnarok Online community resources.

Usage:
    python3 Tools/download_ro_assets.py --all          # Full RO asset set
    python3 Tools/download_ro_assets.py --items        # All item icons only
    python3 Tools/download_ro_assets.py --skills       # All skill icons only
    python3 Tools/download_ro_assets.py --monsters     # All monster sprites only
    python3 Tools/download_ro_assets.py --workers 8    # Parallel downloads

Sources:
    - Item icons: static.divine-pride.net
    - Skill icons: static.divine-pride.net
    - Monster sprites: static.divine-pride.net
"""

import argparse
import os
import sys
import time
import urllib.request
import urllib.error
import json
import threading
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

# CDN URL patterns
ITEM_ICON_URL = "https://static.divine-pride.net/images/items/item/{id}.png"
ITEM_COLLECTION_URL = "https://static.divine-pride.net/images/items/collection/{id}.png"
SKILL_ICON_URL = "https://static.divine-pride.net/images/skills/{id}.png"
MONSTER_IMAGE_URL = "https://static.divine-pride.net/images/mobs/png/{id}.png"

# Default output relative to project root
DEFAULT_CONTENT_DIR = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "RagnarokUE", "Content"
)

# ============================================================================
# Complete Ragnarok Online ID ranges
# ============================================================================

# Items: 501-30000 covers all classic through renewal items
# Major ranges:
#   501-999    Usable items (potions, herbs, cooking materials)
#   1000-1999  Weapons & shields
#   2000-2999  Armor, garments, shoes, accessories
#   3000-3999  Taming items, misc
#   4000-4999  Cards
#   5000-5999  Headgears (upper)
#   6000-6999  Delay-consume, shadow gear, costume
#   7000-7999  Etc items (quest, crafting)
#   10000-19999 Renewal items
#   20000-30000 Extended renewal items
ITEM_RANGES = [
    (501, 1000),      # Usable items
    (1000, 2000),     # Weapons & shields
    (2000, 3000),     # Armor & accessories
    (3000, 4000),     # Taming, misc
    (4000, 5000),     # Cards
    (5000, 6000),     # Headgears
    (6000, 7000),     # Shadow/costume gear
    (7000, 8000),     # Etc items
    (10000, 20000),   # Renewal items
    (20000, 32000),   # Extended renewal
]

# Skills: 1-3036 covers all skills through 4th jobs
SKILL_RANGES = [
    (1, 700),         # 1st/2nd class skills + homunculus
    (700, 1500),      # Extended/3rd class skills
    (1500, 2600),     # 3rd class / rebellion / summoner
    (2600, 3100),     # 4th class skills
    (3100, 3600),     # Extended 4th
    (4000, 4300),     # Soul linker / star gladiator extended
    (5000, 5300),     # Newer skills
    (8000, 8500),     # Special/event skills
]

# Monsters: 1001-4000 covers classic + renewal mobs
MONSTER_RANGES = [
    (1001, 2000),     # Classic monsters (Prontera, Payon, etc.)
    (2000, 2500),     # Episode monsters (Rachel, Veins, etc.)
    (2500, 3000),     # Bio Lab, Instances
    (3000, 3600),     # Renewal monsters
    (3600, 4100),     # Extended renewal
]

# Element names for UI icons
ELEMENT_NAMES = [
    "Neutral", "Water", "Earth", "Fire", "Wind",
    "Poison", "Holy", "Shadow", "Ghost", "Undead"
]

# Thread-safe counters
_print_lock = threading.Lock()


def download_file(url, output_path, retries=2, delay=0.5):
    """Download a file with retry logic."""
    for attempt in range(retries):
        try:
            req = urllib.request.Request(url, headers={
                "User-Agent": "RagnaTH-AssetDownloader/1.0"
            })
            with urllib.request.urlopen(req, timeout=10) as response:
                data = response.read()
                if len(data) < 100:  # Likely an error/placeholder
                    return False
                os.makedirs(os.path.dirname(output_path), exist_ok=True)
                with open(output_path, "wb") as f:
                    f.write(data)
                return True
        except urllib.error.HTTPError as e:
            if e.code == 404:
                return False  # Asset doesn't exist, no retry
            if attempt < retries - 1:
                time.sleep(delay * (2 ** attempt))
            else:
                return False
        except (urllib.error.URLError, Exception):
            if attempt < retries - 1:
                time.sleep(delay * (2 ** attempt))
            else:
                return False
    return False


def download_range(url_template, output_dir, id_start, id_end,
                   category_name, workers=4, rate_limit=0.05):
    """Download assets for a range of IDs using parallel workers."""
    success, fail, skip, not_found = 0, 0, 0, 0

    def fetch_one(asset_id):
        filename = f"{asset_id}.png"
        path = os.path.join(output_dir, filename)

        if os.path.exists(path):
            return ("skip", asset_id)

        url = url_template.format(id=asset_id)
        if download_file(url, path):
            return ("ok", asset_id)
        else:
            return ("miss", asset_id)

    total = id_end - id_start
    batch_size = 100
    current = id_start

    while current < id_end:
        batch_end = min(current + batch_size, id_end)
        ids = range(current, batch_end)

        with ThreadPoolExecutor(max_workers=workers) as executor:
            futures = {executor.submit(fetch_one, i): i for i in ids}
            for future in as_completed(futures):
                status, aid = future.result()
                if status == "ok":
                    success += 1
                elif status == "skip":
                    skip += 1
                else:
                    not_found += 1

        # Progress update every batch
        done = current - id_start + batch_size
        with _print_lock:
            print(f"\r  {category_name}: {min(done, total)}/{total} scanned "
                  f"({success} new, {skip} cached, {not_found} N/A)", end="", flush=True)

        current = batch_end
        time.sleep(rate_limit)  # Small pause between batches

    print()  # Newline after progress
    return success, fail, skip, not_found


def download_items(content_dir, workers=4):
    """Download all item icons across full RO ID ranges."""
    icon_dir = os.path.join(content_dir, "Items", "Icons")
    total_success, total_skip, total_na = 0, 0, 0

    print(f"\n{'='*60}")
    print(f"  DOWNLOADING COMPLETE ITEM ICON SET")
    print(f"{'='*60}")

    for start, end in ITEM_RANGES:
        print(f"\n  Range {start}-{end}:")
        s, _, sk, na = download_range(
            ITEM_ICON_URL, icon_dir, start, end,
            f"Items {start}-{end}", workers
        )
        total_success += s
        total_skip += sk
        total_na += na

    print(f"\n  Items total: {total_success} new + {total_skip} cached "
          f"({total_na} not available)")
    return total_success + total_skip, 0


def download_skills(content_dir, workers=4):
    """Download all skill icons."""
    skill_dir = os.path.join(content_dir, "UI", "Icons", "Skills")
    total_success, total_skip, total_na = 0, 0, 0

    print(f"\n{'='*60}")
    print(f"  DOWNLOADING COMPLETE SKILL ICON SET")
    print(f"{'='*60}")

    for start, end in SKILL_RANGES:
        print(f"\n  Range {start}-{end}:")
        s, _, sk, na = download_range(
            SKILL_ICON_URL, skill_dir, start, end,
            f"Skills {start}-{end}", workers
        )
        total_success += s
        total_skip += sk
        total_na += na

    print(f"\n  Skills total: {total_success} new + {total_skip} cached "
          f"({total_na} not available)")
    return total_success + total_skip, 0


def download_monsters(content_dir, workers=4):
    """Download all monster sprites."""
    monster_dir = os.path.join(content_dir, "Monsters", "Icons")
    total_success, total_skip, total_na = 0, 0, 0

    print(f"\n{'='*60}")
    print(f"  DOWNLOADING COMPLETE MONSTER SPRITE SET")
    print(f"{'='*60}")

    for start, end in MONSTER_RANGES:
        print(f"\n  Range {start}-{end}:")
        s, _, sk, na = download_range(
            MONSTER_IMAGE_URL, monster_dir, start, end,
            f"Monsters {start}-{end}", workers
        )
        total_success += s
        total_skip += sk
        total_na += na

    print(f"\n  Monsters total: {total_success} new + {total_skip} cached "
          f"({total_na} not available)")
    return total_success + total_skip, 0


def generate_element_placeholders(content_dir):
    """Generate colored placeholder PNGs for element icons."""
    element_dir = os.path.join(content_dir, "UI", "Icons", "Elements")
    os.makedirs(element_dir, exist_ok=True)

    colors = {
        "Neutral": (200, 200, 200),
        "Water": (64, 128, 255),
        "Earth": (139, 119, 42),
        "Fire": (255, 64, 32),
        "Wind": (128, 255, 128),
        "Poison": (148, 0, 211),
        "Holy": (255, 255, 200),
        "Shadow": (64, 0, 64),
        "Ghost": (200, 200, 255),
        "Undead": (80, 80, 80),
    }

    success = 0
    print(f"\n--- Generating {len(ELEMENT_NAMES)} element icon placeholders ---")

    for i, elem in enumerate(ELEMENT_NAMES):
        filename = f"{i}_{elem}.png"
        path = os.path.join(element_dir, filename)

        if os.path.exists(path):
            print(f"  [SKIP] {filename}")
            success += 1
            continue

        r, g, b = colors.get(elem, (128, 128, 128))
        png_data = create_minimal_png(24, 24, r, g, b)
        with open(path, "wb") as f:
            f.write(png_data)
        print(f"  [OK]   {filename}")
        success += 1

    return success, 0


def create_minimal_png(width, height, r, g, b):
    """Create a minimal valid PNG file with a solid color."""
    import struct
    import zlib

    def chunk(chunk_type, data):
        c = chunk_type + data
        crc = struct.pack(">I", zlib.crc32(c) & 0xFFFFFFFF)
        return struct.pack(">I", len(data)) + c + crc

    sig = b"\x89PNG\r\n\x1a\n"
    ihdr_data = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    ihdr = chunk(b"IHDR", ihdr_data)

    raw_rows = b""
    for _ in range(height):
        raw_rows += b"\x00"
        raw_rows += bytes([r, g, b]) * width
    compressed = zlib.compress(raw_rows)
    idat = chunk(b"IDAT", compressed)
    iend = chunk(b"IEND", b"")

    return sig + ihdr + idat + iend


def write_asset_manifest(content_dir):
    """Scan all downloaded PNGs and write the asset manifest."""
    manifest = {"items": {}, "skills": {}, "monsters": {}, "elements": {}}

    # Scan items
    items_dir = os.path.join(content_dir, "Items", "Icons")
    if os.path.isdir(items_dir):
        for f in os.listdir(items_dir):
            if f.endswith(".png"):
                item_id = f.split("_")[0] if "_" in f else f.replace(".png", "")
                manifest["items"][item_id] = f"Items/Icons/{f}"

    # Scan skills
    skills_dir = os.path.join(content_dir, "UI", "Icons", "Skills")
    if os.path.isdir(skills_dir):
        for f in os.listdir(skills_dir):
            if f.endswith(".png"):
                skill_id = f.split("_")[0] if "_" in f else f.replace(".png", "")
                manifest["skills"][skill_id] = f"UI/Icons/Skills/{f}"

    # Scan monsters
    monsters_dir = os.path.join(content_dir, "Monsters", "Icons")
    if os.path.isdir(monsters_dir):
        for f in os.listdir(monsters_dir):
            if f.endswith(".png"):
                mon_id = f.split("_")[0] if "_" in f else f.replace(".png", "")
                manifest["monsters"][mon_id] = f"Monsters/Icons/{f}"

    # Scan elements
    elements_dir = os.path.join(content_dir, "UI", "Icons", "Elements")
    if os.path.isdir(elements_dir):
        for f in os.listdir(elements_dir):
            if f.endswith(".png"):
                elem_id = f.split("_")[0] if "_" in f else f.replace(".png", "")
                manifest["elements"][elem_id] = f"UI/Icons/Elements/{f}"

    manifest_path = os.path.join(content_dir, "Data", "asset_manifest.json")
    os.makedirs(os.path.dirname(manifest_path), exist_ok=True)
    with open(manifest_path, "w") as f:
        json.dump(manifest, f, indent=2, sort_keys=True)

    total = sum(len(v) for v in manifest.values())
    print(f"\nAsset manifest: {total} entries written to {manifest_path}")
    print(f"  Items: {len(manifest['items'])}")
    print(f"  Skills: {len(manifest['skills'])}")
    print(f"  Monsters: {len(manifest['monsters'])}")
    print(f"  Elements: {len(manifest['elements'])}")


def main():
    parser = argparse.ArgumentParser(
        description="Download COMPLETE Ragnarok Online assets for Ragna-TH")
    parser.add_argument("--all", action="store_true",
                        help="Download all asset types (full RO set)")
    parser.add_argument("--items", action="store_true",
                        help="Download all item icons (IDs 501-32000)")
    parser.add_argument("--skills", action="store_true",
                        help="Download all skill icons (IDs 1-8500)")
    parser.add_argument("--monsters", action="store_true",
                        help="Download all monster sprites (IDs 1001-4100)")
    parser.add_argument("--elements", action="store_true",
                        help="Generate element icon placeholders")
    parser.add_argument("--workers", type=int, default=4,
                        help="Number of parallel download workers (default: 4)")
    parser.add_argument("--output-dir", default=DEFAULT_CONTENT_DIR,
                        help=f"Content output directory")

    args = parser.parse_args()

    if not any([args.all, args.items, args.skills, args.monsters, args.elements]):
        args.all = True

    content_dir = args.output_dir
    workers = args.workers

    print(f"Ragna-TH Complete Asset Downloader")
    print(f"Output: {content_dir}")
    print(f"Workers: {workers}")

    total_success, total_fail = 0, 0

    if args.all or args.items:
        s, f = download_items(content_dir, workers)
        total_success += s
        total_fail += f

    if args.all or args.skills:
        s, f = download_skills(content_dir, workers)
        total_success += s
        total_fail += f

    if args.all or args.monsters:
        s, f = download_monsters(content_dir, workers)
        total_success += s
        total_fail += f

    if args.all or args.elements:
        s, f = generate_element_placeholders(content_dir)
        total_success += s
        total_fail += f

    write_asset_manifest(content_dir)

    print(f"\n{'='*60}")
    print(f"  COMPLETE: {total_success} total assets available")
    print(f"{'='*60}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
