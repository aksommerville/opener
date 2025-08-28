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

## TODO

- [ ] I'm going to want a fullscreen toggle. (xegl)
