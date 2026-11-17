MUSTI AND CHATGPT - SQUAD OPS
PSP SDK C project

PROJECT FILES
- main.c
- Makefile
- README.txt

BUILD
1. Put these files in one project folder in your PSP SDK environment.
2. Open a terminal in that folder.
3. Run: make
4. The PSP SDK build system should produce EBOOT.PBP.

CONTROLS
- D-pad: Move
- CROSS: Fire
- CIRCLE: Reload / return to menu
- START: Pause
- SELECT: Reserved

FEATURES IN THIS SOURCE
- 480x272 PSP GU rendering
- Intro credit: MUSTI AND CHATGPT
- Main menu and settings screen
- Language selector: Turkish, English, Portuguese, Arabic label set
- Squad companions with arcade cover fire
- Fictional rogue training robots
- Muzzle flash, projectile streaks, hit particles
- Health, ammo and score HUD
- Pause and mission result overlays
- Finite 10-robot clear objective

IMPORTANT COMPATIBILITY NOTES
- This is a PSP SDK C project, not Lua or HTML.
- It uses standard PSP SDK GU/CTRL headers and build.mak.
- The tiny built-in bitmap font covers Latin uppercase letters and digits.
  Arabic menu strings are included in the language table, but this tiny font
  does not shape/render Arabic glyphs. Arabic text will need a proper font
  renderer to display correctly.
- Sound toggle is a settings placeholder; sound effects are not yet implemented.
- The prototype is a top-down 2D arcade game, not a full 3D game.
- Test with your PSP SDK version. If the compiler reports a missing API or
  constant, share the exact error and SDK version for a targeted fix.

This is an original fictional arcade project. Opponents are training robots.
