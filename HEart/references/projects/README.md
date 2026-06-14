# Project Modules

This directory is reserved for v1.1+ derivative/project knowledge.

Use project modules for concrete applications built from the core HEart rules, especially OpenFHE-based work such as encrypted inference, private ML building blocks, or reusable CKKS circuit templates.

## How To Add A Project Module

1. Create `references/projects/<project-slug>.md`.
   - Use a short ASCII slug, for example `openfhe-logreg-inference`.
   - The file must cite relevant core references, such as `../core/01-mental-model.md`, `../core/02-op-semantics.md`, `../core/04-pattern-cookbook.md`, and `../core/05-params-playbook.md`.
   - The file may cite library references, such as `../libs/openfhe.md`, when the implementation depends on a specific library.
   - The file must not redefine or contradict core rules; it should specialize them for the project.

2. Append one row to `SKILL.md` under `Project Registry`.
   - Add only one row.
   - Do not reorder or edit existing rows.
   - Use the relative path `references/projects/<project-slug>.md`.

3. Do not modify any existing `references/core/` or `references/libs/` content.
   - Project modules must reference core/libs instead of changing them.
   - If a project exposes a real bug in core/libs, make that correction as a separate maintenance task with sources and progress notes.

## Required Project Module Shape

Each project module should include:

- Target library and version.
- Design Note: function shape/range, circuit/depth, packing/rotations, parameters/scale schedule, required keys.
- Implementation notes tied to the selected library.
- Validation plan: plaintext reference and slotwise max abs/rel error.
- Gotchas that are specific to this project.
- Sources: at least one core reference and, when applicable, one library reference.
