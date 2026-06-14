# HEart v1.0 Build Plan

## Phase 0 - Workspace Scaffold
## Phase 1 - Source Inventory and Version Anchoring
## Phase 2 - Core FHE/CKKS Decision Knowledge
## Phase 3 - Library-Specific Operational References
## Phase 4 - Paper Knowledge Distillation
## Phase 5 - HEart Skill Assembly
## Phase 6 - Validation and Packaging

## 待补

- Tighten `HEart/SKILL.md` Mandatory Workflow: after target selection, the agent must show the Design Note as a user-visible artifact before creating/editing code; implementation should wait for confirmation unless the user explicitly pre-approved code generation.
- Add self-test isolation guidance: future fresh-session tests should run against a sandbox copy or write only under `HEart/_selftest/` unless the test intentionally validates real artifact generation.
- Decide whether to keep `openfhe-logreg-inference` as a project-registry example or move future examples out of the core skill package; current registry is append-only, so cleanup needs explicit approval.
- Decide whether to keep `flyhe-batched-ckks-matmul` as a project-registry example; it was created by a fresh-session GPU self-test and should be retained only if project examples are intended in the distributed skill.
- Improve validation observability: self-test prompts should ask the fresh session to record loaded HEart reference files in a test transcript, or scoring should explicitly rely on behavioral evidence only.
- Add a stronger target-selection gate to `HEart/SKILL.md`: when CPU/GPU/library is absent, ask the target question and stop unless the user explicitly asks for the default path in the same prompt.
- Add a CUDA validation task for the FlyHE MatMul path in Linux/WSL with `nvcc` available; current GPU self-test only produced a design/run note and did not compile or run FlyHE.
