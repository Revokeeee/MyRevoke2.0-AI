# Math

A small, standalone math utility: TRS (translation/rotation/scale) decomposition of a 4x4 transform
matrix, used by the editor's transform inspector/gizmos.

## Files

- **`Math.h`/`.cpp`** — `DecomposeTransform(const glm::mat4&, glm::vec3& translation, glm::vec3&
  rotation, glm::vec3& scale)`, adapted from glm's internal `matrix_decompose.inl`. Returns `false` if
  the matrix is degenerate (zero perspective term).

Used by `Scene/Components.h` (for editing `TransformComponent` in the inspector) and
`Renderer/EditorCamera.cpp`.

## Known issues

- A block of the original glm logic — the coordinate-flip/determinant check that corrects scale sign
  for left-handed/flipped matrices — is disabled behind `#if 0` in `Math.cpp`, so this diverges
  silently from upstream glm behavior for flipped matrices.
