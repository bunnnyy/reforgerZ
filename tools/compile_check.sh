#!/usr/bin/env bash
# ReforgerZ compile check on the dedicated server (no Workbench; on Linux no Steam account either).
#
#   tools/compile_check.sh            # install what's missing, compile, print ReforgerZ errors/warnings
#   SERVER_DIR=... tools/compile_check.sh
#   EXTRA_ADDONS_DIR=... tools/compile_check.sh   # a folder holding the Workshop dependencies in addon.gproj
#
# Installs SteamCMD and the Arma Reforger dedicated server (app 1874900, anonymous login, ~12 GB)
# on first run, copies this repo into a test addon folder and starts the server just long enough
# to compile every script module. Exit code 0 = the Game module compiled, 1 = it didn't.
# Linux: needs 32-bit libs for SteamCMD (lib32gcc-s1, lib32stdc++6).
# Windows (Git Bash): install Arma Reforger Server from Steam (Library > Tools) and set SERVER_DIR
# to its folder; the script doesn't install it there (docs/PC_SETUP.md).
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WORK="${WORK_DIR:-$HOME/rz_check}"
SERVER="${SERVER_DIR:-$WORK/server}"
ADDONS="$WORK/addons"
LOG="$WORK/compile.log"
GUID="$(sed -n 's/^ *GUID "\([0-9A-F]*\)".*/\1/p' "$ROOT/addon.gproj" | head -1)"

mkdir -p "$WORK"
# The conversation packs are generated from dialogue/**/*.rzd: the dialogue lint runs first and the
# packs must be up to date (docs/DIALOGUE.md, "Tools").
PY=""
if command -v python3 > /dev/null 2>&1; then PY="python3"; elif command -v py > /dev/null 2>&1; then PY="py -3"; fi
if [ -n "$PY" ]; then
	$PY "$ROOT/tools/dialogue_pack.py" --check || { echo "FAILED: the dialogue lint (python3 tools/dialogue_pack.py)"; exit 1; }
else
	echo "Warning: no Python, so the dialogue lint didn't run (python3 tools/dialogue_pack.py --check)."
fi
if [ ! -f "$SERVER/ArmaReforgerServer" ] && [ ! -f "$SERVER/ArmaReforgerServer.exe" ]; then
	if [ "$(uname -s)" != "Linux" ]; then
		echo "No server in $SERVER. Install Arma Reforger Server from Steam (Library > Tools) and set SERVER_DIR to its folder."
		exit 2
	fi
	if [ ! -x "$WORK/steamcmd/steamcmd.sh" ]; then
		mkdir -p "$WORK/steamcmd"
		curl -sSL https://steamcdn-a.akamaihd.net/client/installer/steamcmd_linux.tar.gz | tar xz -C "$WORK/steamcmd"
	fi
	# The first run often fails with "Missing configuration"; a second run works.
	for attempt in 1 2 3; do
		"$WORK/steamcmd/steamcmd.sh" +force_install_dir "$SERVER" +login anonymous +app_update 1874900 validate +quit > "$WORK/steamcmd.log" 2>&1
		grep -q "Success! App '1874900'" "$WORK/steamcmd.log" && break
	done
	[ -x "$SERVER/ArmaReforgerServer" ] || { echo "Server install failed, see $WORK/steamcmd.log"; exit 2; }
fi

# The test addon: same layout as the repo (the engine wants lowercase 'scripts' on Linux).
rm -rf "$ADDONS/ReforgerZ" && mkdir -p "$ADDONS/ReforgerZ"
cp "$ROOT/addon.gproj" "$ADDONS/ReforgerZ/"
for d in scripts Configs UI Prefabs Worlds Missions Layouts; do
	[ -d "$ROOT/$d" ] && cp -r "$ROOT/$d" "$ADDONS/ReforgerZ/"
done

BIN="./ArmaReforgerServer"
[ -f "$SERVER/ArmaReforgerServer.exe" ] && BIN="./ArmaReforgerServer.exe"
cd "$SERVER"
# -listScenarios compiles everything, prints the scenarios and exits; the timeout is a safety net.
# Dependencies (other than the base game) come from EXTRA_ADDONS_DIR; -addonsDir takes a comma-separated
# list. On Windows the list is passed in Windows form (Git Bash only converts a single leading path).
winpath() { if command -v cygpath > /dev/null 2>&1; then cygpath -w "$1"; else printf '%s' "$1"; fi; }
DIRS="$(winpath "$ADDONS")"
[ -n "${EXTRA_ADDONS_DIR:-}" ] && DIRS="$DIRS,$(winpath "$EXTRA_ADDONS_DIR")"
rm -rf "$WORK/profile/logs"
timeout "${TIMEOUT:-180}" "$BIN" -addonsDir "$DIRS" -addons "$GUID" \
	-profile "$WORK/profile" -listScenarios -logStats 0 > "$LOG" 2>&1
# If the server didn't log to stdout (unconfirmed on Windows), use its console.log.
if ! grep -q "Module: Game;" "$LOG"; then
	C="$(ls -td "$WORK/profile"/logs/logs_* 2>/dev/null | head -1)/console.log"
	[ -f "$C" ] && cp "$C" "$LOG"
fi

grep -E "\((E|W)\).*scripts/Game/ReforgerZ/|Can't compile" "$LOG"
if grep -E "dependency '[0-9A-F]*' can't be added" "$LOG"; then
	echo "Warning: a Workshop dependency in addon.gproj is missing; set EXTRA_ADDONS_DIR to a folder holding it."
fi
if grep -q "Module: Game;" "$LOG" && ! grep -q "Can't compile" "$LOG"; then
	echo "OK: the Game module compiled ($(grep -cE '\(W\).*scripts/Game/ReforgerZ/' "$LOG") ReforgerZ warnings). Log: $LOG"
	exit 0
fi
echo "FAILED: see $LOG"
exit 1
