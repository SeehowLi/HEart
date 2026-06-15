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

## Target Interface

Call the skill explicitly with `$HEart` and choose the library target in the prompt:

```text
$HEart target=OpenFHE ...
$HEart target=Lattigo ...
$HEart target=FlyHE ...
$HEart target=flyfhe ...
```

Target aliases:

- `target=OpenFHE`: OpenFHE 1.5.1 / CPU / C++.
- `target=Lattigo`: Lattigo 6.2.0 / CPU / Go.
- `target=FlyHE`, `target=flyfhe`, `target=Phantom`, or `target=GPU`: Phantom/FlyHE GPU.

If no target is specified and the context does not make it obvious, HEart should ask one short target-selection question before proceeding.

For a step-by-step test-code tutorial, see [HEart/USAGE.md](HEart/USAGE.md).

## Delegate Memory Module

When HEart starts work on a concrete project, it should maintain a project-local delegate memory folder:

```text
.heart-memory/
  README.md
  project.md
  rules.md
  sessions/YYYY-MM-DD.md
  decisions.md
  artifacts.md
  open-questions.md
```

Initialize it with:

```powershell
HEart/scripts/init-heart-memory.ps1 `
  -ProjectRoot <your-project-root> `
  -ProjectSlug <project-slug> `
  -TargetLibrary OpenFHE `
  -ProjectSummary "encrypted logistic regression inference"
```

Use it as an append-only working log for basic project information, project-specific rules, decisions, changed files, unresolved facts, validation results, and follow-up tasks. This keeps project memory beside the project, while HEart core/libs stay reusable and unchanged.

## Minimal Example

User:

```text
Use HEart to design CKKS inference for logistic regression on an encrypted vector.
Target OpenFHE CPU.
```

Expected HEart behavior:

```text
Chosen target: OpenFHE 1.5.1 / CPU / C++.

Design Note:
1. Function: f(x)=sigmoid(<w,x>+b), encrypted x, plaintext w/b, encrypted probability output.
2. Circuit: ct-pt multiply, rotate-and-sum dot product, add plaintext bias, polynomial sigmoid approximation.
3. Packing: x[i] in slot i; rotations are powers of two for the slot sum, e.g. {1,2,4} for 8 features.
4. Parameters: choose N, scale, prime chain, slots, and 128-bit security for the depth; decide whether bootstrap is needed.
5. Keys: relin key if the chosen sigmoid circuit uses ciphertext products, rotation keys for the sum, bootstrap keys only if planned.

Please confirm this Design Note before I write code.
```

After confirmation, HEart should generate code with explicit level/scale tracking and a plaintext reference that reports slotwise max absolute/relative error.

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

## Star History

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=SeehowLi/HEart&type=Date&theme=dark" />
  <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=SeehowLi/HEart&type=Date" />
  <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=SeehowLi/HEart&type=Date" />
</picture>

## Notes

- OpenFHE and Lattigo version claims are pinned to local source versions used during distillation.
- Phantom/FlyHE has no confirmed semantic version in the local README/CMake files; this remains marked `[需人工确认 version]`.
- This repository contains distilled operational knowledge, not the upstream library source trees.
