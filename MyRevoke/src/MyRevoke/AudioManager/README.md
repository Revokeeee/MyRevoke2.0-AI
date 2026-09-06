# AudioManager

Wraps OpenAL (plus libsndfile for decoding) to give the ECS a simple 3D positional sound-effect
component.

## Files

- **`AudioRenderer.h`/`.cpp`** — `Init()`/`Shutdown()` manage the ALC device/context.
  `CreateSoundBuffer(filename)` decodes a whole file via libsndfile into an OpenAL buffer.
  `CreateSoundSource`/`UpdateSoundSource`/`RemoveSoundSource` manage AL sources (pitch, gain,
  position, velocity, looping). Consumed by `Scene/Components.h`'s `SoundComponent`.
- **`MusicBuffer.h`/`.cpp`** — intended as a streaming/queued-buffer music player. **Entirely
  commented out** — dead code, never finished.

## Known issues

- `MusicBuffer` is fully commented out and could be removed; the frozen-in-comment code also has bugs
  of its own (a duplicate `ALuint buffer` declaration, and an inverted `if` that would throw on
  success rather than failure).
- Only mono/stereo/(rare) 3–4 channel Ambisonic B-format are handled in `CreateSoundBuffer`; any other
  channel count silently fails with just an error log, no fallback.
- Uses raw `malloc`/`free` for sample buffers rather than RAII containers, inconsistent with the rest
  of the codebase's use of smart pointers.
