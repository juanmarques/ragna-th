#!/usr/bin/env python3
"""
Ragnarok Online Complete Asset Downloader for Ragna-TH Project

Downloads the FULL set of Ragnarok Online assets from publicly available
community resources for a UE5-ready content pipeline.

Asset Categories:
    1. Item Icons          - Inventory icons (IDs 501-32000)
    2. Item Collection     - Paperdoll/preview images (IDs 501-32000)
    3. Skill Icons         - Skill bar icons (IDs 1-8500)
    4. Monster Sprites     - Static monster renders (IDs 1001-4100)
    5. Monster Spritesheets- Full animated spritesheets (IDs 1001-4100)
    6. NPC Sprites         - NPC character renders (IDs 46-500)
    7. Map Images          - World map/minimap images (named maps)
    8. Headgear Previews   - Male/female headgear paperdolls (IDs 1-2500)
    9. Job Class Icons     - Class/job selection icons (IDs 0-200)
    10. UI Assets          - Window chrome, buttons, cursors
    11. Element Icons      - Element attribute placeholders

Usage:
    python3 Tools/download_ro_assets.py --all              # Everything
    python3 Tools/download_ro_assets.py --items             # Item icons only
    python3 Tools/download_ro_assets.py --collections       # Item paperdolls
    python3 Tools/download_ro_assets.py --skills            # Skill icons
    python3 Tools/download_ro_assets.py --monsters          # Monster sprites
    python3 Tools/download_ro_assets.py --spritesheets      # Monster spritesheets
    python3 Tools/download_ro_assets.py --npcs              # NPC sprites
    python3 Tools/download_ro_assets.py --maps              # Map images
    python3 Tools/download_ro_assets.py --headgear          # Headgear previews
    python3 Tools/download_ro_assets.py --jobs              # Job class icons
    python3 Tools/download_ro_assets.py --ui                # UI window assets
    python3 Tools/download_ro_assets.py --elements          # Element placeholders
    python3 Tools/download_ro_assets.py --workers 8         # Parallel workers

Sources:
    - static.divine-pride.net (sprites, icons, maps)
    - www.divine-pride.net (headgear previews)
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

# ============================================================================
# CDN URL patterns
# ============================================================================

# Items
ITEM_ICON_URL = "https://static.divine-pride.net/images/items/item/{id}.png"
ITEM_COLLECTION_URL = "https://static.divine-pride.net/images/items/collection/{id}.png"

# Skills
SKILL_ICON_URL = "https://static.divine-pride.net/images/skills/{id}.png"

# Monsters & NPCs (same CDN path, different ID ranges)
MONSTER_IMAGE_URL = "https://static.divine-pride.net/images/mobs/png/{id}.png"
MONSTER_SPRITESHEET_URL = "https://static.divine-pride.net/images/spritesheets/npc/{id}.png"
NPC_IMAGE_URL = "https://static.divine-pride.net/images/mobs/png/{id}.png"

# Maps
MAP_IMAGE_URL = "https://static.divine-pride.net/images/maps/original/{id}.png"

# Headgear (male + female paperdolls)
HEADGEAR_MALE_URL = "https://www.divine-pride.net/headgear/male/{id}.png"
HEADGEAR_FEMALE_URL = "https://www.divine-pride.net/headgear/female/{id}.png"

# Job class icons
JOB_ICON_URL = "https://static.divine-pride.net/images/jobs/icon_jobs_{id}.png"

# Default output relative to project root
DEFAULT_CONTENT_DIR = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "RagnarokUE", "Content"
)

# ============================================================================
# Complete Ragnarok Online ID ranges
# ============================================================================

# Items: 501-32000 covers all classic through renewal items
ITEM_RANGES = [
    (501, 1000),      # Usable items (potions, herbs, cooking materials)
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

# Skills: 1-8500 covers all skills through 4th jobs
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

# Monsters: 1001-4100 covers classic + renewal mobs
MONSTER_RANGES = [
    (1001, 2000),     # Classic monsters (Prontera, Payon, etc.)
    (2000, 2500),     # Episode monsters (Rachel, Veins, etc.)
    (2500, 3000),     # Bio Lab, Instances
    (3000, 3600),     # Renewal monsters
    (3600, 4100),     # Extended renewal
]

# NPCs: IDs below monsters, plus some overlapping ranges
NPC_RANGES = [
    (46, 125),        # Basic NPCs (merchants, Kafra, quest givers)
    (125, 250),       # Extended NPCs
    (250, 500),       # Episode NPCs
    (700, 1000),      # Special NPCs / pets
    (4200, 4500),     # Newer NPCs
    (10000, 10200),   # Event NPCs
    (20000, 20100),   # Custom server NPCs
]

# Headgear: IDs 1-2500 cover all headgear view sprites
HEADGEAR_RANGES = [
    (1, 500),         # Classic headgears
    (500, 1000),      # Episode headgears
    (1000, 1500),     # Renewal headgears
    (1500, 2000),     # Extended renewal
    (2000, 2500),     # Latest headgears
]

# Job class icon IDs (0-200 covers all job classes)
JOB_RANGES = [
    (0, 30),          # 1st classes (Novice, Swordman, Mage, etc.)
    (30, 50),         # 2nd classes (Knight, Wizard, etc.)
    (50, 80),         # Rebirth classes (Lord Knight, High Wizard, etc.)
    (80, 120),        # 3rd classes (Rune Knight, Warlock, etc.)
    (120, 160),       # 4th classes (Dragon Knight, Archmage, etc.)
    (160, 200),       # Extended / alternate job classes
]

# Complete list of known Ragnarok Online maps
RO_MAP_NAMES = [
    # === Major Towns ===
    "prontera", "geffen", "payon", "morroc", "alberta", "izlude",
    "aldebaran", "comodo", "juno", "yuno", "amatsu", "kunlun", "gonryun",
    "louyang", "ayothaya", "umbala", "niflheim", "lutie", "jawaii",
    "einbroch", "einbech", "lighthalzen", "hugel", "rachel", "veins",
    "moscovia", "brasilis", "dewata", "eclage", "malangdo", "malaya",
    "mora", "dicastes01", "dicastes02", "splendide", "manuk",
    "mid_camp", "lasagna",

    # === Prontera Region ===
    "prt_fild00", "prt_fild01", "prt_fild02", "prt_fild03", "prt_fild04",
    "prt_fild05", "prt_fild06", "prt_fild07", "prt_fild08", "prt_fild09",
    "prt_fild10", "prt_fild11",
    "prt_maze01", "prt_maze02", "prt_maze03",
    "prt_sewb1", "prt_sewb2", "prt_sewb3", "prt_sewb4",
    "prt_monk", "prt_church",
    "prt_in", "prt_castle",

    # === Geffen Region ===
    "gef_fild00", "gef_fild01", "gef_fild02", "gef_fild03", "gef_fild04",
    "gef_fild05", "gef_fild06", "gef_fild07", "gef_fild08", "gef_fild09",
    "gef_fild10", "gef_fild11", "gef_fild12", "gef_fild13", "gef_fild14",
    "gef_tower", "gef_dun00", "gef_dun01", "gef_dun02", "gef_dun03",

    # === Payon Region ===
    "pay_fild01", "pay_fild02", "pay_fild03", "pay_fild04",
    "pay_fild05", "pay_fild06", "pay_fild07", "pay_fild08",
    "pay_fild09", "pay_fild10", "pay_fild11",
    "pay_dun00", "pay_dun01", "pay_dun02", "pay_dun03", "pay_dun04",
    "pay_arche",

    # === Morroc Region ===
    "moc_fild01", "moc_fild02", "moc_fild03", "moc_fild04",
    "moc_fild05", "moc_fild06", "moc_fild07", "moc_fild08",
    "moc_fild09", "moc_fild10", "moc_fild11", "moc_fild12",
    "moc_fild13", "moc_fild14", "moc_fild15", "moc_fild16",
    "moc_fild17", "moc_fild18", "moc_fild19", "moc_fild20",
    "moc_fild21", "moc_fild22",
    "moc_ruins", "moc_pryd01", "moc_pryd02", "moc_pryd03",
    "moc_pryd04", "moc_pryd05", "moc_pryd06",
    "moc_prydb1",
    "in_sphinx1", "in_sphinx2", "in_sphinx3", "in_sphinx4", "in_sphinx5",

    # === Alberta / Izlude Region ===
    "alb_ship",
    "iz_dun00", "iz_dun01", "iz_dun02", "iz_dun03", "iz_dun04", "iz_dun05",
    "treasure01", "treasure02",

    # === Aldebaran Region ===
    "alde_dun01", "alde_dun02", "alde_dun03", "alde_dun04",
    "c_tower1", "c_tower2", "c_tower3", "c_tower4",
    "alde_alche",

    # === Comodo Region ===
    "cmd_fild01", "cmd_fild02", "cmd_fild03", "cmd_fild04",
    "cmd_fild05", "cmd_fild06", "cmd_fild07", "cmd_fild08", "cmd_fild09",
    "beach_dun", "beach_dun2", "beach_dun3",

    # === Juno / Yuno Region ===
    "yuno_fild01", "yuno_fild02", "yuno_fild03", "yuno_fild04",
    "yuno_fild05", "yuno_fild06", "yuno_fild07", "yuno_fild08",
    "yuno_fild09", "yuno_fild10", "yuno_fild11", "yuno_fild12",
    "yuno_in01", "yuno_in02", "yuno_in03", "yuno_in04", "yuno_in05",
    "mag_dun01", "mag_dun02",

    # === Einbroch / Lighthalzen Region ===
    "ein_fild01", "ein_fild02", "ein_fild03", "ein_fild04",
    "ein_fild05", "ein_fild06", "ein_fild07", "ein_fild08",
    "ein_fild09", "ein_fild10",
    "ein_dun01", "ein_dun02",
    "lhz_fild01", "lhz_fild02", "lhz_fild03",
    "lhz_dun01", "lhz_dun02", "lhz_dun03", "lhz_dun04",
    "lhz_cube",

    # === Rachel / Veins Region ===
    "ra_fild01", "ra_fild02", "ra_fild03", "ra_fild04",
    "ra_fild05", "ra_fild06", "ra_fild07", "ra_fild08",
    "ra_fild09", "ra_fild10", "ra_fild11", "ra_fild12", "ra_fild13",
    "ra_san01", "ra_san02", "ra_san03", "ra_san04", "ra_san05",
    "ra_temin",
    "ve_fild01", "ve_fild02", "ve_fild03", "ve_fild04",
    "ve_fild05", "ve_fild06", "ve_fild07",

    # === Hugel Region ===
    "hu_fild01", "hu_fild02", "hu_fild03", "hu_fild04",
    "hu_fild05", "hu_fild06", "hu_fild07",
    "odin_tem01", "odin_tem02", "odin_tem03",
    "kh_dun01", "kh_dun02",

    # === Amatsu / Kunlun / Louyang ===
    "ama_fild01",
    "ama_dun01", "ama_dun02", "ama_dun03",
    "gon_fild01",
    "gon_dun01", "gon_dun02", "gon_dun03",
    "lou_fild01",
    "lou_dun01", "lou_dun02", "lou_dun03",

    # === Ayothaya / Moscovia / Brasilis ===
    "ayo_fild01", "ayo_fild02",
    "ayo_dun01", "ayo_dun02",
    "mosk_fild01", "mosk_fild02",
    "mosk_dun01", "mosk_dun02", "mosk_dun03",
    "bra_fild01",
    "bra_dun01", "bra_dun02",

    # === Niflheim / Umbala ===
    "nif_fild01", "nif_fild02",
    "niflheim",
    "um_fild01", "um_fild02", "um_fild03", "um_fild04",
    "um_dun01", "um_dun02",

    # === GvG / War of Emperium Castles ===
    "prtg_cas01", "prtg_cas02", "prtg_cas03", "prtg_cas04", "prtg_cas05",
    "payg_cas01", "payg_cas02", "payg_cas03", "payg_cas04", "payg_cas05",
    "gefg_cas01", "gefg_cas02", "gefg_cas03", "gefg_cas04", "gefg_cas05",
    "aldeg_cas01", "aldeg_cas02", "aldeg_cas03", "aldeg_cas04", "aldeg_cas05",
    "arug_cas01", "arug_cas02", "arug_cas03", "arug_cas04", "arug_cas05",
    "schg_cas01", "schg_cas02", "schg_cas03", "schg_cas04", "schg_cas05",

    # === Dungeons ===
    "gl_cas01", "gl_cas02",
    "gl_church", "gl_chyard", "gl_dun01", "gl_dun02",
    "gl_knt01", "gl_knt02", "gl_prison", "gl_prison1",
    "gl_sew01", "gl_sew02", "gl_sew03", "gl_sew04",
    "gl_step",
    "xmas_dun01", "xmas_dun02",
    "xmas_fild01",
    "tur_dun01", "tur_dun02", "tur_dun03", "tur_dun04",
    "tur_dun05", "tur_dun06",
    "anthell01", "anthell02",
    "mjolnir_01", "mjolnir_02", "mjolnir_03", "mjolnir_04",
    "mjolnir_05", "mjolnir_06", "mjolnir_07", "mjolnir_08",
    "mjolnir_09", "mjolnir_10", "mjolnir_11", "mjolnir_12",

    # === Instances / Special ===
    "1@tower", "2@tower", "3@tower", "4@tower", "5@tower", "6@tower",
    "1@cata", "2@cata",
    "1@pump", "2@pump",
    "ecl_tdun01", "ecl_tdun02", "ecl_tdun03", "ecl_tdun04",
    "dew_fild01",
    "dew_dun01", "dew_dun02",
    "mal_dun01",
    "ma_fild01", "ma_fild02",
    "ma_dun01",
    "mora_dun01", "mora_dun02",
    "dic_fild01", "dic_fild02",
    "dic_dun01", "dic_dun02", "dic_dun03",
    "spl_fild01", "spl_fild02", "spl_fild03",
    "man_fild01", "man_fild02", "man_fild03",

    # === PvP / Arena ===
    "pvp_y_room", "pvp_n_room",
    "arena_room", "force_map1", "force_map2", "force_map3",

    # === Renewal / Episode 14+ ===
    "rockmi1", "rockmi2",
    "rockrdg1", "rockrdg2",
    "har_in01",
    "moro_vol", "moro_cav",
    "abbey01", "abbey02", "abbey03",
    "nameless_n", "nameless_i", "nameless_in",
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
    """Download assets for a range of numeric IDs using parallel workers."""
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


def download_named_list(url_template, output_dir, names, category_name,
                        workers=4, rate_limit=0.05):
    """Download assets for a list of string names using parallel workers."""
    success, skip, not_found = 0, 0, 0

    def fetch_one(name):
        filename = f"{name}.png"
        path = os.path.join(output_dir, filename)

        if os.path.exists(path):
            return ("skip", name)

        url = url_template.format(id=name)
        if download_file(url, path):
            return ("ok", name)
        else:
            return ("miss", name)

    total = len(names)
    batch_size = 50
    current = 0

    while current < total:
        batch_end = min(current + batch_size, total)
        batch = names[current:batch_end]

        with ThreadPoolExecutor(max_workers=workers) as executor:
            futures = {executor.submit(fetch_one, n): n for n in batch}
            for future in as_completed(futures):
                status, name = future.result()
                if status == "ok":
                    success += 1
                elif status == "skip":
                    skip += 1
                else:
                    not_found += 1

        done = min(batch_end, total)
        with _print_lock:
            print(f"\r  {category_name}: {done}/{total} scanned "
                  f"({success} new, {skip} cached, {not_found} N/A)", end="", flush=True)

        current = batch_end
        time.sleep(rate_limit)

    print()
    return success, 0, skip, not_found


def _print_header(title):
    """Print a section header."""
    print(f"\n{'='*60}")
    print(f"  {title}")
    print(f"{'='*60}")


def _download_id_category(url_template, output_dir, ranges, title, workers):
    """Generic helper for downloading a category by ID ranges."""
    _print_header(title)
    total_success, total_skip, total_na = 0, 0, 0

    for start, end in ranges:
        print(f"\n  Range {start}-{end}:")
        s, _, sk, na = download_range(
            url_template, output_dir, start, end,
            f"{title.split()[-1]} {start}-{end}", workers
        )
        total_success += s
        total_skip += sk
        total_na += na

    print(f"\n  {title} total: {total_success} new + {total_skip} cached "
          f"({total_na} not available)")
    return total_success + total_skip, 0


# ============================================================================
# Download functions for each asset category
# ============================================================================

def download_items(content_dir, workers=4):
    """Download all item icons across full RO ID ranges."""
    return _download_id_category(
        ITEM_ICON_URL,
        os.path.join(content_dir, "Items", "Icons"),
        ITEM_RANGES,
        "ITEM ICONS",
        workers
    )


def download_collections(content_dir, workers=4):
    """Download all item collection/paperdoll preview images."""
    return _download_id_category(
        ITEM_COLLECTION_URL,
        os.path.join(content_dir, "Items", "Collection"),
        ITEM_RANGES,
        "ITEM COLLECTION PREVIEWS",
        workers
    )


def download_skills(content_dir, workers=4):
    """Download all skill icons."""
    return _download_id_category(
        SKILL_ICON_URL,
        os.path.join(content_dir, "UI", "Icons", "Skills"),
        SKILL_RANGES,
        "SKILL ICONS",
        workers
    )


def download_monsters(content_dir, workers=4):
    """Download all monster static sprites."""
    return _download_id_category(
        MONSTER_IMAGE_URL,
        os.path.join(content_dir, "Monsters", "Icons"),
        MONSTER_RANGES,
        "MONSTER SPRITES",
        workers
    )


def download_monster_spritesheets(content_dir, workers=4):
    """Download all monster animated spritesheets."""
    return _download_id_category(
        MONSTER_SPRITESHEET_URL,
        os.path.join(content_dir, "Monsters", "Spritesheets"),
        MONSTER_RANGES,
        "MONSTER SPRITESHEETS",
        workers
    )


def download_npcs(content_dir, workers=4):
    """Download all NPC sprites."""
    return _download_id_category(
        NPC_IMAGE_URL,
        os.path.join(content_dir, "Characters", "NPCs"),
        NPC_RANGES,
        "NPC SPRITES",
        workers
    )


def download_maps(content_dir, workers=4):
    """Download all map images by name."""
    map_dir = os.path.join(content_dir, "Maps", "Images")
    _print_header("MAP IMAGES")

    s, _, sk, na = download_named_list(
        MAP_IMAGE_URL, map_dir, RO_MAP_NAMES,
        "Maps", workers
    )

    print(f"\n  Maps total: {s} new + {sk} cached ({na} not available)")
    return s + sk, 0


def download_headgear(content_dir, workers=4):
    """Download headgear preview images for both genders."""
    _print_header("HEADGEAR PREVIEWS")
    total_success, total_skip, total_na = 0, 0, 0

    # Male headgears
    male_dir = os.path.join(content_dir, "Characters", "Headgear", "Male")
    print("\n  Male headgears:")
    for start, end in HEADGEAR_RANGES:
        print(f"\n  Range {start}-{end}:")
        s, _, sk, na = download_range(
            HEADGEAR_MALE_URL, male_dir, start, end,
            f"Male HG {start}-{end}", workers
        )
        total_success += s
        total_skip += sk
        total_na += na

    # Female headgears
    female_dir = os.path.join(content_dir, "Characters", "Headgear", "Female")
    print("\n  Female headgears:")
    for start, end in HEADGEAR_RANGES:
        print(f"\n  Range {start}-{end}:")
        s, _, sk, na = download_range(
            HEADGEAR_FEMALE_URL, female_dir, start, end,
            f"Female HG {start}-{end}", workers
        )
        total_success += s
        total_skip += sk
        total_na += na

    print(f"\n  Headgear total: {total_success} new + {total_skip} cached "
          f"({total_na} not available)")
    return total_success + total_skip, 0


def download_jobs(content_dir, workers=4):
    """Download job class icons."""
    return _download_id_category(
        JOB_ICON_URL,
        os.path.join(content_dir, "Characters", "Jobs"),
        JOB_RANGES,
        "JOB CLASS ICONS",
        workers
    )


def download_ui_assets(content_dir, workers=4):
    """Download UI window/button assets from divine-pride static CDN."""
    ui_dir = os.path.join(content_dir, "UI", "Windows")
    _print_header("UI ASSETS")

    # Known UI asset filenames from divine-pride CDN
    ui_assets = [
        # Window chrome
        ("CUISkillDelayInfoTitleBar", "https://static.divine-pride.net/images/CUISkillDelayInfoTitleBar.png"),
        # Logos / branding (useful as placeholder textures)
        ("gravity_logo", "https://static.divine-pride.net/images/gravity.png"),
        ("page_error", "https://static.divine-pride.net/images/page_error.png"),
    ]

    # Also try common RO UI element patterns
    ui_patterns = [
        "https://static.divine-pride.net/images/display_mapname/village.png",
        "https://static.divine-pride.net/images/display_mapname/field.png",
        "https://static.divine-pride.net/images/display_mapname/dungeon.png",
        "https://static.divine-pride.net/images/display_mapname/city.png",
    ]

    success, skip = 0, 0
    os.makedirs(ui_dir, exist_ok=True)

    for name, url in ui_assets:
        path = os.path.join(ui_dir, f"{name}.png")
        if os.path.exists(path):
            print(f"  [SKIP] {name}.png")
            skip += 1
            continue
        if download_file(url, path):
            print(f"  [OK]   {name}.png")
            success += 1
        else:
            print(f"  [MISS] {name}.png")

    # Map display name images
    display_dir = os.path.join(content_dir, "UI", "MapDisplay")
    os.makedirs(display_dir, exist_ok=True)
    for url in ui_patterns:
        name = url.split("/")[-1].replace(".png", "")
        path = os.path.join(display_dir, f"{name}.png")
        if os.path.exists(path):
            skip += 1
            continue
        if download_file(url, path):
            print(f"  [OK]   display/{name}.png")
            success += 1

    print(f"\n  UI total: {success} new + {skip} cached")
    return success + skip, 0


def generate_element_placeholders(content_dir, workers=4):
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
    _print_header("ELEMENT ICON PLACEHOLDERS")

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


# ============================================================================
# Manifest generation
# ============================================================================

def _scan_dir(base_dir, subdir, manifest_key, manifest):
    """Scan a directory for PNGs and add to manifest."""
    full_dir = os.path.join(base_dir, *subdir.split("/"))
    if not os.path.isdir(full_dir):
        return
    for f in os.listdir(full_dir):
        if f.endswith(".png"):
            asset_id = f.split("_")[0] if "_" in f else f.replace(".png", "")
            manifest[manifest_key][asset_id] = f"{subdir}/{f}"


def write_asset_manifest(content_dir):
    """Scan all downloaded PNGs and write the comprehensive asset manifest."""
    manifest = {
        "items": {},
        "item_collections": {},
        "skills": {},
        "monsters": {},
        "monster_spritesheets": {},
        "npcs": {},
        "maps": {},
        "headgear_male": {},
        "headgear_female": {},
        "jobs": {},
        "ui": {},
        "elements": {},
    }

    # Scan all asset directories
    _scan_dir(content_dir, "Items/Icons", "items", manifest)
    _scan_dir(content_dir, "Items/Collection", "item_collections", manifest)
    _scan_dir(content_dir, "UI/Icons/Skills", "skills", manifest)
    _scan_dir(content_dir, "Monsters/Icons", "monsters", manifest)
    _scan_dir(content_dir, "Monsters/Spritesheets", "monster_spritesheets", manifest)
    _scan_dir(content_dir, "Characters/NPCs", "npcs", manifest)
    _scan_dir(content_dir, "Maps/Images", "maps", manifest)
    _scan_dir(content_dir, "Characters/Headgear/Male", "headgear_male", manifest)
    _scan_dir(content_dir, "Characters/Headgear/Female", "headgear_female", manifest)
    _scan_dir(content_dir, "Characters/Jobs", "jobs", manifest)
    _scan_dir(content_dir, "UI/Windows", "ui", manifest)
    _scan_dir(content_dir, "UI/MapDisplay", "ui", manifest)
    _scan_dir(content_dir, "UI/Icons/Elements", "elements", manifest)

    manifest_path = os.path.join(content_dir, "Data", "asset_manifest.json")
    os.makedirs(os.path.dirname(manifest_path), exist_ok=True)
    with open(manifest_path, "w") as f:
        json.dump(manifest, f, indent=2, sort_keys=True)

    total = sum(len(v) for v in manifest.values())
    print(f"\n{'='*60}")
    print(f"  ASSET MANIFEST: {total} total entries")
    print(f"{'='*60}")
    for key, entries in sorted(manifest.items()):
        if entries:
            print(f"  {key:25s} {len(entries):>6,}")
    print(f"  {'─'*35}")
    print(f"  {'TOTAL':25s} {total:>6,}")


def main():
    parser = argparse.ArgumentParser(
        description="Download COMPLETE Ragnarok Online assets for Ragna-TH UE5 pipeline")

    parser.add_argument("--all", action="store_true",
                        help="Download ALL asset types (full pipeline)")
    parser.add_argument("--items", action="store_true",
                        help="Item icons (IDs 501-32000)")
    parser.add_argument("--collections", action="store_true",
                        help="Item collection/paperdoll previews (IDs 501-32000)")
    parser.add_argument("--skills", action="store_true",
                        help="Skill icons (IDs 1-8500)")
    parser.add_argument("--monsters", action="store_true",
                        help="Monster static sprites (IDs 1001-4100)")
    parser.add_argument("--spritesheets", action="store_true",
                        help="Monster animated spritesheets (IDs 1001-4100)")
    parser.add_argument("--npcs", action="store_true",
                        help="NPC sprites (IDs 46-20100)")
    parser.add_argument("--maps", action="store_true",
                        help="Map/minimap images (~400 maps)")
    parser.add_argument("--headgear", action="store_true",
                        help="Headgear paperdoll previews M/F (IDs 1-2500)")
    parser.add_argument("--jobs", action="store_true",
                        help="Job class icons (IDs 0-200)")
    parser.add_argument("--ui", action="store_true",
                        help="UI window chrome and display assets")
    parser.add_argument("--elements", action="store_true",
                        help="Element icon placeholders")
    parser.add_argument("--workers", type=int, default=4,
                        help="Parallel download workers (default: 4)")
    parser.add_argument("--output-dir", default=DEFAULT_CONTENT_DIR,
                        help="Content output directory")

    args = parser.parse_args()

    all_flags = [
        args.items, args.collections, args.skills, args.monsters,
        args.spritesheets, args.npcs, args.maps, args.headgear,
        args.jobs, args.ui, args.elements,
    ]
    if not any([args.all] + all_flags):
        args.all = True

    content_dir = args.output_dir
    workers = args.workers

    print(f"Ragna-TH Complete Asset Downloader v2.0")
    print(f"Output: {content_dir}")
    print(f"Workers: {workers}")

    total_success, total_fail = 0, 0

    categories = [
        (args.items, download_items),
        (args.collections, download_collections),
        (args.skills, download_skills),
        (args.monsters, download_monsters),
        (args.spritesheets, download_monster_spritesheets),
        (args.npcs, download_npcs),
        (args.maps, download_maps),
        (args.headgear, download_headgear),
        (args.jobs, download_jobs),
        (args.ui, download_ui_assets),
        (args.elements, generate_element_placeholders),
    ]

    for flag, func in categories:
        if args.all or flag:
            s, f = func(content_dir, workers)
            total_success += s
            total_fail += f

    write_asset_manifest(content_dir)

    print(f"\n{'='*60}")
    print(f"  COMPLETE: {total_success:,} total assets available")
    print(f"{'='*60}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
