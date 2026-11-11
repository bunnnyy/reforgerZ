# PC setup

A runbook for **Claude Code on the author's Windows PC**. Claude reads it and does the steps; the
author only clicks where Claude asks. It covers getting the PC ready (Steam apps, the repo,
Workshop mods), opening the mod in Workbench, compiling and testing locally, and play-testing.
**What** to build in Workbench (prefabs, factions, settings) is in
[WORKBENCH_SETUP.md](WORKBENCH_SETUP.md); this doc is only about the PC and how to drive it.

To start, the author opens Claude Code in the repo folder (or anywhere, for a fresh PC) and says
*"Set up my PC for ReforgerZ, follow docs/PC_SETUP.md"*, or runs `/pc-setup`.

## Rules for Claude on this PC

| Ask the author first | Never |
|---|---|
| Installing anything (Steam apps, Git, Python, Workshop mods) | Type, read or store passwords, Steam Guard codes or payment details. The author logs in themselves |
| Launching the game or Workbench (they take over the screen) | Upload to the Workshop, or publish anything. That's the author's call, every time |
| `git push`, and any commit outside the working branch | Delete or edit files in the Steam game folders. If the game is broken, Steam's **Verify integrity of game files** fixes it |
| Changing Steam, game or Windows settings | Turn off antivirus, BattlEye or Windows security features |
| Overwriting a file you didn't write (look at it and back it up first) | Touch other games, or files outside the folders below |

Folders Claude may work in:

| Folder | What's there |
|---|---|
| The repo (e.g. `C:\Mods\reforgerZ`) | The mod |
| `%USERPROFILE%\rz_check` | Compile-check and runtime-test scratch (safe to delete) |
| `Documents\My Games\ArmaReforgerWorkbench\` | Workbench's profile, logs and downloaded addons |
| `Documents\My Games\ArmaReforger\` | The game's profile, logs and downloaded Workshop mods |

Report honestly at the end of every session: which steps are done, which the author must still do,
and what failed (with the log line).

## How Claude drives the PC

| Task | How |
|---|---|
| Files, git, finding paths, reading logs, launching programs | The shell. Claude Code on Windows runs Git Bash; for Windows commands use `powershell -NoProfile -Command "..."`. Git Bash rewrites arguments that start with `/` into paths, so `reg query ... /v` breaks: use PowerShell for those |
| Installing a Steam app | `powershell -NoProfile -Command "Start-Process 'steam://install/<app id>'"` opens Steam's install dialog; the author clicks **Install** (`start` doesn't exist in Git Bash) |
| Workbench's windows (projects, World Editor, plugins, prefab fields) | Computer use, if this session has it (the Claude desktop app's computer-use tools). Without it, give the author one step at a time and wait for "done" |
| Checking a Workbench result | Read Workbench's log instead of asking the author to copy text (see [Logs](#logs)) |

## 1. Check what's there

Read-only. Find everything, then tell the author what's installed and what's missing.

1. **Steam:** `powershell -NoProfile -Command "(Get-ItemProperty 'HKCU:\Software\Valve\Steam').SteamPath"`.
   Every Steam library is a `"path"` line in `<SteamPath>/steamapps/libraryfolders.vdf`.
2. **The Steam apps:** an app is installed when `appmanifest_<id>.acf` is in a library's `steamapps`
   folder with `"StateFlags" "4"`. Its folder is `steamapps/common/<install folder>`.

   | App | ID | Install folder | Program | Needed for |
   |---|---|---|---|---|
   | Arma Reforger | 1874880 | `Arma Reforger` | `ArmaReforgerSteam.exe` (Steam starts it through `ArmaReforger_BE.exe`) | Playing |
   | Arma Reforger Tools | 1874910 | `Arma Reforger Tools` | `Workbench\ArmaReforgerWorkbenchSteamDiag.exe` | Workbench |
   | Arma Reforger Server | 1874900 | `Arma Reforger Server` | `ArmaReforgerServer.exe` | Compile check, runtime test, local multiplayer |

   Tools and Server are free, under **Library → Tools** in Steam.
3. **Other programs:** `git --version`; Python 3 (`python --version` or `py -3 --version`), needed by
   `tools/prefab_manifest.py` and the runtime test.
4. **Workshop mods** the game has downloaded: folders named like `BaconZombies_622120A5448725E3`
   under `Documents\My Games\ArmaReforger\addons`. The mods ReforgerZ needs are in
   [MOD_STACK.md](MOD_STACK.md).
5. **Disk space** on the drive Steam uses (`df -h`). The server alone is about 12 GB.

## 2. Install what's missing

Ask first, then:

| Missing | How |
|---|---|
| Tools or Server | Open `steam://install/1874910` or `steam://install/1874900` (as in [How Claude drives the PC](#how-claude-drives-the-pc)); wait for its `StateFlags` to reach 4 |
| Git | `winget install --id Git.Git -e` (Claude Code needs it anyway) |
| Python 3 | `winget install --id Python.Python.3.12 -e`. Don't rely on the `python3` that Windows ships: it's a shortcut to the Microsoft Store |
| Workshop mods | The author subscribes in the game (**Workshop** tab, search, **Subscribe**). At minimum Bacon Zombies; see [MOD_STACK.md](MOD_STACK.md) |

## 3. The repo

1. Clone to a short path with no spaces, for example `C:\Mods\reforgerZ`:
   `git clone https://github.com/bunnnyy/reforgerZ.git`. If GitHub asks for a login, the author signs in.
2. Check out the working branch: `git branch -r` lists them; work happens on `claude/...` branches
   until they're merged.
3. `git pull` at the start of every session.
4. Line endings: `.gitattributes` keeps `.sh` and `.py` files on LF, or Git Bash can't run them.
   If a script fails with `$'\r': command not found`, run `git add --renormalize .` and check out again.
5. What to commit after Workbench work: new resources with their `.meta` files (they hold the GUIDs),
   the changed `addon.gproj` and world files. What not to: anything `.gitignore` lists (`*.bak`,
   `resourceDatabase.rdb`, logs, profiles). Check `git status` before every commit.

## 4. Workbench

1. Steam must be running. Start `Arma Reforger Tools\Workbench\ArmaReforgerWorkbenchSteamDiag.exe`.
2. In the launcher: **Add existing project** → the repo's `addon.gproj`, then open it.
3. Continue with [WORKBENCH_SETUP.md, section 1.1](WORKBENCH_SETUP.md#11-before-you-start-project-and-dependencies)
   step 3 (dependencies) and work through that doc in order. Claude drives Workbench with computer use,
   or guides the author through it one step at a time.
4. After each compile (**F7**) or plugin run, read Workbench's log for errors and the plugin's
   `[ReforgerZ]` lines.

## 5. Compile check and runtime test

The same scripts as on Linux, run from Git Bash, with `SERVER_DIR` set to the Steam server folder
(on Windows they don't install the server themselves):

```bash
SERVER_DIR="/c/Program Files (x86)/Steam/steamapps/common/Arma Reforger Server" tools/compile_check.sh
```

```bash
SERVER_DIR="/c/Program Files (x86)/Steam/steamapps/common/Arma Reforger Server" tools/runtime_test/run.sh
```

Use the library path found in step 1 if Steam isn't on `C:`. What each one shows and can't show is
in [WORKBENCH_SETUP.md, section 6](WORKBENCH_SETUP.md#compile-check-without-workbench). The compile
check doesn't compile the Workbench plugin: Workbench's **F7** does. The runtime test takes 60 to 90
minutes and needs Python.

## 6. Play-testing

| Test | How |
|---|---|
| Quick single-player | Open the world in Workbench's World Editor and press **Play** |
| Local multiplayer | Workbench's peer-client tool (Workbench calls it the Peer Tool) |
| Full server | Run `ArmaReforgerServer.exe` with `-addonsDir <folder holding the mod> -addons A88C8A4C8C74454A -server <scenario .conf> -bindPort 2001`, then join `127.0.0.1:2001` from the game |

During and after a session, read the logs against
[WORKBENCH_SETUP.md, section 7](WORKBENCH_SETUP.md#7-log-lines-to-check), tick the
[play-test checklist](WORKBENCH_SETUP.md#8-play-test-checklist) with the author, and mark the
[VERIFY items](WORKBENCH_SETUP.md#6-the-verify-list) that the session settled.

### Logs

| Program | Logs |
|---|---|
| Workbench | `Documents\My Games\ArmaReforgerWorkbench\logs\` (newest folder, `console.log`) |
| Game | `Documents\My Games\ArmaReforger\logs\` |
| Server | `<profile>\logs\` (`-profile` folder; the scripts use `%USERPROFILE%\rz_check`) |

## 7. After a session

1. Write down what you learned where it belongs: VERIFY results in
   [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#6-the-verify-list), bugs fixed in code, play-test notes in
   [ROADMAP.md](ROADMAP.md), and anything about this PC or these steps in this doc.
2. Commit on the working branch; ask before pushing.
3. When a row in [Not yet confirmed](#not-yet-confirmed) is settled, update the step it belongs to
   and delete the row.

## Not yet confirmed

Written before the first run on the author's PC. Confirm each on the first session.

| Step | What to check |
|---|---|
| 4 | Workbench lists the game's downloaded Workshop mods under **Project Settings → Dependencies**. If not, find out where Workbench wants them (its own `addons` folder, or an `-addonsDir` start parameter) and write it here |
| 4 | The exact launcher button names in this Tools version |
| 5 | Git Bash converts the `/c/...` paths for the Windows server, and `timeout` stops it |
| 6 | The World Editor's **Play** and the Peer Tool names in this version, and whether the game client needs `-addonsDir`/`-addons` to join a server running the unpublished mod |
| Logs | The Workbench and game log folders above |
