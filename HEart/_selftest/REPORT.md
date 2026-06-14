# HEart End-to-End Self-Test Report

Date: 2026-06-14

Method: fresh sub-agent sessions were started with no parent conversation context. Each received only the `HEart` skill folder as an attached skill path plus the user prompt. Outputs and observed workspace side effects were saved under `HEart/_selftest/`.

## Summary

| Test | Scenario | Overall | Main finding |
|---|---|---|---|
| Test 1 | Chinese trigger + encrypted-vector logistic regression inference | PASS after fix | After strengthening `SKILL.md`, the agent asked exactly one target-selection question and stopped; it did not silently default to OpenFHE or write files. |
| Test 2 | GPU path + FlyHE large-batch CKKS matrix multiplication | PASS with caveat | It selected FlyHE GPU and produced a strong design/run note with VRAM and transfer planning. The durable-file caveat is addressed by the new self-test isolation rule. |

## Test 1 Findings After Fix

- Skill trigger: PASS. Evidence: response offered the three supported HEart routes: OpenFHE CPU, Lattigo CPU, and FlyHE/Phantom GPU.
- CPU/library resolution: PASS. Evidence: the ambiguous prompt produced exactly one target-selection question.
- Design Note before implementation: PASS. Evidence: no code, examples, project notes, or registry entries were created in the rerun.
- Design Note fields: N/A for this rerun. Correct behavior is to wait for target selection before producing a Design Note.
- Plaintext reference + slotwise error: N/A for this rerun. Correct behavior is to wait for target selection and Design Note approval before implementation.

## Test 2 Findings

- GPU/FlyHE route: PASS. Evidence: output and project note target FlyHE / Phantom GPU CKKS.
- Reference behavior: PASS by behavioral evidence. The note uses FlyHE paths and GPU concepts from `phantom-flyhe.md` and `gpu-considerations.md`.
- Design Note: PASS. Evidence: generated note contains `CKKS Design Note` with function, circuit, packing, parameters/scale schedule, and keys.
- VRAM budget: PASS. Evidence: includes ciphertext-size formula and enumerates ciphertexts, keys, plaintexts, and temporaries.
- Host-device transfer plan: PASS. Evidence: lists host/device boundaries and flags `multiply_power_of_x` D2H/H2D round trips.
- Caveat addressed for future runs: `SKILL.md` now includes self-test isolation and a `Loaded HEart references:` observability line for self-test scoring only.

## Fixes Applied

| Previous gap | Fix applied | Current status |
|---|---|---|
| Ambiguous-target gate was not enforced strongly enough | `SKILL.md` now says ambiguous CPU/GPU/library must ask exactly one target-selection question and stop; it must not load library-specific files, choose OpenFHE by default, write code, or create files until the user answers. | PASS in Test 1 rerun |
| Design Note gate was not enforced strongly enough | `SKILL.md` now says Steps 1-5 must be the next user-visible artifact before code or durable files, and that implementation/file writes require user confirmation unless explicitly requested. | PASS in Test 1 rerun |
| Self-tests were not isolated | `SKILL.md` now requires self-test artifacts to stay under `HEart/_selftest/<test>/artifacts/` unless the test explicitly modifies the real skill. | Fixed for future self-tests |
| Reference loading was not observable | `SKILL.md` now asks self-test runs to include a `Loaded HEart references:` line naming the HEart reference files actually used. | Fixed for future self-tests |

## Remaining / Manual Decisions

- Project Registry still contains test-derived projects: `openfhe-logreg-inference` and `flyhe-batched-ckks-matmul`. Because registry is append-only, cleanup needs explicit approval.
- FlyHE runtime remains unverified; earlier Test 2 found `nvidia-smi` but no `nvcc` in PATH.

## Output Files

- `HEart/_selftest/test1/transcript.md`
- `HEart/_selftest/test1/score.md`
- `HEart/_selftest/test2/transcript.md`
- `HEart/_selftest/test2/score.md`
- `HEart/_selftest/REPORT.md`
