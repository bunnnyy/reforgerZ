#!/usr/bin/env bash
# ReforgerZ land survey on the dedicated server (no Workbench, no game client, no Workshop mods).
#
#   SERVER_DIR=... tools/land_survey/run.sh [arland] [everon] [cain]   # default: all three
#   CACHE_RUN=0 ...                                                      # skip the second (cached) boot
#
# For each map it boots a sub-scene of the base map with RZ_GameModeComponent on GameMode_Plain,
# the map's SCR_AIWorld (the road network comes with it) and Zombies > Tribes > Land survey on. A
# test-only script (files/scripts/Game/RZLandSurvey/RZL_Diag.c) waits for the land grid, prints
# what it found as "RZL ..." lines (cell size, read time, memory, wild km2, tribe slots at 1 per
# 4, 6 and 9 km2, rivers, lakes, bridges checked against the bridge prefabs on the map, ridge posts,
# valley lines, lair candidates, a tree histogram and a picture of the grid), then closes the
# server. A second boot of the same map loads the cached grid instead of reading it.
# One server at a time, about 1 to 3 GB of memory each; Everon and Cain take 6 to 10 minutes a boot.
# On Linux it runs the server in its own network namespace (unshare -n, as root) so its port can't
# collide with another server on the machine (the runtime test). Elsewhere it uses the server's
# default port like the runtime test, so don't run both at once.
# Nothing here is part of the mod: the files are copied into a throwaway addon folder.
# Windows (Git Bash): set SERVER_DIR to the Steam server folder (docs/PC_SETUP.md).
set -u
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
HERE="$ROOT/tools/land_survey"
WORK="${WORK_DIR:-$HOME/rz_check}"
SERVER="${SERVER_DIR:-$WORK/server}"
ADDON="$WORK/land_addons/ReforgerZ"
OUT="$WORK/land_survey"
GUID="$(sed -n 's/^ *GUID "\([0-9A-F]*\)".*/\1/p' "$ROOT/addon.gproj" | head -1)"
BIN="./ArmaReforgerServer"
[ -f "$SERVER/ArmaReforgerServer.exe" ] && BIN="./ArmaReforgerServer.exe"
[ -f "$SERVER/$BIN" ] || { echo "No server in $SERVER: run tools/compile_check.sh first (on Windows, install it from Steam and set SERVER_DIR)"; exit 2; }
MAPS="${*:-arland everon cain}"

rm -rf "$ADDON" && mkdir -p "$ADDON" "$OUT"
cp "$ROOT/addon.gproj" "$ADDON/"
cp -r "$ROOT/scripts" "$ROOT/Configs" "$ADDON/"
cp -r "$HERE/files/." "$ADDON/"

# Its own network namespace where possible (Linux, root): the server always listens on 2001.
WRAP=""
if command -v unshare > /dev/null 2>&1 && unshare -n true > /dev/null 2>&1; then
	WRAP="unshare -n"
fi
winpath() { if command -v cygpath > /dev/null 2>&1; then cygpath -w "$1"; else printf '%s' "$1"; fi; }
DIRS="$(winpath "$(dirname "$ADDON")")"
[ -n "${EXTRA_ADDONS_DIR:-}" ] && DIRS="$DIRS,$(winpath "$EXTRA_ADDONS_DIR")"

# boot MAP NAME PROFILE: one server run; prints the log path
boot() {
	local name="$2" profile="$3" rss="$OUT/$1_rss_$4.txt"
	rm -f "$rss"
	( cd "$SERVER" && $WRAP timeout "${TIMEOUT:-1500}" "$BIN" -addonsDir "$DIRS" -addons "$GUID" -profile "$profile" \
		-server "{7A1C2E3F4B5D6E80}Missions/$name.conf" -logStats 0 -maxFPS 30 -noBackend > /dev/null 2>&1 ) &
	local job=$!
	while kill -0 "$job" 2> /dev/null; do
		local pid
		pid="$(pgrep -n -f -- "-profile $profile")"	# the newest: the server, not its wrappers
		if [ -n "$pid" ] && [ -r "/proc/$pid/status" ]; then
			echo "$(date +%H:%M:%S) $(awk '/VmRSS/{print $2}' "/proc/$pid/status")" >> "$rss"
		fi
		sleep 2
	done
	wait "$job"
}

# rss_at FILE TIME BEFORE: the RSS (MB) sampled last before (1) or first after (0) TIME
rss_at() {
	awk -v t="$2" -v before="$3" '
		before == 1 && $1 <= t { v = $2 }
		before == 0 && $1 >= t && v == "" { v = $2 }
		END { if (v == "") print "?"; else printf "%d", v / 1024 }' "$1" 2> /dev/null
}

FAIL=0
for MAP in $MAPS; do
	case "$MAP" in
		arland) NAME=RZLand_Arland;;
		everon) NAME=RZLand_Everon;;
		cain) NAME=RZLand_Cain;;
		*) echo "Unknown map $MAP (arland, everon or cain)"; exit 2;;
	esac
	PROFILE="$WORK/land_profile_$MAP"
	rm -rf "$PROFILE"
	RUNS="build"
	[ "${CACHE_RUN:-1}" = "1" ] && RUNS="build cache"
	for RUN in $RUNS; do
		boot "$MAP" "$NAME" "$PROFILE" "$RUN"
		LOG="$(ls -td "$PROFILE"/logs/logs_* | head -1)/console.log"
		echo "== $MAP ($RUN boot) log: $LOG"
		EXC="$(grep -c 'Virtual Machine Exception' "$LOG")"
		echo "Script exceptions: $EXC"
		[ "$EXC" != "0" ] && { FAIL=1; grep -A6 'Virtual Machine Exception' "$LOG" | grep -E 'Reason|Function|scripts/' | sort | uniq -c | head -10; }
		grep -q "RZL done" "$LOG" || { echo "The land grid didn't finish (see the log)"; FAIL=1; }
		grep -E '\[ReforgerZ\] (Land|Settlement scan|Perimeter:|Territory:|Haven safehouses|Long Table:)' "$LOG" | sed 's/.*\[ReforgerZ\] /  /'
		grep -E 'RZL ' "$LOG" | grep -v 'RZL map |' | sed 's/.*RZL /  /'
		grep -E 'RZL map' "$LOG" | sed 's/.*RZL map //' > "$OUT/${MAP}_map.txt"
		RSS="$OUT/${MAP}_rss_$RUN.txt"
		if [ -s "$RSS" ]; then
			T0="$(grep -m1 'Land survey: reading the map' "$LOG" | cut -c1-8)"
			T1="$(grep -m1 '\[ReforgerZ\] Land: ' "$LOG" | cut -c1-8)"
			PEAK="$(awk '$2 > m { m = $2 } END { printf "%d", m / 1024 }' "$RSS")"
			if [ -n "$T0" ] && [ -n "$T1" ]; then
				echo "  server memory: $(rss_at "$RSS" "$T0" 1) MB before reading, $(rss_at "$RSS" "$T1" 0) MB after; peak $PEAK MB"
			else
				echo "  server memory: peak $PEAK MB"
			fi
		fi
		CACHE="$(ls "$PROFILE"/profile/ReforgerZ/land_*.json 2> /dev/null | head -1)"
		[ -n "$CACHE" ] && echo "  cache file: $(basename "$CACHE"), $(wc -c < "$CACHE") bytes"
	done
	echo "  map picture: $OUT/${MAP}_map.txt"
done
exit $FAIL
