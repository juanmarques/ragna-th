#!/usr/bin/env python3
"""
Ragnarok Online Asset Downloader for Ragna-TH Project

Downloads item icons, skill icons, and monster sprites from publicly available
Ragnarok Online community resources and saves them as PNGs in the UE5 Content
directory structure.

Usage:
    python3 Tools/download_ro_assets.py [--all] [--items] [--skills] [--monsters] [--elements]
    python3 Tools/download_ro_assets.py --all --output-dir /path/to/RagnarokUE/Content

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
from pathlib import Path

# CDN URL patterns
ITEM_ICON_URL = "https://static.divine-pride.net/images/items/item/{item_id}.png"
ITEM_COLLECTION_URL = "https://static.divine-pride.net/images/items/collection/{item_id}.png"
SKILL_ICON_URL = "https://static.divine-pride.net/images/skills/{skill_id}.png"
MONSTER_IMAGE_URL = "https://static.divine-pride.net/images/mobs/png/{monster_id}.png"

# Default output relative to project root
DEFAULT_CONTENT_DIR = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "RagnarokUE", "Content"
)

# All IDs used in the Ragna-TH codebase
ITEM_IDS = {
    # Consumables
    501: "Red_Potion",
    505: "Blue_Potion",
    512: "Apple",
    515: "Meat",
    511: "Green_Herb",
    601: "Wing_of_Fly",
    619: "Old_Card_Album",
    620: "Orange_Juice",
    713: "Empty_Bottle",
    741: "Unripe_Apple",
    # Materials
    705: "Clover",
    908: "Spawn",
    909: "Jellopy",
    910: "Insect_Feeler",
    914: "Fluff",
    915: "Chrysalis",
    917: "Feather_of_Birds",
    918: "Singing_Plant",
    924: "Worm_Peeling",
    935: "Shell",
    938: "Sticky_Mucus",
    940: "Grasshoppers_Leg",
    943: "Iron",
    949: "Feather",
    999: "Steel",
    # Equipment
    1101: "Sword",
    1201: "Knife",
    1301: "Axe",
    1402: "Violin",
    1501: "Mace",
    1601: "Rod",
    1701: "Bow",
    1750: "Arrow",
    # Armor
    1019: "Wooden_Mail",
    # Cards
    4001: "Poring_Card",
    4002: "Fabre_Card",
    4003: "Pupa_Card",
    4004: "Willow_Card",
    4005: "Condor_Card",
    4006: "Lunatic_Card",
    4007: "Roda_Frog_Card",
    4018: "Steel_Chonchon_Card",
    4020: "Thief_Bug_Card",
    4021: "Rocker_Card",
    4033: "Drops_Card",
    # Quest items
    7001: "Swordsman_Badge",
    7002: "Mage_Test_Reagent",
}

SKILL_IDS = {
    5: "Bash",
    7: "Magnum_Break",
    19: "Fire_Bolt",
    20: "Cold_Bolt",
    21: "Lightning_Bolt",
    28: "Heal",
    42: "Discount",
    46: "Double_Strafe",
    51: "Hiding",
}

MONSTER_IDS = {
    1002: "Poring",
    1007: "Fabre",
    1008: "Pupa",
    1009: "Condor",
    1010: "Willow",
    1012: "Roda_Frog",
    1031: "Poporing",
    1042: "Steel_Chonchon",
    1051: "Thief_Bug",
    1052: "Rocker",
    1063: "Lunatic",
    1113: "Drops",
}

# Element names for UI icons (these use skill-like icons in RO)
ELEMENT_NAMES = [
    "Neutral", "Water", "Earth", "Fire", "Wind",
    "Poison", "Holy", "Shadow", "Ghost", "Undead"
]


def download_file(url, output_path, retries=3, delay=1.0):
    """Download a file with retry logic and exponential backoff."""
    for attempt in range(retries):
        try:
            req = urllib.request.Request(url, headers={
                "User-Agent": "RagnaTH-AssetDownloader/1.0"
            })
            with urllib.request.urlopen(req, timeout=15) as response:
                data = response.read()
                if len(data) < 100:  # Likely an error page
                    return False
                os.makedirs(os.path.dirname(output_path), exist_ok=True)
                with open(output_path, "wb") as f:
                    f.write(data)
                return True
        except (urllib.error.HTTPError, urllib.error.URLError) as e:
            if attempt < retries - 1:
                wait = delay * (2 ** attempt)
                time.sleep(wait)
            else:
                return False
        except Exception:
            return False
    return False


def download_items(content_dir, include_collection=False):
    """Download item icons."""
    icon_dir = os.path.join(content_dir, "Items", "Icons")
    collection_dir = os.path.join(content_dir, "Items", "Collection")
    success, fail = 0, 0

    print(f"\n--- Downloading {len(ITEM_IDS)} item icons ---")
    for item_id, name in sorted(ITEM_IDS.items()):
        filename = f"{item_id}_{name}.png"

        # Item icon (24x24)
        url = ITEM_ICON_URL.format(item_id=item_id)
        path = os.path.join(icon_dir, filename)
        if os.path.exists(path):
            print(f"  [SKIP] {filename} (already exists)")
            success += 1
            continue

        if download_file(url, path):
            print(f"  [OK]   {filename}")
            success += 1
        else:
            print(f"  [FAIL] {filename} ({url})")
            fail += 1

        # Collection image (75x100)
        if include_collection:
            col_url = ITEM_COLLECTION_URL.format(item_id=item_id)
            col_path = os.path.join(collection_dir, filename)
            if not os.path.exists(col_path):
                download_file(col_url, col_path)

        time.sleep(0.2)  # Rate limiting

    print(f"Items: {success} downloaded, {fail} failed")
    return success, fail


def download_skills(content_dir):
    """Download skill icons."""
    skill_dir = os.path.join(content_dir, "UI", "Icons", "Skills")
    success, fail = 0, 0

    print(f"\n--- Downloading {len(SKILL_IDS)} skill icons ---")
    for skill_id, name in sorted(SKILL_IDS.items()):
        filename = f"{skill_id}_{name}.png"
        url = SKILL_ICON_URL.format(skill_id=skill_id)
        path = os.path.join(skill_dir, filename)

        if os.path.exists(path):
            print(f"  [SKIP] {filename} (already exists)")
            success += 1
            continue

        if download_file(url, path):
            print(f"  [OK]   {filename}")
            success += 1
        else:
            print(f"  [FAIL] {filename} ({url})")
            fail += 1
        time.sleep(0.2)

    print(f"Skills: {success} downloaded, {fail} failed")
    return success, fail


def download_monsters(content_dir):
    """Download monster sprites."""
    monster_dir = os.path.join(content_dir, "Monsters", "Icons")
    success, fail = 0, 0

    print(f"\n--- Downloading {len(MONSTER_IDS)} monster sprites ---")
    for monster_id, name in sorted(MONSTER_IDS.items()):
        filename = f"{monster_id}_{name}.png"
        url = MONSTER_IMAGE_URL.format(monster_id=monster_id)
        path = os.path.join(monster_dir, filename)

        if os.path.exists(path):
            print(f"  [SKIP] {filename} (already exists)")
            success += 1
            continue

        if download_file(url, path):
            print(f"  [OK]   {filename}")
            success += 1
        else:
            print(f"  [FAIL] {filename} ({url})")
            fail += 1
        time.sleep(0.2)

    print(f"Monsters: {success} downloaded, {fail} failed")
    return success, fail


def generate_element_placeholders(content_dir):
    """Generate simple colored placeholder PNGs for element icons.
    These are 24x24 solid-color squares since there's no standard
    CDN source for element icons."""
    element_dir = os.path.join(content_dir, "UI", "Icons", "Elements")
    os.makedirs(element_dir, exist_ok=True)

    # Element -> RGB color mapping
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
            print(f"  [SKIP] {filename} (already exists)")
            success += 1
            continue

        # Create a minimal 24x24 PNG (uncompressed)
        r, g, b = colors.get(elem, (128, 128, 128))
        png_data = create_minimal_png(24, 24, r, g, b)
        with open(path, "wb") as f:
            f.write(png_data)
        print(f"  [OK]   {filename} (placeholder)")
        success += 1

    print(f"Elements: {success} generated")
    return success, 0


def create_minimal_png(width, height, r, g, b):
    """Create a minimal valid PNG file with a solid color."""
    import struct
    import zlib

    def chunk(chunk_type, data):
        c = chunk_type + data
        crc = struct.pack(">I", zlib.crc32(c) & 0xFFFFFFFF)
        return struct.pack(">I", len(data)) + c + crc

    # PNG signature
    sig = b"\x89PNG\r\n\x1a\n"

    # IHDR: width, height, 8-bit RGB, no interlace
    ihdr_data = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    ihdr = chunk(b"IHDR", ihdr_data)

    # IDAT: pixel data (filter byte 0 + RGB for each row)
    raw_rows = b""
    for _ in range(height):
        raw_rows += b"\x00"  # filter: none
        raw_rows += bytes([r, g, b]) * width
    compressed = zlib.compress(raw_rows)
    idat = chunk(b"IDAT", compressed)

    # IEND
    iend = chunk(b"IEND", b"")

    return sig + ihdr + idat + iend


def write_asset_manifest(content_dir):
    """Write a JSON manifest mapping IDs to downloaded asset paths."""
    manifest = {
        "items": {str(k): f"Items/Icons/{k}_{v}.png" for k, v in ITEM_IDS.items()},
        "skills": {str(k): f"UI/Icons/Skills/{k}_{v}.png" for k, v in SKILL_IDS.items()},
        "monsters": {str(k): f"Monsters/Icons/{k}_{v}.png" for k, v in MONSTER_IDS.items()},
        "elements": {str(i): f"UI/Icons/Elements/{i}_{e}.png" for i, e in enumerate(ELEMENT_NAMES)},
    }

    manifest_path = os.path.join(content_dir, "Data", "asset_manifest.json")
    os.makedirs(os.path.dirname(manifest_path), exist_ok=True)
    with open(manifest_path, "w") as f:
        json.dump(manifest, f, indent=2)
    print(f"\nAsset manifest written to: {manifest_path}")


def main():
    parser = argparse.ArgumentParser(description="Download Ragnarok Online assets for Ragna-TH")
    parser.add_argument("--all", action="store_true", help="Download all asset types")
    parser.add_argument("--items", action="store_true", help="Download item icons")
    parser.add_argument("--skills", action="store_true", help="Download skill icons")
    parser.add_argument("--monsters", action="store_true", help="Download monster sprites")
    parser.add_argument("--elements", action="store_true", help="Generate element icon placeholders")
    parser.add_argument("--collection", action="store_true", help="Also download item collection images")
    parser.add_argument("--output-dir", default=DEFAULT_CONTENT_DIR,
                        help=f"Content output directory (default: {DEFAULT_CONTENT_DIR})")

    args = parser.parse_args()

    if not any([args.all, args.items, args.skills, args.monsters, args.elements]):
        args.all = True

    content_dir = args.output_dir
    print(f"Ragna-TH Asset Downloader")
    print(f"Output: {content_dir}")

    total_success, total_fail = 0, 0

    if args.all or args.items:
        s, f = download_items(content_dir, args.collection)
        total_success += s
        total_fail += f

    if args.all or args.skills:
        s, f = download_skills(content_dir)
        total_success += s
        total_fail += f

    if args.all or args.monsters:
        s, f = download_monsters(content_dir)
        total_success += s
        total_fail += f

    if args.all or args.elements:
        s, f = generate_element_placeholders(content_dir)
        total_success += s
        total_fail += f

    write_asset_manifest(content_dir)

    print(f"\n{'='*50}")
    print(f"Total: {total_success} assets, {total_fail} failures")
    print(f"{'='*50}")

    return 0 if total_fail == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
