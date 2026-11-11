#!/usr/bin/env bash
# ReforgerZ runtime test on the Linux dedicated server (no Workbench, no game client).
#
#   SERVER_DIR=... tools/runtime_test/run.sh [seconds]     # default 5400 (all missions, all questlines, then the route)
#
# Needs the server installed (tools/compile_check.sh installs it). Boots a sub-scene of Arland
# with RZ_GameModeComponent on GameMode_Plain, vanilla stand-ins for the minimum prefab slots
# (an FIA group as "zombies", a crate as the reward crate...), and a test-only harness that:
#   - adds a fake player (id 901, a spawned civilian) to the players every system sees;
#   - starts every registered mission type one at a time, moves the player there, then ends it
#     (alternately won and failed) so Setup, Update, rewards and cleanup all run;
#   - then begins every registered questline in turn, fast-forwards its waits, goes to each stage
#     mission and ends it (mostly won), and reports which reached an ending, fizzled or got stuck;
#   - walks the player through every settlement, the safehouses, the lab, then into the
#     Perimeter's warning and kill zones; kills them if the Perimeter didn't; respawns them.
# Every notice sent to the fake player is logged as "RZT notify [title] text".
# Nothing here is part of the mod: the files are copied into a throwaway addon folder, and the
# patches (patch.py) only touch that copy. The world's prefab GUIDs are Arma Reforger 1.9's.
set -u
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
HERE="$ROOT/tools/runtime_test"
WORK="${WORK_DIR:-$HOME/rz_check}"
SERVER="${SERVER_DIR:-$WORK/server}"
ADDON="$WORK/runtime_addons/ReforgerZ"
PROFILE="$WORK/runtime_profile"
GUID="$(sed -n 's/^ *GUID "\([0-9A-F]*\)".*/\1/p' "$ROOT/addon.gproj" | head -1)"
[ -x "$SERVER/ArmaReforgerServer" ] || { echo "No server in $SERVER: run tools/compile_check.sh first"; exit 2; }

rm -rf "$ADDON" "$PROFILE/profile/ReforgerZ" && mkdir -p "$ADDON"
cp "$ROOT/addon.gproj" "$ADDON/"
cp -r "$ROOT/scripts" "$ROOT/Configs" "$ADDON/"
cp -r "$HERE/files/." "$ADDON/"
python3 "$HERE/patch.py" "$ADDON/scripts/Game/ReforgerZ" || exit 2

cd "$SERVER"
timeout "${1:-5400}" ./ArmaReforgerServer -addonsDir "$(dirname "$ADDON")" -addons "$GUID" -profile "$PROFILE" \
	-server "{7A1C2E3F4B5D6E71}Missions/RZTest.conf" -bindPort 2001 -logStats 0 -maxFPS 30 -noBackend > /dev/null 2>&1
LOG="$(ls -td "$PROFILE"/logs/logs_* | head -1)/console.log"

echo "Log: $LOG"
echo "Script exceptions: $(grep -c 'Virtual Machine Exception' "$LOG")"
grep -A6 'Virtual Machine Exception' "$LOG" | grep -E 'Reason|Function|scripts/' | sort | uniq -c | head -20
grep -E 'RZT all mission types tried' "$LOG" | sed 's/.*RZT //'
grep -E 'RZT mission .* started=0' "$LOG" | sed -E 's/.*RZT mission ([A-Za-z_]+).*/  did not start: \1/'
grep -E 'RZT (fake player already dead|killing|fake player died)' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT (questlines:|all questlines tried|quest .* (ENDED|STUCK|fizzled|did not start))' "$LOG" | sed 's/.*RZT /  /'
grep -q 'Virtual Machine Exception' "$LOG" && exit 1
exit 0
