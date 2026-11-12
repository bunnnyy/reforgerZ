#!/usr/bin/env bash
# ReforgerZ runtime test on the dedicated server (no Workbench, no game client).
#
#   SERVER_DIR=... tools/runtime_test/run.sh [seconds]     # default 5400 (all missions, all questlines, then the route)
#   SERVER_DIR=... tools/runtime_test/run.sh sites [arland] [everon] [cain]   # the site audit only (default arland)
#   EXTRA_ADDONS_DIR=...                                   # a folder holding the Workshop dependencies in addon.gproj
#
# The site audit (docs/SITES.md, "The audit"): "sites" boots each map's own world (a sub-scene with
# the map's AI world, so its roads, and camp life's kits from RZ_GameMode.et), waits for the land
# grid, measures every camp-kit prop's box (FOOTPRINTS=1 writes them to tools/camp_kit/footprints.json),
# lays out every holding, safehouse and road post as data (RZ_CampLife.AuditAll: nothing spawns),
# prints each site's "Sites:" line and the summary, and closes. The log's "Sites draw" lines are what
# tools/camp_kit/render.py --placed draws. It fails if a stronghold scores under Audit threshold.
# Each boot runs in its own network namespace where it can (Linux, root), so it can run beside a full
# test; Everon takes 10 to 20 minutes. The full test runs the same audit alongside its route (camp
# life's kits are in its world too) and prints the summary.
#
# Needs the server installed (tools/compile_check.sh installs it). Boots a sub-scene of Arland
# with RZ_GameModeComponent on GameMode_Plain, vanilla stand-ins for the minimum prefab slots
# (an FIA group as "zombies", a crate as the reward crate...), and a test-only harness that:
#   - adds a fake player (id 901, a spawned civilian) to the players every system sees;
#   - runs every conversation's nodes once for that player, and a lie, a promise and a callback
#     ("RZT Talk test: ..."), and walks it through reputation's cells, the credit rule and the
#     forgiveness budget ("RZT Reputation test: ..."); from minute 2 it cycles Voice > Force mood
#     through the five moods, 5 minutes each, and prints each one's picks ("RZT voice <mood> ...");
#   - starts every registered mission type one at a time, moves the player there, then ends it
#     (alternately won and failed) so Setup, Update, rewards and cleanup all run;
#   - forces what missions need (a spread of town infection before each, a few goes at a Setup
#     that misses, stand-ins for the optional prefabs, trucks for the convoys and Mobile Lab;
#     RZT_Prep.c);
#   - then forces each registered questline's start conditions (RZT_Prep.c), begins it, fast-forwards
#     its waits, goes to each stage mission and ends it as won (with the outcome that moves the
#     story on), and reports which reached an ending, fizzled or got stuck;
#   - walks the player through every settlement, the safehouses, the lab, then into the
#     Perimeter's warning and kill zones (until then the line leaves the fake player alone);
#     kills them if the Perimeter didn't; respawns them;
#   - last, a scripted cure finale at Wait scale 0.1 (RZT_Finale.c): a commit refused below the
#     gate, the slow finish, two routes racing and a pact, a stand lost and remade, a scripted
#     take and overtime, the release's terms ("RZT finale check: ...", "RZT finale done: ...");
#   - then the epilogue week at Epilogue > Test scale 0.02 (RZT_Epilogue.c): the commit and the
#     hall's freeze, a dose, loose ends live and as data, a name brought home, the Last Entry, a
#     grey choice, a town taken back, processing, the walk-out and the wipe's lines, every closing
#     item reaching its end ("RZT epilogue check: ...", "RZT epilogue done: ...").
# Every notice sent to the fake player is logged as "RZT notify [title] text".
# Nothing here is part of the mod: the files are copied into a throwaway addon folder, and the
# patches (patch.py) only touch that copy. The world's prefab GUIDs are Arma Reforger 1.9's.
# Windows (Git Bash): set SERVER_DIR to the Steam server folder; needs Python 3 (docs/PC_SETUP.md).
set -u
MODE=full
if [ "${1:-}" = "sites" ]; then MODE=sites; shift; fi
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
HERE="$ROOT/tools/runtime_test"
WORK="${WORK_DIR:-$HOME/rz_check}"
SERVER="${SERVER_DIR:-$WORK/server}"
ADDON="$WORK/runtime_addons/ReforgerZ"
PROFILE="$WORK/runtime_profile"
GUID="$(sed -n 's/^ *GUID "\([0-9A-F]*\)".*/\1/p' "$ROOT/addon.gproj" | head -1)"
BIN="./ArmaReforgerServer"
[ -f "$SERVER/ArmaReforgerServer.exe" ] && BIN="./ArmaReforgerServer.exe"
[ -f "$SERVER/$BIN" ] || { echo "No server in $SERVER: run tools/compile_check.sh first (on Windows, install it from Steam and set SERVER_DIR)"; exit 2; }
# Python 3: python3 on Linux; on Windows usually python or py (the Store's python3 shortcut fails the check).
PY="${PYTHON:-}"
if [ -z "$PY" ]; then
	for p in python3 python py; do
		"$p" -c 'import sys; sys.exit(sys.version_info[0] != 3)' > /dev/null 2>&1 && { PY="$p"; break; }
	done
fi
[ -n "$PY" ] || { echo "No Python 3 found (set PYTHON=...)"; exit 2; }

[ "$MODE" = sites ] && ADDON="$WORK/sites_addons/ReforgerZ"
rm -rf "$ADDON" "$PROFILE/profile/ReforgerZ" && mkdir -p "$ADDON"
cp "$ROOT/addon.gproj" "$ADDON/"
sed -i 's/ *"69BD721866937F85"//' "$ADDON/addon.gproj"	# Special Zombies' "Scripts/" hides ours on Linux (see compile_check.sh)
. "$ROOT/tools/strip_missing_deps.sh"	# on Linux, other dependencies missing from EXTRA_ADDONS_DIR too
rz_strip_missing_deps "$ADDON/addon.gproj" "$SERVER" "${EXTRA_ADDONS_DIR:-}" || exit 2
cp -r "$ROOT/scripts" "$ROOT/Configs" "$ADDON/"
cp -r "$HERE/sites/." "$ADDON/"
winpath() { if command -v cygpath > /dev/null 2>&1; then cygpath -w "$1"; else printf '%s' "$1"; fi; }
DIRS="$(winpath "$(dirname "$ADDON")")"
[ -n "${EXTRA_ADDONS_DIR:-}" ] && DIRS="$DIRS,$(winpath "$EXTRA_ADDONS_DIR")"

if [ "$MODE" = sites ]; then
	"$PY" "$HERE/sites.py" prepare "$ADDON" Worlds/RZSites_Arland/RZSites_Arland.ent Worlds/RZSites_Everon/RZSites_Everon.ent Worlds/RZSites_Cain/RZSites_Cain.ent || exit 2
	# Its own network namespace where possible (Linux, root): the server always listens on 2001
	WRAP=""
	if command -v unshare > /dev/null 2>&1 && unshare -n true > /dev/null 2>&1; then
		WRAP="unshare -n"
	fi
	FAIL=0
	for MAP in ${*:-arland}; do
		case "$MAP" in
			arland) NAME=RZSites_Arland;;
			everon) NAME=RZSites_Everon;;
			cain) NAME=RZSites_Cain;;
			*) echo "Unknown map $MAP (arland, everon or cain)"; exit 2;;
		esac
		SPROFILE="$WORK/sites_profile_$MAP"
		rm -rf "$SPROFILE"
		( cd "$SERVER" && $WRAP timeout "${TIMEOUT:-3000}" "$BIN" -addonsDir "$DIRS" -addons "$GUID" -profile "$SPROFILE" \
			-server "{7A1C2E3F4B5D6E90}Missions/$NAME.conf" -logStats 0 -maxFPS 30 -noBackend > /dev/null 2>&1 )
		LOG="$(ls -td "$SPROFILE"/logs/logs_* | head -1)/console.log"
		echo "== $MAP log: $LOG"
		EXC="$(grep -c 'Virtual Machine Exception' "$LOG")"
		echo "Script exceptions: $EXC"
		[ "$EXC" != "0" ] && { FAIL=1; grep -A6 'Virtual Machine Exception' "$LOG" | grep -E 'Reason|Function|scripts/' | sort | uniq -c | head -10; }
		"$PY" "$HERE/sites.py" summary "$LOG"
		grep -q "RZS done" "$LOG" || { echo "The audit didn't finish (see the log)"; FAIL=1; }
		UNDER="$(sed -n 's/.*RZS done: \([0-9]*\) strongholds under.*/\1/p' "$LOG" | head -1)"
		[ -n "$UNDER" ] && [ "$UNDER" != "0" ] && { echo "FAILED: $UNDER strongholds under the threshold"; FAIL=1; }
		N="$("$PY" "$HERE/sites.py" footprints "$LOG" "$WORK/footprints_$MAP.json")"
		echo "  $N prop boxes measured: $WORK/footprints_$MAP.json"
		if [ "${FOOTPRINTS:-0}" = "1" ] && [ "$N" != "0" ]; then
			cp "$WORK/footprints_$MAP.json" "$ROOT/tools/camp_kit/footprints.json" && echo "  written to tools/camp_kit/footprints.json (now run tools/camp_kit/build.py)"
		fi
		echo "  draw it: $PY tools/camp_kit/render.py --placed $LOG"
	done
	exit $FAIL
fi

cp -r "$HERE/files/." "$ADDON/"
"$PY" "$HERE/patch.py" "$ADDON/scripts/Game/ReforgerZ" || exit 2
"$PY" "$HERE/sites.py" prepare "$ADDON" Worlds/RZTest/RZTest.ent || exit 2

cd "$SERVER"
# Workshop dependencies come from EXTRA_ADDONS_DIR (comma-separated list, Windows form on Windows).
timeout "${1:-5400}" "$BIN" -addonsDir "$DIRS" -addons "$GUID" -profile "$PROFILE" \
	-server "{7A1C2E3F4B5D6E71}Missions/RZTest.conf" -bindPort 2001 -logStats 0 -maxFPS 30 -noBackend > /dev/null 2>&1
LOG="$(ls -td "$PROFILE"/logs/logs_* | head -1)/console.log"

echo "Log: $LOG"
[ -n "$RZ_STRIP_NOTE" ] && echo "Ran${RZ_STRIP_NOTE#;}"
echo "Script exceptions: $(grep -c 'Virtual Machine Exception' "$LOG")"
grep -A6 'Virtual Machine Exception' "$LOG" | grep -E 'Reason|Function|scripts/' | sort | uniq -c | head -20
grep -E 'RZT all mission types tried' "$LOG" | sed 's/.*RZT //'
grep -E 'RZT mission .* why: ' "$LOG" | sed -E 's/.*RZT mission ([A-Za-z_]+) why: /  did not start: \1: /'
grep -E 'RZT (fake player already dead|killing|fake player died|perimeter: )' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT (questlines:|all questlines tried|quest .* (ENDED|STUCK|fizzled|did not start|why: ))' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT (majors done|finale check: .*FAILED|finale done|finale: )' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT (epilogue check: .*FAILED|epilogue done|epilogue: )' "$LOG" | sed 's/.*RZT /  /'
grep -E '\[ReforgerZ\] Sites audit: [0-9]+ sites,' "$LOG" | sed 's/.*\[ReforgerZ\] /  /'
grep -q 'Virtual Machine Exception' "$LOG" && exit 1
exit 0
