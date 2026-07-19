# Installation

Kobo Sudoku ships as a single zip you extract onto the e-reader over USB.
There are two ways to launch it:

| Launcher | Status |
|---|---|
| **NickelMenu** (recommended) — a **Sudoku** entry in the **More** menu | ✅ Tested on a real device (Kobo Libra Colour, firmware 4.5) |
| **KFMon** — tap a disguised book cover in your library | ⚠️ Included in the zip, but **not yet tested on real hardware** |

Both launchers start the same binary and share the same saved progress; you
can install either one, or both.

## 1. Prerequisite: NickelMenu (one-time per firmware update)

NickelMenu lets Nickel (the Kobo home software) show custom entries in the
**More** menu. It survives until a Kobo firmware update, after which you just
redo this step.

1. Download the latest `KoboRoot.tgz` from the
   [NickelMenu project](https://pgaskin.github.io/NickelMenu/) (firmware 4.x
   only — confirmed working on a Kobo Libra Colour with firmware 4.5; other
   firmware versions are best-effort).
2. Connect the Kobo over USB, copy `KoboRoot.tgz` into the hidden `.kobo`
   folder on the device, and eject safely. The Kobo reboots and installs it.

## 2. Install the game

1. Download `kobo-sudoku.zip` from the
   [latest release](https://github.com/PawelSwitalski/kobo-sudoku/releases/latest).
2. Connect the Kobo over USB (it shows up as a drive, e.g. `D:`).
3. Extract the zip onto the drive **root**, merging the `.adds` folder with
   any existing one.
4. Eject safely and let the Kobo finish importing.
5. Open **More** (bottom navigation) — a **Sudoku** entry appears alongside
   Settings and your other items. Tap it to start the game; tap
   **Menu → Exit** in the game to go back to your books.

If taps land in the wrong place on your model, see
[Touch calibration](settings.md#touch-calibration).

## Alternative launcher: KFMon cover-tap (untested)

> **Note:** this launch path is inherited from earlier development and has
> **not yet been verified on a real device**. The NickelMenu entry above is
> the tested way to start the game. If you try KFMon, please report whether
> it works.

[KFMon](https://github.com/NiLuJe/kfmon) launches programs when you tap a
specific "book" cover in your library. The required config
(`.adds/kfmon/config/sudoku.ini`) and cover image (`kfmon-sudoku.png`)
already ship in `kobo-sudoku.zip` — only the KFMon prerequisite is extra:

1. Download the latest `KoboRoot.tgz` from
   [KFMon releases](https://github.com/NiLuJe/kfmon/releases) (the
   "uninstaller-less" package is fine), copy it into `.kobo` on the device,
   and eject. The Kobo reboots and installs it.
2. After installing the game, a book called **kfmon-sudoku** appears in your
   library. Tap its cover to start the game.

## Uninstall

Delete these from the device over USB:

- `.adds/sudoku/` (the game, its settings and saved progress)
- `.adds/nm/kobo-sudoku` (the NickelMenu entry)
- `.adds/kfmon/config/sudoku.ini` and `kfmon-sudoku.png` (the KFMon launcher)

NickelMenu/KFMon themselves are separate installs; removing them is described
by their own projects.
