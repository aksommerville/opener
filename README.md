# Opener of Cages

For [LowRezJam 2025](https://itch.io/jam/lowrezjam-2025), theme "LIBERATE".

Cloned from [Shovel](https://github.com/aksommerville/shovel) at 7796b03c7a349219ab776b02914edd5d40a101f2.

Dot rescues the circus animals.
I'm thinking Zelda-style adventure, with 1-bit graphics (coloring per tile at compositing).
I've already made the bare-minimum 1-bit renderer and square-only synth for Shovel, and want to try using those.
LowRez doesn't have a size limit, but it would amuse me to stay under 16 kB for the web package (`64*64*4`, the size of LowRez's framebuffer limit).
UPDATE: I blasted past 16 kB already, and really who cares.

## Building

 - Tweak `local/config.mk` to suit your environment, see comments there.
 - Linux: You'll need PulseAudio and xegl (you probably already have them).
 - Web: No tools necessary to build*, but you'll need npm `http-server` to run it. Can't run over file protocol.
 - [*] except a wasm-capable `clang`, that's not negotiable!
 - `make run` for native or `make serve` for web, or just `make` to build everything.
 
## Agenda

I'll be doing (GDEX Jam 2025)[https://itch.io/jam/gdex-game-jam-2025] at the same time, so mostly don't expect much the first week.

 - 2025-08-01T07:00 FRI: LowRez begins.
 - 2025-08-01T15:00 FRI: 4 hours of furious work on this. Can I get the skeleton of maps and sprites running?
 - 2025-08-01T19:00 FRI: GDEX begins.
 - 2025-08-08T23:00 FRI: Aim to be done with GDEX and switch fully to LowRez.
 - 2025-08-09T23:00 SAT: GDEX ends.
 - 2025-08-10T23:00 SUN: Gameplay mechanics complete.
 - 2025-08-15T23:00 FRI: Aim to have the major work complete: Maps, music, modals.
 - 2025-08-16T06:00 SAT: Testing and refinement.
 - 2025-08-16T23:00 SAT: Aim to be finished with the game (not ancillary materials).
 - 2025-08-17T06:00 SUN: Itch page, etc.
 - 2025-08-17T16:00 SUN: LowRez ends.
 
Refined agenda for the home stretch:
 - 2025-08-14 THU: Combat.
 - 2025-08-15 FRI: Combat continued.
 - 2025-08-16 SAT: Cleanup.
 - 2025-08-17 SUN: Itch page, test, submit by noon.

## TODO

- [ ] genioc_main.c:genioc_store_load,genioc_store_save: Implement if we need, and copy back to shovel.
- [ ] Eliminate AUX1-to-quit before release. (and provide a friendlier option).
- [ ] `evdev_shovel_glue.c`: Still have hard-coded devices. Implement general mapping and copy back to shovel. (native only, so not a big deal).
- [ ] I'm going to want a fullscreen toggle too. (xegl)
- [x] Repair songs. (long notes, and notes out of range, etc)
- [ ] Persist selection of sound and music enable.
- [ ] Hand-draw the inner bit for the itch cover.
- [ ] Slide into place if you near-miss a corner.
