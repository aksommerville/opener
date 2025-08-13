# Opener of Cages

For [LowRezJam 2025](https://itch.io/jam/lowrezjam-2025), theme "LIBERATE".

Cloned from [Shovel](https://github.com/aksommerville/shovel) at 7796b03c7a349219ab776b02914edd5d40a101f2.

Dot rescues the circus animals.
I'm thinking Zelda-style adventure, with 1-bit graphics (coloring per tile at compositing).
I've already made the bare-minimum 1-bit renderer and square-only synth for Shovel, and want to try using those.
LowRez doesn't have a size limit, but it would amuse me to stay under 16 kB for the web package (`64*64*4`, the size of LowRez's framebuffer limit).

## Building

 - Tweak `config.mk` to suit your environment, see comments there.
 - Linux: You'll need PulseAudio and xegl (you probably already have them).
 - Web: No tools necessary to build, but you'll need npm `http-server` to run it. Can't run over file protocol.
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

## TODO

- [x] Using a 3x5 font, so store them glyphwise in 16 bits each.
- [x] Map format and loader.
- - [x] Change it to use just one map, and scroll freely.
- [x] Map editor.
- [x] Render maps.
- [x] Sprites, generically.
- [x] Transitions. XXX just one map.
- [x] Try locking the camera until you approach the edge, and slide to positions not an entire screen apart.
- - Stop should be a factor of (84,41). Um. 41 is prime. Bump world to (100,51).
- - XXX There just isn't enough screen for that.
- - [x] But what if we did it based on entire screens?
- - - Yes I prefer this.
- [x] Hero movement, collisions, trigger transitions.
- [ ] Enemies.
- [ ] Combat: Shoot enemies with your wand and they are disabled until you leave the room.
- [ ] Damage: No death. Full Moon style, getting hit interrupts and pushes you.
- [ ] Spell casting.
- - [ ] Spell of Opening
- - [ ] Spell of Slumber
- - [ ] Need a few more. And also some hidden ones.
- [x] Animals to rescue.
- [ ] Game over. Win only, you play until you win or give up.
- [x] Dialogue.
- [ ] Proper maps and graphics.
- [x] Hello modal.
- [ ] Music.
- [ ] Sound effects.
- [ ] genioc_main.c:genioc_store_load,genioc_store_save: Implement if we need, and copy back to shovel.
- [ ] Eliminate AUX1-to-quit before release. (and provide a friendlier option).
- [ ] `evdev_shovel_glue.c`: Still have hard-coded devices. Implement general mapping and copy back to shovel. (native only, so not a big deal).
- [ ] I'm going to want a fullscreen toggle too. (xegl)
- [ ] Newly-liberated animals should walk to the path, don't snap to it until you've reached your place for the first time.
