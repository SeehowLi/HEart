# Test 2 Score

Prompt: GPU path, large-batch CKKS matrix multiplication, explicitly using FlyHE.

| Item | Result | Evidence |
|---|---|---|
| 1. GPU/FlyHE target selected | PASS | The fresh-session output and generated project note both target `FlyHE / Phantom GPU CKKS`. |
| 2. `gpu-considerations` plus `phantom-flyhe` behavior | PASS | The generated note uses FlyHE-specific local paths, MatMul examples, `galois_elts`, `create_galois_keys_from_elts`, `bootstrap` policy, CUDA/GPU timing concerns, `is_ntt_form`, host-device boundaries, and `multiply_power_of_x` D2H/H2D behavior. This is behavioral evidence; the final answer does not expose internal reference-load logs. |
| 3. Design Note present | PASS | `HEart/references/projects/flyhe-batched-ckks-matmul.md` contains `CKKS Design Note` with function, circuit, packing/rotations, parameters/scale schedule, and required keys. |
| 4. Design Note before code | PARTIAL | The fresh session did not write implementation code and did produce a design/run note. However, it wrote project files directly before returning the Design Note to the user. This is acceptable for a design-note artifact but still shows the same workflow-isolation weakness as Test 1. |
| 5. VRAM budget | PASS | The note includes `bytes(ct) ~= components * active_primes * N * 8`, estimates 2- and 3-component ciphertext sizes, and lists compressed ciphertexts, expanded ciphertexts, plaintext vectors, output ciphertexts, Galois keys, and temporaries. |
| 6. Host-device transfer plan | PASS | The note lists host input, encode/encrypt, GPU eval, decrypt/decode, validation-only transfer boundaries, and flags `multiply_power_of_x` as a D2H/H2D transfer hazard. |

Overall: PASS with workflow caveat.

Exposed gaps:

- The GPU references are strong enough for a FlyHE design note, but the final answer does not explicitly state which HEart reference files were loaded.
- The fresh session created durable project artifacts outside `_selftest`; future tests should sandbox or instruct all writes into a test directory.
- FlyHE was not compiled or run because `nvcc` was not in PATH.
