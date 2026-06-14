---
name: HEart
version: "1.0"
description: Use when writing, reviewing, debugging, or designing fully homomorphic
  encryption (CKKS) code. Supports CPU libraries OpenFHE (1.5.1) and Lattigo (6.2.0),
  and GPU library Phantom/FlyHE [需人工确认 version]. Covers prime-chain/parameter selection, slot packing
  and rotation planning, scale/level/depth (modulus chain) management, rescaling,
  relinearization, Galois/rotation and bootstrapping keys, polynomial
  (Chebyshev/Paterson-Stockmeyer) approximation of nonlinear functions
  (sign, comparison, ReLU, sigmoid, 1/x, sqrt), CKKS bootstrapping placement, GPU
  device-memory and host-device-transfer concerns, and validating approximate results
  against a plaintext reference. Trigger on CKKS, RNS-CKKS, FHE, homomorphic
  encryption, ciphertext, EvalMult/Rescale/Rotate, bootstrap, target=OpenFHE,
  target=Lattigo, target=FlyHE, target=flyfhe, OpenFHE, Lattigo, Phantom, FlyHE,
  GPU FHE, 全同态加密, 同态加密, 隐私计算, 加密推理, CKKS 方案.
---

## Target Selection(路由)

## Invocation Interface

Use this skill explicitly as `$HEart` when the user wants FHE/CKKS design, code,
review, or debugging help. The caller may select the library with a short target
field and optional project slug in the prompt. Treat this as the stable v1.0 interface:

```text
$HEart target=OpenFHE project=<slug> ...
$HEart target=Lattigo project=<slug> ...
$HEart target=FlyHE project=<slug> ...
$HEart target=flyfhe project=<slug> ...
```

Accepted target aliases:

- `OpenFHE`, `openfhe`, `OpenFHE 1.5.1`: route to OpenFHE 1.5.1 / CPU / C++ and load `references/libs/openfhe.md`.
- `Lattigo`, `lattigo`, `Lattigo 6.2.0`: route to Lattigo 6.2.0 / CPU / Go and load `references/libs/lattigo.md`.
- `FlyHE`, `flyhe`, `flyfhe`, `Phantom`, `GPU`: route to Phantom/FlyHE GPU and load `references/libs/phantom-flyhe.md` plus `references/core/gpu-considerations.md`.

If the prompt omits the target, apply the ambiguity gate below.

## Delegate Memory Module

For every new concrete project that uses HEart, create or update a project-local delegate memory folder at `.heart-memory/` in that project's root. This is the project's live memory, separate from HEart's reusable core/libs.

Required files:

- `.heart-memory/project.md`: project slug, target library, scope, inputs/outputs, dynamic-range assumptions, source references.
- `.heart-memory/rules.md`: project-specific rules, constraints, and "do not" items.
- `.heart-memory/sessions/YYYY-MM-DD.md`: append-only session log.
- `.heart-memory/decisions.md`: durable decisions and rationale.
- `.heart-memory/artifacts.md`: files created/modified and why.
- `.heart-memory/open-questions.md`: unresolved facts, marked `[需人工确认]` when needed.

Initialize it with `scripts/init-heart-memory.ps1 -ProjectRoot <path> -ProjectSlug <slug> -TargetLibrary <OpenFHE|Lattigo|FlyHE> -ProjectSummary "<summary>"` when the script is available. If not, create the same files manually. Update memory continuously during work and before returning.

First route the task:

- Ambiguity gate: If CPU/GPU or the specific library is not specified and cannot be inferred from context (code, imports, hardware), ask exactly ONE short target-selection question and stop. Do not load library-specific files, choose OpenFHE by default, write code, or create files until the user answers. If the user explicitly asks for a default, choose OpenFHE (CPU) and state that choice.
- Use CPU when correctness, maintainability, mature bootstrap APIs, ordinary deployment, or small/medium workloads dominate.
- Use GPU when the workload is large and batched, data can stay on device, extreme throughput matters, and the deployment has controlled NVIDIA/CUDA hardware.
- Within CPU, prefer OpenFHE for C++ workflows, broad CKKS/FHE features, AUTO scale modes, and mature bootstrap examples.
- Within CPU, prefer Lattigo for Go workflows, explicit scale/level control, and Go-native service integration.

Load references:

- All targets: [core/01-mental-model.md](references/core/01-mental-model.md), [core/02-op-semantics.md](references/core/02-op-semantics.md), [core/03-invariants-gotchas.md](references/core/03-invariants-gotchas.md), [core/04-pattern-cookbook.md](references/core/04-pattern-cookbook.md), [core/05-params-playbook.md](references/core/05-params-playbook.md).
- Layering note: keep loading all core 01-05 for safety; 01-03 are always required, 04 is for circuit/nonlinear design, and 05 is for parameter/security decisions.
- OpenFHE: add [libs/openfhe.md](references/libs/openfhe.md).
- Lattigo: add [libs/lattigo.md](references/libs/lattigo.md).
- GPU Phantom/FlyHE: add [libs/phantom-flyhe.md](references/libs/phantom-flyhe.md) and [core/gpu-considerations.md](references/core/gpu-considerations.md).
- If target selection is unclear: add [libs/comparison.md](references/libs/comparison.md).

## Mandatory Workflow

Before writing any CKKS code or reviewing a CKKS implementation:

When HEart accepts a concrete project, initialize or update the Delegate Memory Module before implementation work. Record project basics, rules, decisions, modified artifacts, open questions, and validation results there throughout the session. This memory is project-local and must not be written back into HEart core/libs unless the user explicitly asks for a reusable skill update.

Steps 1-5 must be written out as a short **Design Note** and shown to the user as the next user-visible artifact BEFORE any code or durable file creation. After showing the Design Note, wait for user confirmation before creating/editing code, examples, project notes, or registry entries, unless the user explicitly requested "write the Design Note to a file now" or has already approved implementation in the same request. If depth/precision/security do not fit, iterate the note first.

1. Restate `f`: input/output shapes and dynamic range.
2. Express it as an arithmetic circuit: multiplicative depth, total multiplication count; for nonlinear functions choose the approximation polynomial and its degree/depth.
3. Define packing layout: input-to-slot mapping; enumerate every rotation step.
4. Choose parameters: `N`, prime chain, `Delta`, and slots to satisfy depth, precision, and 128-bit security; decide whether/where to bootstrap. Pick the scale-management strategy and write the rescale schedule: OpenFHE scaling mode (`FIXEDMANUAL`/`FIXEDAUTO`/`FLEXIBLEAUTO(EXT)`); Lattigo explicit Scale alignment; SEAL manual. The schedule lists the scale and level of every intermediate ciphertext.
5. List keys to generate: relinearization, Galois/rotation set, bootstrap keys.
6. Implement with explicit level and scale tracking for every ciphertext via comments or assertions; never branch on ciphertext data.
7. Validate against a plaintext reference, compare slot by slot, and report max absolute/relative error.
8. For GPU targets, add a VRAM budget: ciphertexts, temporaries, relin/Galois/bootstrap keys, linear-transform plaintexts, and conversion keys.
9. For GPU targets, add a transfer plan: every host-device boundary, expected stream/synchronization points, and how batching keeps data resident.

## Self-Test Mode

When evaluating this skill under `HEart/_selftest/`, write all generated artifacts only under `HEart/_selftest/<test>/artifacts/` unless the test explicitly says to modify the real skill. Do not append `Project Registry`, create durable examples, or update project references during self-tests.
For self-test scoring only, include a short `Loaded HEart references:` line that names the HEart reference files actually used; outside self-tests, keep normal responses concise.

## Self-Check Before Returning

- Append the session outcome to `.heart-memory/sessions/YYYY-MM-DD.md` when working in a concrete project.
- Update `.heart-memory/artifacts.md`, `.heart-memory/decisions.md`, and `.heart-memory/open-questions.md` if files, design choices, or unresolved facts changed.
- Re-verify every Never Do item.
- Check the final code's per-ciphertext level/scale against the Design Note schedule.
- Confirm all rotation/relin/bootstrap keys used are actually generated.
- Confirm a plaintext reference plus slotwise max abs/rel error harness is included.

## Never Do

- Never branch on ciphertext values.
- Never forget that ciphertext-ciphertext multiplication produces an extended ciphertext until relinearized.
- Never forget that scale-growing ciphertext/plaintext or ciphertext/ciphertext multiplication needs a rescale schedule.
- Never operate on mismatched scale, level, chain index, or NTT state without an explicit alignment step.
- Never rely on encrypted output without a plaintext reference and slotwise error report.
- Never ship parameters without checking security, including special primes `P` and bootstrap parameters.
- Never expose raw CKKS decryptions to untrusted parties; model approximate-decryption leakage and apply noise flooding when needed.

## Reference Index

| Concern | References |
|---|---|
| CKKS mental model, scale/level/noise, RNS/NTT, ciphertext structure | [core/01-mental-model.md](references/core/01-mental-model.md) |
| Operation semantics, keys, rescale/mod-switch naming | [core/02-op-semantics.md](references/core/02-op-semantics.md) |
| Failure modes and debugging gotchas | [core/03-invariants-gotchas.md](references/core/03-invariants-gotchas.md) |
| Packing, rotations, linear algebra, polynomial/nonlinear recipes, bootstrap placement | [core/04-pattern-cookbook.md](references/core/04-pattern-cookbook.md) |
| Parameter selection, prime chains, security checks | [core/05-params-playbook.md](references/core/05-params-playbook.md) |
| OpenFHE 1.5.1 APIs, AUTO/FIXED scaling, bootstrapping | [libs/openfhe.md](references/libs/openfhe.md) |
| Lattigo 6.2.0 APIs, explicit rescale, Go paths, bootstrapping | [libs/lattigo.md](references/libs/lattigo.md) |
| Phantom/FlyHE GPU APIs, CUDA NTT, key switching, examples | [libs/phantom-flyhe.md](references/libs/phantom-flyhe.md) |
| GPU memory, transfers, streams, batching | [core/gpu-considerations.md](references/core/gpu-considerations.md) |
| Library selection | [libs/comparison.md](references/libs/comparison.md) |

## Project Registry(v1.1+,append-only)

For v1.1+ project-specific or derivative knowledge, append one row here and write details under `references/projects/<project-slug>.md`. Project notes must cite core references and must not modify core files.

| project | applicable libraries | file path | summary |
|---|---|---|---|
| openfhe-logreg-inference | OpenFHE 1.5.1 | references/projects/openfhe-logreg-inference.md | CKKS encrypted-vector logistic regression inference with plaintext weights, EvalSum dot product, EvalLogistic sigmoid, and slotwise validation. |
| flyhe-batched-ckks-matmul | FlyHE / Phantom GPU | references/projects/flyhe-batched-ckks-matmul.md | Batched CKKS matrix multiplication on FlyHE GPU using the local MMEvaluator path, with packing, key, scale, VRAM, transfer, and runbook constraints. |
