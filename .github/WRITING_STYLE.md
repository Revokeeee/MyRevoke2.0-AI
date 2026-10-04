# Writing style

For everything written here: PR descriptions, issues, review comments. Short, simple, technical.

- Say **what** changed and **why**. Not how; the diff shows how.
- Plain words, short sentences, one idea each. No filler ("This PR aims to", "it is worth noting").
- Bullets over paragraphs. One line per bullet.
- Be exact: `file:line`, function names, error text. Precision, not length.
- Don't restate the diff or list every file touched.
- Don't argue with the reviewer in advance. If something is deliberate, say so in one line.
- A section with nothing to say gets "None." Never pad it.

## Limits

- **PR description:** summary 3 sentences max. Whole body under 150 words. Each Notes section 3 bullets max.
- **Issue:** Context 3 sentences max. What to do 5 bullets max. Acceptance criteria 4 bullets max. Under 200 words. Audit or parent issues that list verified findings may run longer.
- **Review finding:** one bullet, `file:line` problem, then the fix. No intro, no closing summary, no praise.
- **Comments** (`Fixes applied:`, `NEEDS DECISION:`): 4 lines max.

## Example PR body

```
## PR summary
UUIDs came from hidden static state, so tests couldn't seed them. Adds a seedable
`UuidGenerator`; `UUID()` now uses one shared instance. Closes #14.

## Checklist
- [x] Tests added or updated
- [ ] Documentation updated
- [ ] Needs close human review

Tests: `UuidGeneratorTests.cpp` (same seed, same sequence; ids stay unique).

## Notes for AI review
None.

## Notes for human review
- `UUID::Get()` now returns `uint64_t` (was `int64_t`). The only caller already used `uint64_t`.
```
