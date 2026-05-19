import requests
from bs4 import BeautifulSoup
import hashlib
import json
import os
from datetime import datetime

# =========================
# CONFIG
# =========================

WEBHOOK_URL = "YOUR_DISCORD_WEBHOOK_URL_HERE"

SOURCES = {
    "Flecs": "https://www.flecs.dev/flecs/",
    "Entt": "https://github.com/skypjack/entt",
    "Stingray": "https://bitsquid.blogspot.com/",
    "JoltPhysics": "https://jrouwe.github.io/JoltPhysics/",
    "ImGui": "https://github.com/ocornut/imgui/releases",
    "Tracy": "https://github.com/wolfpld/tracy/releases",
}

DATA_FILE = "saved_hashes.json"

# =========================
# LOAD PREVIOUS HASHES
# =========================

if os.path.exists(DATA_FILE):
    with open(DATA_FILE, "r", encoding="utf-8") as f:
        saved_hashes = json.load(f)
else:
    saved_hashes = {}

# =========================
# HELPERS
# =========================


def get_page_text(url):
    headers = {
        "User-Agent": "Mozilla/5.0"
    }

    response = requests.get(url, headers=headers, timeout=30)
    response.raise_for_status()

    soup = BeautifulSoup(response.text, "html.parser")

    # Remove scripts/styles
    for tag in soup(["script", "style", "noscript"]):
        tag.decompose()

    text = soup.get_text(separator="\n")

    # Clean lines
    lines = [line.strip() for line in text.splitlines()]
    lines = [line for line in lines if line]

    return "\n".join(lines)


def hash_text(text):
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def send_discord_message(source_name, url, text):
    max_length = 1800

    shortened = text[:max_length]

    payload = {
        "content": (
            f"## Source Updated: {source_name}\n"
            f"URL: {url}\n"
            f"Detected at: {datetime.now()}\n\n"
            f"```txt\n{shortened}\n```"
        )
    }

    response = requests.post(WEBHOOK_URL, json=payload)

    if response.status_code not in [200, 204]:
        print(f"Failed to send Discord message: {response.text}")

# =========================
# MAIN
# =========================


changes_detected = False

for source_name, url in SOURCES.items():
    try:
        print(f"Checking {source_name}...")

        text = get_page_text(url)
        current_hash = hash_text(text)

        previous_hash = saved_hashes.get(source_name)

        if previous_hash != current_hash:
            print(f"Change detected for {source_name}")

            send_discord_message(source_name, url, text)

            saved_hashes[source_name] = current_hash
            changes_detected = True
        else:
            print(f"No changes for {source_name}")

    except Exception as e:
        print(f"Error checking {source_name}: {e}")

# =========================
# SAVE HASHES
# =========================

if changes_detected:
    with open(DATA_FILE, "w", encoding="utf-8") as f:
        json.dump(saved_hashes, f, indent=4)

print("Done.")
