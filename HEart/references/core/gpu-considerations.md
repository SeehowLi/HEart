# GPU FHE Programming Considerations

Chinese summary: GPU FHE is not just CPU FHE with faster multiplication. The main design variables are device memory residency, host-device transfer count, launch/synchronization overhead, stream/batch structure, and the size of evaluation keys. These are visible in FlyHE/Phantom's CUDA implementation.

## Decision Rules

| Concern | Rule | Why it matters | Sources |
|---|---|---|---|
| Device residency | Keep ciphertexts, plaintexts, NTT tables, and evaluation keys on the GPU across the whole pipeline. | Host-device transfers and stream sync can erase GPU gains. Encode/decode and debug print cross the boundary. | `FlyHE-main/FlyHE-main/RLWE/phantom/src/batchencoder.cu`; `RLWE/phantom/include/ckks_evaluator.cuh` |
| Memory budget | Budget before choosing `N`, levels, special primes, rotations, and bootstrap. | A CKKS ciphertext costs roughly `components * active_primes * N * 8` bytes, before allocator overhead. Relin/Galois/bootstrap keys can dominate. | `RLWE/phantom/include/ciphertext.h`; `RLWE/boot/examples/bootstrapping.cu` |
| Transfers | Move raw data at pipeline boundaries only: host vector -> encode/encrypt -> GPU evaluation -> decrypt/decode -> host vector. | FlyHE encode/decode use async H2D/D2H copies and decode synchronizes. Repeated inspect/print is expensive. | `RLWE/phantom/src/batchencoder.cu`; `conversion/include/rlwe.cuh` |
| Kernel launches | Fuse or batch small operations when possible; avoid long chains of tiny scalar kernels if one batched kernel can do the work. | Additions and scalar ops are cheap mathematically but still launch kernels. Launch overhead becomes visible for small `N` or few ciphertexts. | `RLWE/phantom/src/evaluate.cu`; `RLWE/phantom/examples/4_kernel_fusing.cu` |
| Streams | Use one explicit stream per independent pipeline; do not assume default-stream overlap. | FlyHE has a non-blocking default stream wrapper, but several wrappers synchronize after rotation/bootstrap. | `RLWE/phantom/include/cuda_wrapper.cuh`; `RLWE/phantom/include/ckks_evaluator.cuh`; `RLWE/boot/src/bootstrapping/Bootstrapper.cu` |
| Batch/slot parallelism | Fill slots and batch LWE/CKKS conversions; amortize keys and launches over many slots/ciphertexts. | GPU kernels parallelize over coefficients, RNS towers, slots, and many LWE ciphertexts. Underfilled slots waste bandwidth and key cost. | `RLWE/phantom/examples/3_ckks.cu`; `conversion/example/repack.cu` |
| NTT state | Treat NTT form as an invariant in CKKS hot paths. Convert only at explicit algorithm boundaries. | CKKS evaluator functions often require `is_ntt_form == true`; conversion/extraction may require non-NTT form. | `RLWE/phantom/src/evaluate.cu`; `conversion/include/extract.cuh` |
| Key switching | Count relin, rotate, conjugate, and bootstrap linear transforms as GPU key-switch workloads, not just algebraic metadata. | Key switching does mod-up, inner products with large eval keys, mod-down, and device memory traffic. | `RLWE/phantom/src/eval_key_switch.cu`; `RLWE/phantom/src/evaluate.cu` |
| Precision | GPU acceleration does not change CKKS scale/noise rules. Track scale/level exactly and compare approximate errors. | FlyHE has commented-out ciphertext-ciphertext scale checks in multiply, so wrong schedules may run and only fail numerically. | `RLWE/phantom/src/evaluate.cu`; `RLWE/phantom/examples/3_ckks.cu` |

## Memory Budget

Approximate live memory for one CKKS ciphertext:

```text
bytes(ct) ~= components * active_rns_primes * N * 8
```

Decision rules:

- Fresh/relinearized CKKS ciphertexts usually have `components = 2`; unrelinearized products have `components = 3`. Relin early when memory pressure matters; lazy relin only when it reduces total key-switch work and temporary memory still fits. Source: `RLWE/phantom/include/ciphertext.h`, `RLWE/phantom/src/evaluate.cu`.
- Every extra scaling prime increases every ciphertext, plaintext, key-switch temporary, and NTT table footprint. More levels are not free on a GPU. Source: `RLWE/phantom/include/ciphertext.h`, `RLWE/phantom/src/eval_key_switch.cu`.
- Galois keys are often the largest user-visible key set. Bootstrapping examples generate powers-of-two plus BSGS linear-transform rotations and print Galois key size. Source: `RLWE/boot/examples/bootstrapping.cu`, `RLWE/boot/src/bootstrapping/Bootstrapper.cu`.
- Bootstrapping also stores linear-transform coefficients as plaintext vectors (`fftcoeff_plain*`, `invfftcoeff_plain*`) and modular-reduction polynomials. Source: `RLWE/boot/include/bootstrapping/Bootstrapper.cuh`.
- FlyHE's boot example explicitly says its parameters were adjusted for A100 memory; copying those parameters to a smaller GPU may fail. Source: `RLWE/boot/examples/bootstrapping.cu`.

Budget checklist:

```text
1. N and active prime count per stage.
2. Peak number of live ciphertexts, including temporaries.
3. Relin key size and all Galois-key steps.
4. Bootstrap linear-transform plaintexts and modular-reduction temporaries.
5. Conversion keys if using LWE<->RLWE/CKKS.
6. Safety margin for cudaMallocAsync pool fragmentation and deep copies.
```

## Host-Device Transfers

GPU FHE workflow should look like this:

```text
host input vector
  -> H2D encode/encrypt
  -> all homomorphic operations on device
  -> decrypt/decode
  -> D2H result vector
```

Rules:

- Do not call decrypt/decode/print inside a performance path. FlyHE print helpers synchronize and move data to host. Source: `RLWE/phantom/include/ckks_evaluator.cuh`, `conversion/include/rlwe.cuh`.
- Avoid serializing keys/ciphertexts mid-pipeline; serialization copies device buffers to host. Source: `RLWE/phantom/include/ciphertext.h`, `conversion/include/rlwe.cuh`.
- If a CPU library is part of the same experiment, batch the transfer boundary: convert many ciphertexts or many slots at once, not one slot at a time. Source: `conversion/example/repack.cu`, `conversion/example/extract.cu`.
- Use pinned host buffers only after confirming the local code path benefits; this pass did not find a general pinned-buffer pipeline in FlyHE encode/decode [需人工确认 pinned-memory policy]. Source: `RLWE/phantom/src/batchencoder.cu`.

## Kernel Launch And Synchronization

GPU acceleration is strongest when each operation has enough parallel work. In FHE that usually means large `N`, many RNS primes, full slots, or many ciphertexts.

Rules:

- Add/sub are cheap but still launch kernels over `N * primes`. In small circuits, launch overhead can dominate. Source: `RLWE/phantom/src/evaluate.cu`.
- NTT/iNTT, mod-up/mod-down, key-switch inner products, and rotation are bandwidth-heavy. Group operations so data stays in cache/device memory where possible. Source: `RLWE/phantom/src/ntt/fntt_2d.cu`, `RLWE/phantom/src/eval_key_switch.cu`.
- Beware hidden barriers. FlyHE's CKKS rotation wrapper synchronizes after rotation, and `bootstrap_3` synchronizes at the end. Source: `RLWE/phantom/include/ckks_evaluator.cuh`, `RLWE/boot/src/bootstrapping/Bootstrapper.cu`.
- Use CUDA events for GPU timing. FlyHE includes `CUDATimer`, which records events on a stream. CPU wall-clock around async launches can undercount unless synchronized. Source: `RLWE/phantom/include/cuda_wrapper.cuh`.

## Streams And Parallel Batches

Useful stream-level pattern:

```text
stream 0: encode/encrypt batch A -> eval A -> decode A
stream 1: encode/encrypt batch B -> eval B -> decode B
shared read-only: context tables, public/eval keys
```

Rules:

- Use explicit stream wrappers when APIs expose them. Default-stream code is easier but may serialize unrelated work. Source: `RLWE/phantom/include/cuda_wrapper.cuh`, `RLWE/phantom/include/evaluate.cuh`.
- Do not share mutable ciphertext buffers across streams unless you own synchronization. `cuda_auto_ptr` remembers its stream and frees asynchronously on that stream. Source: `RLWE/phantom/include/cuda_wrapper.cuh`.
- Batch rotations from the same ciphertext with hoisting. `hoisting_inplace` reuses the expensive mod-up of `c1` across multiple rotation steps. Source: `RLWE/phantom/src/evaluate.cu`.
- Batch LWE/CKKS conversion. `repack.cu` repacks many LWE ciphertexts into one RLWE/CKKS ciphertext, amortizing keys and kernels. Source: `conversion/example/repack.cu`.

## GPU-Specific FHE Design Note Additions

Add these items to the normal CKKS design note:

1. GPU target: device model, CUDA architecture, CUDA version, compiler path, and whether `PHANTOM_USE_CUDA_PTX` is on.
2. Memory peak: estimate live ciphertexts, key sets, bootstrap plaintexts, and conversion keys.
3. Transfer plan: list every host-device boundary and justify it.
4. Stream plan: default stream or named streams; list expected synchronization points.
5. Batch plan: slot fill ratio, number of ciphertexts per batch, conversion group size.
6. NTT-state plan: which objects are NTT form, which custom steps need coefficient form, and where transforms happen.
7. Timing plan: CUDA events around kernels, no debug decrypt/print in timed section.

## Compared With CPU Libraries

| Workflow item | CPU OpenFHE/Lattigo focus | GPU FlyHE/Phantom extra work | Sources |
|---|---|---|---|
| Parameters | security, depth, scale, slots | also fit device memory and kernel occupancy | `RLWE/boot/examples/bootstrapping.cu` |
| Operation schedule | minimize depth/noise/key count | also minimize transfers, syncs, and tiny launches | `RLWE/phantom/src/evaluate.cu`; `RLWE/phantom/include/ckks_evaluator.cuh` |
| Rotations | count rotation keys and key-switch cost | also consider hoisting and synchronization after wrapper rotations | `RLWE/phantom/src/evaluate.cu`; `RLWE/phantom/include/ckks_evaluator.cuh` |
| Bootstrap | choose placement and post-boot levels | also budget LT plaintext memory and A100-like constraints | `RLWE/boot/include/bootstrapping/Bootstrapper.cuh`; `RLWE/boot/examples/bootstrapping.cu` |
| Debugging | decrypt/compare often | decrypt sparingly; each decode may synchronize/copy | `RLWE/phantom/src/batchencoder.cu` |
| Conversion | often out of scope | track CKKS/RLWE/LWE scales, extract indices, and reverse-bit layout | `conversion/example/extract.cu`; `conversion/example/repack.cu` |

## Gotchas

- Symptom: GPU memory allocation fails during bootstrap. Cause: too many levels, keys, and LT coefficients for the device. Fix: reduce `logN`, use sparse slots, reduce post-boot levels, reduce rotation set with BSGS, or move to a larger GPU. Source: `RLWE/boot/examples/bootstrapping.cu`.
- Symptom: timed code looks fast but output arrives late. Cause: CPU timer measured async launch, not completion. Fix: use CUDA events or synchronize around the timed region. Source: `RLWE/phantom/include/cuda_wrapper.cuh`.
- Symptom: many rotations scale poorly. Cause: each rotation performs automorphism plus key switching and the wrapper synchronizes. Fix: use hoisting for many rotations of one ciphertext and generate exact rotation keys. Source: `RLWE/phantom/src/evaluate.cu`, `RLWE/phantom/include/ckks_evaluator.cuh`.
- Symptom: operation throws `NTT form mismatch` or `CKKS encrypted must be in NTT form`. Cause: a custom path changed representation. Fix: explicitly transform back to the expected NTT state before evaluator calls. Source: `RLWE/phantom/src/evaluate.cu`.
- Symptom: conversion extracts wrong slots. Cause: fast-transform layout uses bit-reversed indices. Fix: reverse indices as in `extract.cu` and verify slot mapping with plaintext reference. Source: `conversion/example/extract.cu`.
- Symptom: scale error appears only after several operations. Cause: GPU code may not enforce all scale checks, especially ciphertext-ciphertext multiply. Fix: assert scale/chain after each multiply, rescale, mod-switch, and conversion. Source: `RLWE/phantom/src/evaluate.cu`.

## Minimal GPU Evaluation Skeleton

```cpp
// 1. Build context and keys once; keep them on GPU.
// 2. Encode/encrypt host batch once.
// 3. Evaluate on device; no print/decode in hot loop.
// 4. Use hoisting for many rotations of the same ciphertext.
// 5. Decode only final outputs; compare against plaintext reference.
```

Source: `FlyHE-main/FlyHE-main/RLWE/phantom/examples/3_ckks.cu`, `FlyHE-main/FlyHE-main/RLWE/boot/examples/bootstrapping.cu`, `FlyHE-main/FlyHE-main/conversion/example/*.cu`.
