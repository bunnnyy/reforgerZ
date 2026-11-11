#!/usr/bin/env bash
# ReforgerZ zombie lab: real Bacon Zombies AI on the dedicated server, measured (test only, not the mod).
#
#   SERVER_DIR=... EXTRA_ADDONS_DIR=<folder holding BaconZombies_622120A5448725E3> tools/zombie_lab/run.sh [seconds]
#   QUICK=1      a short debug run (about 12 minutes) instead of the full one (about 50)
#   MAX_FPS=240  the server's frame cap (frame times under 1000/MAX_FPS ms read as the cap)
#   ISOLATE=0    don't run the server in its own network namespace (see below)
#
# Boots a sub-scene of Everon (files/Worlds/ZLab) with Bacon Zombies loaded and the lab component on
# a plain game mode. The lab finds open fields and wood edges far apart on the map and runs, in one
# session, at different places at once: runtime groups (AddAIEntityToGroup), the waypoint leak in
# RZ_Util, one group on waypoints for 30 minutes, speeds per prefab, perception distances by day, night
# and fog with perception factors, target sharing and memory, gunfire, health and removal, the game
# clock, and last the server frame time at 0/60/100/140 agents. Then a second short boot reads the
# clock after the restart. It prints the results as tables (report.py).
# Infected are forced to AI LOD 0, as if a player were near: with no player on the server they'd
# otherwise sit at the lowest LOD. The target is a spawned civilian (Bacon's faction is hostile to every
# faction, CIV included). Nothing here is part of the mod: the files are copied into a throwaway
# addon folder (with Bacon Zombies added to its dependencies).
# Started with -server, the server always listens on UDP 2001 (it ignores -bindPort), so on Linux the
# lab runs it in a private network namespace (unshare -n, as root) and it can't clash with another
# server on this machine. Elsewhere, stop any other server first.
# Windows (Git Bash): set SERVER_DIR to the Steam server folder; needs Python 3 (docs/PC_SETUP.md).
set -u
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
HERE="$ROOT/tools/zombie_lab"
WORK="${WORK_DIR:-$HOME/rz_check}"
SERVER="${SERVER_DIR:-$WORK/server}"
ADDON="$WORK/zlab_addons/ReforgerZ"
PROFILE="$WORK/zlab_profile"
GUID="$(sed -n 's/^ *GUID "\([0-9A-F]*\)".*/\1/p' "$ROOT/addon.gproj" | head -1)"
BACON="622120A5448725E3"
BIN="./ArmaReforgerServer"
[ -f "$SERVER/ArmaReforgerServer.exe" ] && BIN="./ArmaReforgerServer.exe"
[ -f "$SERVER/$BIN" ] || { echo "No server in $SERVER: run tools/compile_check.sh first (on Windows, install it from Steam and set SERVER_DIR)"; exit 2; }
[ -n "${EXTRA_ADDONS_DIR:-}" ] && ls "$EXTRA_ADDONS_DIR" | grep -q "$BACON" || { echo "Set EXTRA_ADDONS_DIR to the folder holding Bacon Zombies ($BACON)"; exit 2; }
PY="${PYTHON:-}"
if [ -z "$PY" ]; then
	for p in python3 python py; do
		"$p" -c 'import sys; sys.exit(sys.version_info[0] != 3)' > /dev/null 2>&1 && { PY="$p"; break; }
	done
fi
[ -n "$PY" ] || { echo "No Python 3 found (set PYTHON=...)"; exit 2; }

rm -rf "$ADDON" "$PROFILE/profile" "$PROFILE/logs" && mkdir -p "$ADDON" "$PROFILE/profile"
cp "$ROOT/addon.gproj" "$ADDON/"
grep -q "$BACON" "$ADDON/addon.gproj" || sed -i "s/^\( *\)\"58D0FB3206B6F859\"/&\n\1\"$BACON\"/" "$ADDON/addon.gproj"
cp -r "$ROOT/scripts" "$ROOT/Configs" "$ADDON/"
cp -r "$HERE/files/." "$ADDON/"
[ "${QUICK:-0}" = 1 ] && touch "$PROFILE/profile/zlab_quick.txt"

ISO=""
if [ "${ISOLATE:-1}" = 1 ] && command -v unshare > /dev/null 2>&1 && unshare -n true 2> /dev/null; then
	ISO="unshare -n"
fi

cd "$SERVER"
winpath() { if command -v cygpath > /dev/null 2>&1; then cygpath -w "$1"; else printf '%s' "$1"; fi; }
DIRS="$(winpath "$(dirname "$ADDON")"),$(winpath "$EXTRA_ADDONS_DIR")"
boot() {
	timeout "$1" $ISO "$BIN" -addonsDir "$DIRS" -addons "$GUID" -profile "$PROFILE" \
		-server "{7A1C2E3F4B5D6E72}Missions/ZLab.conf" -logStats 0 -maxFPS "${MAX_FPS:-240}" -noBackend > /dev/null 2>&1
	ls -td "$PROFILE"/logs/logs_* | head -1
}
LOG1="$(boot "${1:-4800}")/console.log"
# The second boot only reads the game clock after the restart
touch "$PROFILE/profile/zlab_restart.txt"
LOG2="$(boot 300)/console.log"
"$PY" "$HERE/report.py" "$LOG1" "$LOG2"
