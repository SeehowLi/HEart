# HEart Changelog

## v1.0 Formal - 2026-06-14

HEart v1.0 is the first complete CKKS/FHE skill package.

Core references:

- `references/core/01-mental-model.md`
- `references/core/02-op-semantics.md`
- `references/core/03-invariants-gotchas.md`
- `references/core/04-pattern-cookbook.md`
- `references/core/05-params-playbook.md`
- `references/core/gpu-considerations.md`

Library references:

- `references/libs/openfhe.md`
- `references/libs/lattigo.md`
- `references/libs/phantom-flyhe.md`
- `references/libs/comparison.md`

Project extension references:

- `references/projects/README.md`
- `references/projects/openfhe-logreg-inference.md`
- `references/projects/flyhe-batched-ckks-matmul.md`

Release checks:

- `SKILL.md` frontmatter version is `"1.0"`.
- `VALIDATION.md` records reference/link/operator consistency checks.
- `_selftest/REPORT.md` records end-to-end self-test status and gate-hardening follow-up.
- v1.0 invocation interface accepts `$HEart target=OpenFHE`, `$HEart target=Lattigo`, `$HEart target=FlyHE`, and `$HEart target=flyfhe`.

## v1.0 Draft History - 2026-06-14

- Created the initial `SKILL.md` entry point for CKKS/FHE design, implementation, review, and debugging.
- Added target routing for OpenFHE 1.5.1, Lattigo 6.2.0, and Phantom/FlyHE GPU workflows.
- Linked the compact skill surface to detailed references under `references/core/` and `references/libs/`.
- Added mandatory CKKS design workflow, GPU memory/transfer additions, Never Do rules, reference index, and append-only project registry stub.
- Updated trigger terms with Chinese CKKS/FHE phrases, added target ambiguity handling, made the pre-code Design Note mandatory, added scale/rescale scheduling, and added the final self-check gate.
- Added validation/audit notes, corrected the unresolved Phantom/FlyHE version claim, and aligned OpenFHE's operation table with core `Conjugate` terminology.
- Hardened the target-selection and Design Note gates after self-test failure: ambiguous targets now require one question and stop, and implementation/file writes require user confirmation after the visible Design Note.
- Added self-test isolation and reference-load observability guidance for future HEart validation runs.
