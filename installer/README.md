# Windows installer

`MyRevoke.iss` is an [Inno Setup](https://jrsoftware.org/isdl.php) 6.3+ script
that turns the staged editor folder into a single setup executable.

1. Stage the editor, so `dist\MyRevoke\RevokeCraft.exe` exists.
2. `iscc installer\MyRevoke.iss`

The result is `dist\MyRevoke-Setup-<version>.exe`, versioned from the `VERSION`
file in the repo root.

Setup installs to `Program Files\MyRevoke` with a Start Menu shortcut, an
optional desktop shortcut and an uninstaller, and downloads the Microsoft
Visual C++ 2015-2022 Redistributable (x64) if the machine doesn't already have
it. User projects are created wherever the user puts them, never in the install
folder, so uninstalling leaves them alone.

The setup executable is unsigned, so Windows SmartScreen warns about it.
