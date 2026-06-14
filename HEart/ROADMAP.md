# HEart Roadmap

## v1.1 Plan

HEart v1.1 will add project-derived knowledge for CKKS/FHE applications built on these libraries, with OpenFHE as the primary target and Lattigo / FlyHE added when the project genuinely needs them.

Planned focus:

- OpenFHE-based encrypted inference examples: logistic regression, small MLP blocks, polynomial activations, encrypted linear algebra, and bootstrapping placement notes.
- Project-specific parameter and scale schedules tied back to the core rules.
- Reusable gotchas from real builds: missing keys, wrong scale mode, parameter/security failures, bootstrap placement, validation harness mistakes.
- GPU project modules only when they add FlyHE-specific memory, transfer, stream, or kernel behavior beyond CPU CKKS rules.

## Extension Convention

- Put derivative/project knowledge in `references/projects/<project-slug>.md`.
- Every project module must cite relevant core files and must not rewrite core facts.
- Add a project by appending one row to `SKILL.md` `Project Registry`.
- Do not modify existing `references/core/` or `references/libs/` files when adding a project module.
- If a project reveals a true core/library correction, record it separately and update core/libs in a dedicated maintenance change.

See `references/projects/README.md` for the exact v1.1 project-module procedure.
