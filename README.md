# HEart

HEart is a Codex skill for designing, reviewing, and debugging CKKS fully homomorphic encryption code.

Version: `1.0`

It covers CPU libraries OpenFHE 1.5.1 and Lattigo 6.2.0, plus the local Phantom/FlyHE GPU code path whose semantic release version still needs manual confirmation.

## What It Provides

- A compact skill entry point: `HEart/SKILL.md`
- CKKS core mental models: scale, level, noise, RNS/NTT, ciphertext structure, operation semantics, failure modes, patterns, and parameter planning.
- Library maps for OpenFHE, Lattigo, and Phantom/FlyHE.
- GPU-specific guidance for VRAM budgeting, host-device transfers, streams, batching, NTT state, and synchronization.
- Project-module conventions for v1.1+ derivative work.

## Repository Layout

```text
HEart/
  SKILL.md
  CHANGELOG.md
  ROADMAP.md
  VALIDATION.md
  references/
    core/
    libs/
    projects/
  _selftest/
README.md
AGENTS.md
BUILD_PLAN.md
PROGRESS.md
```

The original library source trees and downloaded papers are local research inputs and are not part of the intended v1.0 release payload.

## Using The Skill

Attach or install the `HEart/` directory as a Codex skill. Trigger it with CKKS/FHE tasks such as:

- "Review this OpenFHE CKKS code."
- "Design a CKKS circuit for encrypted inference."
- "Debug scale mismatch after EvalMult/Rescale."
- "Plan FlyHE GPU memory and transfer behavior."
- "用同态加密做加密推理。"

If the target library is ambiguous, the skill should ask one short target-selection question before loading library-specific guidance.

## Mandatory CKKS Workflow

Before implementation, HEart requires a user-visible Design Note:

1. Restate the function, shapes, and dynamic range.
2. Express the arithmetic circuit and multiplicative depth.
3. Define packing and enumerate rotations.
4. Choose `N`, prime chain, scale, slots, security target, and bootstrap placement.
5. List required keys.
6. Implement with explicit level/scale tracking.
7. Validate against plaintext reference with slotwise max abs/rel error.

GPU targets additionally require a VRAM budget and host-device transfer plan.

## v1.0 Contents

Core references:

- `HEart/references/core/01-mental-model.md`
- `HEart/references/core/02-op-semantics.md`
- `HEart/references/core/03-invariants-gotchas.md`
- `HEart/references/core/04-pattern-cookbook.md`
- `HEart/references/core/05-params-playbook.md`
- `HEart/references/core/gpu-considerations.md`

Library references:

- `HEart/references/libs/openfhe.md`
- `HEart/references/libs/lattigo.md`
- `HEart/references/libs/phantom-flyhe.md`
- `HEart/references/libs/comparison.md`

Project extension entrypoint:

- `HEart/references/projects/README.md`

## Status

- v1.0 consistency audit: see `HEart/VALIDATION.md`.
- End-to-end self-test report: see `HEart/_selftest/REPORT.md`.
- Future work: see `HEart/ROADMAP.md`.

## Notes

- OpenFHE and Lattigo version claims are pinned to local source versions used during distillation.
- Phantom/FlyHE has no confirmed semantic version in the local README/CMake files; this remains marked `[需人工确认 version]`.
- This repository contains distilled operational knowledge, not the upstream library source trees.
