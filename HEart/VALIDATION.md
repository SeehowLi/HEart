# HEart Validation

Date: 2026-06-14

Scope: `HEart/SKILL.md`, all files under `HEart/references/`, `HEart/CHANGELOG.md`.

## Summary

The skill package is internally linked and usable as a compact CKKS/FHE routing skill. This audit fixed two consistency issues:

- `SKILL.md` no longer claims `Phantom/FlyHE(1.0.0)`; the local FlyHE README/CMake audit found no semantic version, so it now says `Phantom/FlyHE [需人工确认 version]`.
- `references/libs/openfhe.md` now has an explicit `Conjugate` row in the API mapping table, aligned with `references/core/02-op-semantics.md`.

## Checks

| Check | Status | Evidence |
|---|---|---|
| Every reference file is linked by `SKILL.md` | PASS | All 10 files under `references/` are directly linked from the Target Selection / Reference Index sections of [SKILL.md](SKILL.md). |
| Internal relative Markdown links resolve | PASS | Checked 68 relative Markdown links under `HEart/**/*.md`; broken links: 0. |
| Operator terminology is consistent across core and libs | PASS | Core primitives in [02-op-semantics.md](references/core/02-op-semantics.md) are mirrored by OpenFHE, Lattigo, and Phantom/FlyHE mapping tables, including `Conjugate`, `Rescale / ModReduce`, and `ModSwitch / DropLevel`. |
| Three library versions are pinned | PASS WITH UNCERTAINTY | OpenFHE 1.5.1 and Lattigo 6.2.0 are pinned. FlyHE is pinned to the local source tree, but no semantic version was found, so docs mark `[需人工确认 version/tag]`. |
| Gotchas use symptoms/cause/fix shape | PASS | [03-invariants-gotchas.md](references/core/03-invariants-gotchas.md) uses `Symptoms: ... Cause: ... Fix: ...` for each gotcha, equivalent to `症状 -> 成因 -> 修法`. |
| Patterns include depth cost and required keys | PASS | Each recipe in [04-pattern-cookbook.md](references/core/04-pattern-cookbook.md) follows `Applicable | Depth cost | Required keys | Packing/range assumptions | Pseudocode`. |
| Description covers English and Chinese triggers | PASS | `SKILL.md` description includes CKKS/RNS-CKKS/FHE/OpenFHE/Lattigo/Phantom/FlyHE/GPU FHE plus `全同态加密`, `同态加密`, `隐私计算`, `加密推理`, `CKKS 方案`. |
| Version claims are aligned across entrypoint and libs | PASS AFTER FIX | `SKILL.md`, [comparison.md](references/libs/comparison.md), and [phantom-flyhe.md](references/libs/phantom-flyhe.md) now agree that FlyHE's semantic version is unresolved. |

## Residual Uncertainties

- FlyHE / Phantom semantic version or release tag still needs manual confirmation from an upstream release, tag, or author-provided version source.
- OpenFHE's preferred public user-facing conjugation wrapper remains `[需人工确认]`; local source confirms CKKS conjugation and conjugation-key usage through `FHECKKSRNS::Conjugate`, `ConjugateKeyGen`, and `EvalAutomorphism`.
- This audit did not build or run the libraries; it checked documentation consistency and local Markdown link integrity.
