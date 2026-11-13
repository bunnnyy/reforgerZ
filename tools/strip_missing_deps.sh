# Sourced by tools/compile_check.sh, tools/runtime_test/run.sh and tools/zombie_lab/run.sh (not run).
# Two functions: rz_strip_capital_scripts (further down), and
#
#   rz_strip_missing_deps <throwaway addon.gproj> <server folder> <EXTRA_ADDONS_DIR>
#
# Linux only (on Windows it does nothing): drops from the throwaway copy of addon.gproj every
# dependency that is neither one of the server's own addons nor in EXTRA_ADDONS_DIR (a folder, or a
# comma-separated list of folders, holding the Workshop mods), and prints a line for each. Without it
# the server refuses the whole addon ("dependency '...' can't be added") and compiles none of it.
# Only mods no ReforgerZ script references can go: Bacon Zombies (its classes are modded by the
# behaviour layer) must be there, or this returns 1. Sets RZ_STRIP_NOTE to "" or to a note for the
# result line ("; without 2 dependencies: ...").
RZ_REQUIRED_DEPS="622120A5448725E3"	# Bacon Zombies (docs/MOD_STACK.md)

#   rz_strip_capital_scripts <throwaway addon.gproj>
#
# Drops the dependencies that keep their scripts in "Scripts/" (capital S). On the Linux server, with
# one of them loaded, the Game module never finds our lowercase "scripts/" and compiles nothing of
# ReforgerZ, without an error (docs/MOD_STACK.md). No ReforgerZ script names anything of theirs, so
# the checks run without them: Special Zombies and WildlifeExpansion.
RZ_CAPITAL_SCRIPTS_DEPS="69BD721866937F85 68B3771125D0A209"

rz_strip_capital_scripts() {
	local gproj="$1" g
	for g in $RZ_CAPITAL_SCRIPTS_DEPS; do
		sed -i "s/ *\"$g\"//" "$gproj"
	done
}

rz_strip_missing_deps() {
	local gproj="$1" server="$2" extra="$3"
	local have="" dir f g stripped=0 list=""
	RZ_STRIP_NOTE=""
	[ "$(uname -s)" = "Linux" ] || return 0
	for f in "$server"/addons/*/*.gproj; do
		[ -f "$f" ] && have="$have $(sed -n 's/^ *GUID "\([0-9A-F]*\)".*/\1/p' "$f" | head -1)"
	done
	if [ -n "$extra" ]; then
		local IFS_OLD="$IFS"
		IFS=','
		for dir in $extra; do
			IFS="$IFS_OLD"
			for f in "$dir"/*/addon.gproj; do
				[ -f "$f" ] && have="$have $(sed -n 's/^ *GUID "\([0-9A-F]*\)".*/\1/p' "$f" | head -1)"
			done
			IFS=','
		done
		IFS="$IFS_OLD"
	fi
	for g in $(sed -n '/Dependencies *{/,/}/p' "$gproj" | grep -oE '"[0-9A-F]{16}"' | tr -d '"'); do
		case " $have " in *" $g "*) continue ;; esac
		case " $RZ_REQUIRED_DEPS " in
			*" $g "*)
				echo "FAILED: dependency $g isn't in EXTRA_ADDONS_DIR, and ReforgerZ's scripts use it (docs/MOD_STACK.md)."
				return 1 ;;
		esac
		sed -i "s/ *\"$g\"//" "$gproj"
		echo "Left out: dependency $g isn't in EXTRA_ADDONS_DIR, so this run goes without it (docs/MOD_STACK.md)."
		stripped=$((stripped + 1))
		list="$list $g"
	done
	[ "$stripped" -gt 0 ] && RZ_STRIP_NOTE="; without $stripped dependencies:$list"
	return 0
}
