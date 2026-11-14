#!/usr/bin/env bash
# ReforgerZ runtime test on the dedicated server (no Workbench, no game client).
#
#   SERVER_DIR=... tools/runtime_test/run.sh [seconds]     # default 5400 (all missions, all questlines, then the route)
#   SERVER_DIR=... tools/runtime_test/run.sh sites [arland] [everon] [cain]   # the site audit only (default arland)
#   SERVER_DIR=... tools/runtime_test/run.sh tasks [arland] [everon] [cain]   # the task place audit only (default arland)
#   SERVER_DIR=... EXTRA_ADDONS_DIR=... tools/runtime_test/run.sh layer   # the behaviour layer test only (needs Bacon Zombies)
#   EXTRA_ADDONS_DIR=...                                   # a folder holding the Workshop dependencies in addon.gproj
#
# The behaviour layer test (docs/WORKBENCH_SETUP.md, "Behaviour layer test without Workbench"): "layer"
# boots the site audit's Arland world with the mod's own game mode settings, two tribe slots, Bacon's
# faction, Arland's navmesh and a perception manager (layer.py prepare), puts the fake player near the
# tribe, starts a hunt with the function /tribes hunt calls and walks the layer through its modes at
# noon and at night (layer/scripts/Game/RZLayer/RZB_Layer.c): the pack put out and steered, releases
# on a strike, a shot and contact, the fight, take-back and recover, the walk home, a shot heard, the
# bodies put away. layer.py summary prints the results table. 17 to 30 minutes.
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
# The task place audit (docs/PLACES.md, "The audit"): "tasks" boots the same worlds with the mod's own
# game mode settings (RZ_GameMode.et's, places.py prepare; the mod's prefabs come along), waits for the
# land grid, asks the game's own pickers for task places across the map (the openings' place roll and
# every step it leads to, every errand way from a few givers each, the spawn points), checks each
# against its need and its things (World/RZ_PlaceNeeds.c), prints "Places:" per kind of task and the
# summary, and closes. It fails if a kind scores under the threshold (RZ_PlaceAudit.s_iThreshold).
# Draw it: python3 tools/runtime_test/places.py draw <console.log>.
#
# Needs the server installed (tools/compile_check.sh installs it). Boots a sub-scene of Arland
# with RZ_GameModeComponent on GameMode_Plain, vanilla stand-ins for the minimum prefab slots
# (an FIA group as "zombies", a crate as the reward crate...), and a test-only harness that:
#   - adds a fake player (id 901, a spawned civilian) to the players every system sees;
#   - runs every conversation's nodes once for that player, and a lie, a promise and a callback
#     ("RZT Talk test: ..."), and walks it through reputation's cells, the credit rule and the
#     forgiveness budget ("RZT Reputation test: ..."); from minute 2 it cycles Voice > Force mood
#     through the five moods, 5 minutes each, and prints each one's picks ("RZT voice <mood> ...");
#   - 12 s in, before the missions, checks the openings' play-test faults at the first town
#     (RZT_Openings.c): the Fever's son lies there on a care work site with the tracker on him, 24 s
#     by the giver doesn't finish it and the hold with a dressing does; a hand-over (the Cut) is done
#     there and then and only with the thing; a real outside door is found near the town and a
#     door-like prop is refused in the open ("RZT openings OK|FAIL ...", "RZT openings: N checks");
#   The map's fog (RZT_Fog.c): where the tester stands is seen, walking 600 m reveals new ground, a paper map in his pack reveals its far town, the fog survives the save's JSON round trip, the payload is small ("RZT fog OK|FAIL ...", "RZT fog: N checks");
#   The door and the favour (RZT_Favour.c): for every background, the door asks who you were and a habit is picked (each habit's effect checked), the favour's fetch is handed at a place, the hand-over waits for its reply (standing by him never counts), and the background's way settles it without a hand-over ("RZT favour OK|FAIL ...", "RZT favour: N checks");
#   The line (RZT_Line.c): a road above the line near a town (150-250 m outside the warning zone, 600 m to 2 km off), the scene played with the tester at it: the loudspeakers, a runner who breaks from cover (or is laid down), shot in the warning zone not the kill zone, dead there with the case by him, the line said, the tester never shot and never in the zones ("RZT line OK|FAIL ...", "RZT line: N checks");
#   A fresh spawn (RZT_Fresh.c): as a veteran, the opening has its contact within 15 m and the door's favour first, the last life's questline part and jobs aren't the new one's, and the Quests tab lists nothing taken ("RZT fresh OK|FAIL ...", "RZT fresh: N checks");
#   - starts every registered mission type one at a time, moves the player where the quest tracker
#     points, then ends it (alternately won and failed) so Setup, Update, rewards and cleanup all run;
#   - checks every objective's place on the way (RZT_ObjPlace.c): for each mission and questline
#     stage, the place the tracker reports for the current objective must be within the step's
#     arrival radius of where the step is really done (a delivery's thing, then, carried, its drop; a
#     visit's points, walked one by one where the tracker sends him; a chain's step; the job's own
#     place or props); "RZT objplace FAIL ..." lines, summed as "RZT objective places (...)";
#   - forces what missions need (a spread of town infection before each, a few goes at a Setup
#     that misses, stand-ins for the optional prefabs, trucks for the convoys and Mobile Lab;
#     RZT_Prep.c);
#   - then forces each registered questline's start conditions (RZT_Prep.c), begins it, fast-forwards
#     its waits, goes to each stage mission and ends it as won (with the outcome that moves the
#     story on), and reports which reached an ending, fizzled or got stuck;
#   - 10 s in, runs the power station's looks once (RZT_Power.c): the dressing up beside the control,
#     the generator's sound up while it turns and down when it stops, everything down with nobody
#     near, and the street lamps' list dark and powered ("RZT Power test: ...");
#   - 4 minutes in, carries three crates into the nearest building (RZT_Carry.c): the pile, the
#     put-down indoors under a roof, the stack ("RZT carry: ...");
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
if [ "${1:-}" = "tasks" ]; then MODE=tasks; shift; fi
if [ "${1:-}" = "layer" ]; then MODE=layer; shift; fi
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
[ "$MODE" = tasks ] && ADDON="$WORK/tasks_addons/ReforgerZ"
[ "$MODE" = layer ] && ADDON="$WORK/layer_addons/ReforgerZ"
rm -rf "$ADDON" "$PROFILE/profile/ReforgerZ" && mkdir -p "$ADDON"
cp "$ROOT/addon.gproj" "$ADDON/"
. "$ROOT/tools/strip_missing_deps.sh"
rz_strip_capital_scripts "$ADDON/addon.gproj"	# Special Zombies' and WildlifeExpansion's "Scripts/" hide ours on Linux (see compile_check.sh)
# on Linux, other dependencies missing from EXTRA_ADDONS_DIR too
rz_strip_missing_deps "$ADDON/addon.gproj" "$SERVER" "${EXTRA_ADDONS_DIR:-}" || exit 2
cp -r "$ROOT/scripts" "$ROOT/Configs" "$ADDON/"
if [ "$MODE" = tasks ]; then
	cp -r "$ROOT/Prefabs" "$HERE/sites/Worlds" "$HERE/sites/Missions" "$ADDON/"
	cp -r "$HERE/tasks/." "$ADDON/"
elif [ "$MODE" = layer ]; then
	cp -r "$ROOT/Prefabs" "$HERE/sites/Worlds" "$HERE/sites/Missions" "$ADDON/"
	cp -r "$HERE/layer/." "$ADDON/"
	mkdir -p "$ADDON/scripts/Game/RZTest"
	cp "$HERE/files/scripts/Game/RZTest/RZT_Fake.c" "$ADDON/scripts/Game/RZTest/"
else
	cp -r "$HERE/sites/." "$ADDON/"
fi
winpath() { if command -v cygpath > /dev/null 2>&1; then cygpath -w "$1"; else printf '%s' "$1"; fi; }
DIRS="$(winpath "$(dirname "$ADDON")")"
[ -n "${EXTRA_ADDONS_DIR:-}" ] && DIRS="$DIRS,$(winpath "$EXTRA_ADDONS_DIR")"

if [ "$MODE" = tasks ]; then
	"$PY" "$HERE/places.py" prepare "$ADDON" Worlds/RZSites_Arland/RZSites_Arland.ent Worlds/RZSites_Everon/RZSites_Everon.ent Worlds/RZSites_Cain/RZSites_Cain.ent || exit 2
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
		TPROFILE="$WORK/tasks_profile_$MAP"
		rm -rf "$TPROFILE"
		( cd "$SERVER" && $WRAP timeout "${TIMEOUT:-3600}" "$BIN" -addonsDir "$DIRS" -addons "$GUID" -profile "$TPROFILE" \
			-server "{7A1C2E3F4B5D6E90}Missions/$NAME.conf" -logStats 0 -maxFPS 30 -noBackend > /dev/null 2>&1 )
		LOG="$(ls -td "$TPROFILE"/logs/logs_* | head -1)/console.log"
		echo "== $MAP log: $LOG"
		EXC="$(grep -c 'Virtual Machine Exception' "$LOG")"
		echo "Script exceptions: $EXC"
		[ "$EXC" != "0" ] && { FAIL=1; grep -A6 'Virtual Machine Exception' "$LOG" | grep -E 'Reason|Function|scripts/' | sort | uniq -c | head -10; }
		"$PY" "$HERE/places.py" summary "$LOG"
		grep -q "RZK done" "$LOG" || { echo "The audit didn't finish (see the log)"; FAIL=1; }
		UNDER="$(sed -n 's/.*RZK done: \([0-9]*\) kinds under.*/\1/p' "$LOG" | head -1)"
		[ -n "$UNDER" ] && [ "$UNDER" != "0" ] && { echo "FAILED: $UNDER kinds of task under the threshold"; FAIL=1; }
		"$PY" "$HERE/places.py" draw "$LOG" "$WORK/task_places_$MAP.html"
	done
	exit $FAIL
fi

if [ "$MODE" = layer ]; then
	grep -q 622120A5448725E3 "$ADDON/addon.gproj" || { echo "The layer test needs Bacon Zombies (622120A5448725E3) in EXTRA_ADDONS_DIR: without its bodies there is no layer to test"; exit 2; }
	"$PY" "$HERE/patch.py" "$ADDON/scripts/Game/ReforgerZ" || exit 2
	"$PY" "$HERE/places.py" prepare "$ADDON" Worlds/RZSites_Arland/RZSites_Arland.ent || exit 2
	"$PY" "$HERE/layer.py" prepare "$ADDON" Worlds/RZSites_Arland/RZSites_Arland.ent || exit 2
	WRAP=""
	if command -v unshare > /dev/null 2>&1 && unshare -n true > /dev/null 2>&1; then
		WRAP="unshare -n"
	fi
	LPROFILE="$WORK/layer_profile"
	rm -rf "$LPROFILE"
	( cd "$SERVER" && $WRAP timeout "${TIMEOUT:-4200}" "$BIN" -addonsDir "$DIRS" -addons "$GUID" -profile "$LPROFILE" \
		-server "{7A1C2E3F4B5D6E90}Missions/RZSites_Arland.conf" -logStats 0 -maxFPS 30 -noBackend > /dev/null 2>&1 )
	LOG="$(ls -td "$LPROFILE"/logs/logs_* | head -1)/console.log"
	echo "Log: $LOG"
	[ -n "$RZ_STRIP_NOTE" ] && echo "Ran${RZ_STRIP_NOTE#;}"
	FAIL=0
	EXC="$(grep -c 'Virtual Machine Exception' "$LOG")"
	echo "Script exceptions: $EXC"
	# The summary tells ReforgerZ's exceptions (a failure) from the game's own (listed, not a failure)
	[ "$EXC" != "0" ] && grep -A6 'Virtual Machine Exception' "$LOG" | grep -E 'Reason|Function|scripts/' | sort | uniq -c | head -10
	grep -q "RZB done" "$LOG" || { echo "The test didn't finish (see the log)"; FAIL=1; }
	"$PY" "$HERE/layer.py" summary "$LOG" || FAIL=1
	exit $FAIL
fi

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
# The power station's own look-only prefabs come along (the test world's Power grid slots name them),
# so the power test spawns the real ones
mkdir -p "$ADDON/Prefabs/ReforgerZ/World"
cp "$ROOT"/Prefabs/ReforgerZ/World/RZ_PowerDressing.et* "$ROOT"/Prefabs/ReforgerZ/World/RZ_PowerGeneratorSound.et* "$ADDON/Prefabs/ReforgerZ/World/"
# The work site box (Shared > Work sites > Site prefab) for the openings' check (RZT_Openings.c),
# which turns work sites on with it when the test world leaves them off
mkdir -p "$ADDON/Prefabs/ReforgerZ/Work"
cp "$ROOT"/Prefabs/ReforgerZ/Work/RZ_WorkSite.et* "$ADDON/Prefabs/ReforgerZ/Work/"
# The hand-built sites' role spots (docs/SITES.md, "Role spots"): a shipped save's spots spawn them
mkdir -p "$ADDON/Prefabs/ReforgerZ/World/HandBuilt"
cp "$ROOT"/Prefabs/ReforgerZ/World/HandBuilt/RZ_HandSpot*.et* "$ADDON/Prefabs/ReforgerZ/World/HandBuilt/"
# What shows in the arms while carrying (docs/DESIGN.md, "Carrying"): the carry check reads it
mkdir -p "$ADDON/Prefabs/ReforgerZ/Work"
cp "$ROOT"/Prefabs/ReforgerZ/Work/RZ_Carried_*.et* "$ADDON/Prefabs/ReforgerZ/Work/"
# The kitchen work looks (tools/camp_kit/work_looks.py) the kitchen check stands at a prompt
cp "$ROOT"/Prefabs/ReforgerZ/Work/RZ_Work_Stove.et* "$ROOT"/Prefabs/ReforgerZ/Work/RZ_Work_KitchenTable.et* "$ROOT"/Prefabs/ReforgerZ/Work/RZ_Work_ServeTable.et* "$ROOT"/Prefabs/ReforgerZ/Work/RZ_Work_Pot.et* "$ADDON/Prefabs/ReforgerZ/Work/"
# The runner's cold case (Guidance > Cold case), for the case check (RZT_Case.c)
mkdir -p "$ADDON/Prefabs/ReforgerZ/Guidance"
cp "$ROOT"/Prefabs/ReforgerZ/Guidance/RZ_ColdCase.et* "$ADDON/Prefabs/ReforgerZ/Guidance/"
# The crafting items and the recipes' outputs (RZT_Craft.c sets them on the test world's Crafting)
mkdir -p "$ADDON/Prefabs/ReforgerZ/Crafting" "$ADDON/Prefabs/ReforgerZ/Interactions"
cp "$ROOT"/Prefabs/ReforgerZ/Crafting/RZ_*.et* "$ADDON/Prefabs/ReforgerZ/Crafting/"
cp "$ROOT"/Prefabs/ReforgerZ/Interactions/RZ_Planks.et* "$ROOT"/Prefabs/ReforgerZ/Interactions/RZ_SparkPlugs.et* "$ROOT"/Prefabs/ReforgerZ/Interactions/RZ_Tablets.et* "$ROOT"/Prefabs/ReforgerZ/Interactions/RZ_TinRattle.et* "$ADDON/Prefabs/ReforgerZ/Interactions/"
"$PY" "$HERE/patch.py" "$ADDON/scripts/Game/ReforgerZ" || exit 2
"$PY" "$HERE/sites.py" prepare "$ADDON" Worlds/RZTest/RZTest.ent || exit 2

cd "$SERVER"
# On Linux in its own network namespace when it can be (as the audits): two runs side by side don't
# fight over the port
WRAP=""
if command -v unshare > /dev/null 2>&1 && unshare -n true > /dev/null 2>&1; then
	WRAP="unshare -n"
fi
# Workshop dependencies come from EXTRA_ADDONS_DIR (comma-separated list, Windows form on Windows).
$WRAP timeout "${1:-5400}" "$BIN" -addonsDir "$DIRS" -addons "$GUID" -profile "$PROFILE" \
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
grep -E 'RZT objective places \(' "$LOG" | tail -1 | sed 's/.*RZT /  /'
grep -E 'RZT objplace FAIL' "$LOG" | sed 's/.*RZT objplace FAIL /  objective place wrong: /' | head -40
grep -E 'RZT Power test: ' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT carry(: done| FAIL|: off|: no building)' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT openings' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT case' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT line' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT fresh' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT favour' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT kitchen' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT fog' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT patrol' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT craft' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT (majors done|finale check: .*FAILED|finale done|finale: )' "$LOG" | sed 's/.*RZT /  /'
grep -E 'RZT (epilogue check: .*FAILED|epilogue done|epilogue: )' "$LOG" | sed 's/.*RZT /  /'
grep -E '\[ReforgerZ\] Sites audit: [0-9]+ sites,' "$LOG" | sed 's/.*\[ReforgerZ\] /  /'
echo "Place checks failed (live, docs/PLACES.md): $(grep -c '\[ReforgerZ\] Place check: ' "$LOG")"
grep '\[ReforgerZ\] Place check: ' "$LOG" | sed 's/.*\[ReforgerZ\] /  /' | head -20
grep -q 'Virtual Machine Exception' "$LOG" && exit 1
grep -q 'RZT objplace FAIL' "$LOG" && { echo "FAILED: an objective points somewhere else than where its step is done"; exit 1; }
grep -q 'RZT carry FAIL' "$LOG" && { echo "FAILED: carrying for real (the pile, the put-down indoors, the stack)"; exit 1; }
grep -q 'RZT openings FAIL' "$LOG" && { echo "FAILED: an opening's step (the son, a hand-over, a door: RZT_Openings.c)"; exit 1; }
grep -q 'RZT case FAIL' "$LOG" && { echo "FAILED: the runner's case (found, carried, kept over a death, handed over by reply: RZT_Case.c)"; exit 1; }
grep -q 'RZT line FAIL' "$LOG" && { echo "FAILED: the opening's line (a road above it, the runner shot in the warning zone, the line said, the tester never shot: RZT_Line.c)"; exit 1; }
grep -q 'RZT fresh FAIL' "$LOG" && { echo "FAILED: a fresh spawn (its contact, the favour first, a clean Quests tab: RZT_Fresh.c)"; exit 1; }
grep -q 'RZT fog FAIL' "$LOG" && { echo "FAILED: the map's fog (RZT_Fog.c: walking reveals, a found map, the save, the payload)"; exit 1; }
grep -q 'RZT favour FAIL' "$LOG" && { echo "FAILED: the door and the favour (who you were, the habit, the fetch, the hand-over by reply, the background's way: RZT_Favour.c)"; exit 1; }
grep -q 'RZT kitchen FAIL' "$LOG" && { echo "FAILED: kitchen work at a kitchen (RZT_Kitchen.c: a kitchen thing at the prompt, the camp's own cook pot)"; exit 1; }
grep -q 'RZT patrol FAIL' "$LOG" && { echo "FAILED: a patrol's finds or its marker (RZT_Patrol.c: each find's thing there, a door find at a real doorway, the marker on the walker)"; exit 1; }
grep -q 'RZT craft FAIL' "$LOG" && { echo "FAILED: a new recipe or the lockpick (RZT_Craft.c)"; exit 1; }
grep -q 'RZT Power test: .*FAILED' "$LOG" && { echo "FAILED: the power station's looks (the dressing, the generator's sound or the lamps' list)"; exit 1; }
exit 0
