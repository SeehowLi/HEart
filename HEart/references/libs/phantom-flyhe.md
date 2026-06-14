# FlyHE / Phantom GPU Notes

Pinned source tree: `FlyHE-main/FlyHE-main`. Project name from the top-level README: **FlyHE**, a CUDA-based FHE library for GPU acceleration. No semantic release/version string was found in the local README or CMake files [需人工确认 version/tag].

Chinese summary: FlyHE wraps and extends Phantom's CUDA RLWE engine, adds CKKS bootstrapping and CKKS/TFHE conversion experiments, and keeps most expensive RLWE data resident on the GPU. Treat it as research code: plan memory, keys, streams, and level/scale state explicitly.

## Repository Map

- Top-level project and build: `README.md`, `CMakeLists.txt`.
  Source: `FlyHE-main/FlyHE-main/README.md`, `FlyHE-main/FlyHE-main/CMakeLists.txt`.
- Phantom-based RLWE core: `RLWE/phantom/`.
  - Public includes: `include/phantom.h`, `include/evaluate.cuh`, `include/ckks.h`, `include/ckks_evaluator.cuh`.
  - CUDA kernels and arithmetic: `src/evaluate.cu`, `src/eval_key_switch.cu`, `src/ntt/*.cu`, `src/util/*`.
  - Examples: `examples/3_ckks.cu`, `examples/4_kernel_fusing.cu`, `examples/example.cu`.
  Source: `RLWE/phantom/README.md`, `RLWE/phantom/include/*`, `RLWE/phantom/src/*`, `RLWE/phantom/examples/*`.
- FlyHE CKKS bootstrapping layer: `RLWE/boot/`.
  - Main API class: `include/bootstrapping/Bootstrapper.cuh`.
  - Implementation: `src/bootstrapping/Bootstrapper.cu`, `src/bootstrapping/ModularReducer.cu`.
  - Examples: `examples/bootstrapping.cu`, `examples/nn.cu`, `examples/test_*.cu`.
  Source: `RLWE/boot/include/bootstrapping/Bootstrapper.cuh`, `RLWE/boot/src/bootstrapping/Bootstrapper.cu`, `RLWE/boot/examples/CMakeLists.txt`.
- TFHE / LWE GPU layer: `LWE/cuTFHEpp/` plus `LWE/examples/`.
  Source: `README.md`, `LWE/CMakeLists.txt`, `LWE/cuTFHEpp/src/cutfhe++.cu`.
- Scheme conversion: `conversion/`.
  - CKKS/RLWE to LWE extraction: `include/extract.cuh`, `src/extract.cu`, `example/extract.cu`.
  - LWE to CKKS/RLWE repack: `include/repack.h`, `src/repack.cu`, `example/repack.cu`.
  - Conversion driver: `include/conversion.cuh`, `src/conversion.cu`.
  Source: `conversion/CMakeLists.txt`, `conversion/src/CMakeLists.txt`, `conversion/example/*.cu`.

## What It Is Based On

| Component | What FlyHE says | Operational consequence | Sources |
|---|---|---|---|
| Phantom | Modified code from Phantom. Phantom README calls itself "A CUDA-Accelerated Fully Homomorphic Encryption Library" and cites "Phantom: A CUDA-Accelerated Word-Wise Homomorphic Encryption Library" (IEEE TDSC 2024 / ePrint 2023/049). | Use Phantom-style APIs and SEAL-like names for BFV/BGV/CKKS core operations. Phantom core says CKKS is supported **without bootstrapping**. | `README.md`; `RLWE/phantom/README.md` |
| Opera | Top-level README says some files contain modified code from Opera. Exact file-to-feature mapping was not fully traced [需人工确认 Opera mapping]. | Treat boot/conversion code provenance carefully when reusing license or algorithm claims. | `README.md` |
| TFHEpp | Top-level README says some files contain modified code from TFHEpp. | LWE/SISD-FHE path uses TFHEpp/cuTFHEpp APIs and parameters; do not assume CKKS semantics there. | `README.md`; `LWE/cuTFHEpp/src/cutfhe++.cu` |
| HE3DB | Several `conversion/src/*.cu` files say they were modified based on HE3DB. | Conversion helpers are research/experiment code; verify every scale and key before transplanting. | `conversion/src/utils.cu`; `conversion/src/repack.cu`; `conversion/src/polyeval_bsgs.cu`; `conversion/src/conversion.cu` |

## Referenced Papers And Technical Buckets

- CKKS core: "Homomorphic Encryption for Arithmetic of Approximate Numbers" is cited by the top-level FlyHE README. Use it for approximate-number semantics, scale growth, and rescale reasoning. Source: `README.md`.
- TFHE: "TFHE: Fast Fully Homomorphic Encryption over the Torus" is cited by the top-level FlyHE README. Use it for SISD/LWE bootstrapping context, not CKKS slot semantics. Source: `README.md`.
- PEGASUS: "PEGASUS: Bridging Polynomial and Non-Polynomial Evaluations in Homomorphic Encryption" is cited by the top-level README and is relevant to the conversion directory's CKKS/TFHE bridge. Source: `README.md`, `conversion/include/conversion.cuh`.
- Phantom GPU library: cited in Phantom README as IEEE TDSC 2024 / ePrint 2023/049. Use it as the primary GPU/NTT/library-design source for Phantom-based operations. Source: `RLWE/phantom/README.md`.
- BFV GPU variants: Phantom README cites ePrint 2023/1429 for BFV GPU framework design. It is BFV-specific; use it only as a secondary GPU implementation source when CKKS code shares infrastructure. Source: `RLWE/phantom/README.md`.
- NTT implementation: local code uses CUDA 2D radix-8 NWT/NTT kernels with Shoup twiddle tables and Harvey-like butterfly naming; this pass found implementation evidence but no standalone NTT paper citation in the FlyHE README [需人工确认 exact NTT paper-to-code mapping]. Source: `RLWE/phantom/src/ntt/fntt_2d.cu`, `RLWE/phantom/src/ntt/intt_2d.cu`, `RLWE/phantom/include/butterfly.cuh`.

## Setup

Build from `FlyHE-main/FlyHE-main`:

```bash
mkdir build
cd build
cmake ..
make -j
```

Version-sensitive setup facts:

- Top-level CMake pins `/usr/bin/gcc-13`, `/usr/bin/g++-13`, and `/usr/local/cuda-12.4/bin/nvcc`; this is local-environment-specific and likely needs editing on another machine. Source: `CMakeLists.txt`.
- Top-level CMake sets `CMAKE_CUDA_ARCHITECTURES native` and enables `PHANTOM_USE_CUDA_PTX` by default. Source: `CMakeLists.txt`.
- Phantom README lists prerequisites `CUDA >= 11.0`, `CMake >= 3.20`, `GCC >= 11.0`, and examples such as A100 `80`, V100 `70`, P100 `60` for CUDA architecture selection. Source: `RLWE/phantom/README.md`.
- Top-level build adds `RLWE`, `LWE`, `conversion`, and `examples`; `RLWE` adds both `phantom` and `boot`. Source: `CMakeLists.txt`, `RLWE/CMakeLists.txt`.

## API Mapping To Core 02

| Core primitive | Phantom/FlyHE API | Key requirement | GPU/scale/level note | Sources |
|---|---|---|---|---|
| Add | `add`, `add_inplace`; wrapper `rlwe::Evaluator::add*` | none | Requires matching parameter/chain state; `add_many` sums vector of ciphertexts. GPU kernel does RNS polynomial add. | `RLWE/phantom/include/evaluate.cuh`; `RLWE/phantom/src/evaluate.cu`; `RLWE/phantom/include/ckks_evaluator.cuh` |
| AddPlain | `add_plain`, `add_plain_inplace`; wrapper `add_const_inplace` for CKKS scalar | none | CKKS ciphertext must be in NTT form; plaintext scale and chain index must match for normal `add_plain`. | `RLWE/phantom/src/evaluate.cu`; `RLWE/phantom/include/ckks_evaluator.cuh` |
| MulConst / MulPlain | `multiply_plain(_inplace)`, `multiply_const_inplace`, wrapper `multiply_vector_inplace_reduced_error` | none | CKKS multiply-plain multiplies stored scale. Normal path expects aligned chain index; wrapper encodes constants at `ct.scale()` and mod-switches plaintext. | `RLWE/phantom/src/evaluate.cu`; `RLWE/phantom/include/ckks_evaluator.cuh` |
| Mul | `multiply`, `multiply_inplace`, `multiply_and_relin_inplace`, wrapper `Evaluator::multiply*` | relin key needed if the result will be used as degree-2-free ciphertext | Requires matching chain index, NTT form, and ciphertext size. CKKS/BGV multiplication creates size 3 unless fused with relin. | `RLWE/phantom/src/evaluate.cu`; `RLWE/phantom/examples/3_ckks.cu` |
| Relinearize | `relinearize_inplace` | `PhantomRelinKey`, generated by `secret_key.gen_relinkey(context)` | Requires size-3 ciphertext and CKKS NTT form; calls GPU key switching and shrinks result back to size 2. | `RLWE/phantom/src/evaluate.cu`; `RLWE/phantom/src/eval_key_switch.cu` |
| Rescale | `rescale_to_next_inplace` / `rescale_to_next` | none | CKKS-only public rescale calls `mod_switch_scale_to_next`: divides by the dropped last prime and advances the chain. | `RLWE/phantom/include/evaluate.cuh`; `RLWE/phantom/src/evaluate.cu` |
| Rotate | `rotate_vector`, `rotate_vector_inplace`, `rotate_inplace`, wrapper `many_rotate`/`hoisting_inplace` | Galois key for step or NAF decomposition | Rotation applies automorphism then key switching. Wrapper currently synchronizes after rotation because code comments say rotation is unstable. | `RLWE/phantom/src/evaluate.cu`; `RLWE/phantom/include/ckks_evaluator.cuh` |
| Conjugate | `complex_conjugate`, `complex_conjugate_inplace` | Galois key for conjugation element | Implemented as Galois element from step `0`; same key-switch cost class as rotation. | `RLWE/phantom/src/evaluate.cu`; `RLWE/phantom/include/ckks_evaluator.cuh` |
| ModSwitch / DropLevel | `mod_switch_to_next_inplace`, `mod_switch_to_inplace` | none | For CKKS, `mod_switch_to_next` drops modulus without scale division; use `rescale_to_next` when scale must be divided. | `RLWE/phantom/include/evaluate.cuh`; `RLWE/phantom/src/evaluate.cu` |
| Bootstrap | FlyHE `Bootstrapper::bootstrap*`, especially `bootstrap_3` / `bootstrap_slim_3` | relin key plus large Galois-key set for linear transforms | Phantom README says Phantom core has no bootstrapping; FlyHE `RLWE/boot` adds CKKS bootstrapping with ModRaise, CoeffToSlot, modular reduction, SlotToCoeff variants. | `RLWE/phantom/README.md`; `RLWE/boot/include/bootstrapping/Bootstrapper.cuh`; `RLWE/boot/src/bootstrapping/Bootstrapper.cu` |

## Keys

- Public/secret: `PhantomSecretKey secret_key(context)` and `secret_key.gen_publickey(context)`. Source: `RLWE/phantom/examples/3_ckks.cu`.
- Relinearization: `PhantomRelinKey relin_keys = secret_key.gen_relinkey(context)`. Required before `relinearize_inplace` and practical ciphertext-ciphertext multiply workflows. Source: `RLWE/phantom/examples/3_ckks.cu`, `RLWE/phantom/src/evaluate.cu`.
- Rotation/conjugation: `secret_key.create_galois_keys(context)` for broad examples, or `create_galois_keys_from_steps(steps, galois_keys)` via the FlyHE wrapper for exact steps. Source: `RLWE/phantom/examples/3_ckks.cu`, `RLWE/phantom/include/ckks_evaluator.cuh`.
- Bootstrapping rotations: `Bootstrapper::addLeftRotKeys_Linear_to_vector_3` appends BSGS/linear-transform steps, then `create_galois_keys_from_steps`. Key size is printed in the example. Source: `RLWE/boot/examples/bootstrapping.cu`, `RLWE/boot/src/bootstrapping/Bootstrapper.cu`.
- Conversion keys: extraction uses `GenExtractKey`; repack uses LWE2RLWE prekeys/Galois keys. Treat these as conversion-specific, not CKKS EvalMult keys. Source: `conversion/example/extract.cu`, `conversion/example/repack.cu`, `conversion/include/rlwe.cuh`.

## GPU Implementation Strategies

### Device residency and memory

- `PhantomCiphertext` owns device memory through `phantom::util::cuda_auto_ptr<uint64_t> data_`. Resize allocates with `cudaMallocAsync`, preserves overlapping old contents with `cudaMemcpyAsync`, and uses the ciphertext stream. Source: `RLWE/phantom/include/ciphertext.h`, `RLWE/phantom/include/cuda_wrapper.cuh`.
- `cuda_auto_ptr` copy construction and assignment are deep device-to-device async copies. Passing ciphertexts by value or assigning large vectors can allocate and copy many `size * coeff_modulus_size * N` words. Source: `RLWE/phantom/include/cuda_wrapper.cuh`, `RLWE/phantom/include/ciphertext.h`.
- Context, RNS tables, NTT tables, keys, plaintexts, and ciphertexts are GPU-backed. CPU-host vectors appear at encode/decode/encrypt input/output boundaries, not in the inner evaluator path. Source: `RLWE/phantom/src/batchencoder.cu`, `RLWE/phantom/src/evaluate.cu`.

### NTT / iNTT

- NTT kernels are 2D radix-8 CUDA kernels. Forward uses CT-style butterflies; inverse uses GS-style butterflies, shared memory buffers, Shoup twiddles, and per-RNS-prime loops. Source: `RLWE/phantom/src/ntt/fntt_2d.cu`, `RLWE/phantom/src/ntt/intt_2d.cu`, `RLWE/phantom/include/butterfly.cuh`.
- CKKS ciphertexts are expected to stay in NTT form for add-plain, multiply-plain, relin, rotation, and conjugation. Some helpers expose `transform_to_ntt_inplace` and `transform_from_ntt_inplace`; use them only when an algorithm explicitly needs coefficient form. Source: `RLWE/phantom/src/evaluate.cu`, `RLWE/phantom/include/ckks_evaluator.cuh`.
- Conversion/extraction may require non-NTT RLWE data and bit-reversed indices; `extract.cuh` throws if a sampled RLWE is still in NTT form, and `extract.cu` example reverses indices because of fast-transform layout. Source: `conversion/include/extract.cuh`, `conversion/example/extract.cu`.

### Key switching

- Relinearization and rotation both call `keyswitch_inplace`. The runtime pattern is: choose the active `Ql/P` bases, `modup` the switching component, run GPU inner products against evaluation-key arrays, mod down from NTT, and add the result into `(c0,c1)`. Source: `RLWE/phantom/src/eval_key_switch.cu`, `RLWE/phantom/src/evaluate.cu`.
- The inner product kernel `key_switch_inner_prod_c2_and_evk` parallelizes over RNS towers and coefficients, accumulates 128-bit products, and performs Barrett reductions under each modulus. Source: `RLWE/phantom/src/eval_key_switch.cu`.
- Hoisting optimization: `hoisting_inplace` extracts/mod-ups `c1` once, applies multiple automorphisms, runs per-rotation key-switch inner products, accumulates outputs, then mod-downs. Use it for many rotations of the same ciphertext; do not use it for unrelated ciphertexts. Source: `RLWE/phantom/src/evaluate.cu`, `RLWE/phantom/include/ckks_evaluator.cuh`.

### Host-device transfers

- Encoding copies host vectors to device with `cudaMemcpyAsync(..., cudaMemcpyHostToDevice, stream)`, then runs GPU encode/NTT kernels. Decoding copies back with `cudaMemcpyAsync(..., cudaMemcpyDeviceToHost, stream)` and synchronizes. Source: `RLWE/phantom/src/batchencoder.cu`, `RLWE/phantom/src/ckks.cu`.
- Debug print helpers decrypt and decode, causing device-host synchronization and transfer; keep them out of timed GPU pipelines. Source: `RLWE/phantom/include/ckks_evaluator.cuh`, `conversion/include/rlwe.cuh`.
- Serialization and some conversion paths explicitly copy secret/key/ciphertext material to host; do not use them in hot loops. Source: `RLWE/phantom/include/ciphertext.h`, `conversion/include/rlwe.cuh`.

### Streams, synchronization, batching

- The default stream is a non-blocking `cuda_stream_wrapper`; most APIs accept a stream wrapper and launch async kernels on that stream. Source: `RLWE/phantom/include/cuda_wrapper.cuh`, `RLWE/phantom/include/util/globals.h`.
- Rotation wrapper in `ckks_evaluator.cuh` calls `cudaStreamSynchronize(ct.data_ptr().get_stream())` and comments that it is currently required because rotation is unstable. This can dominate pipelines with many rotations. Source: `RLWE/phantom/include/ckks_evaluator.cuh`.
- `Bootstrapper::bootstrap_3` also synchronizes at the end. Treat bootstrap as a stream barrier unless you rewrite/verify that path. Source: `RLWE/boot/src/bootstrapping/Bootstrapper.cu`.
- Batch parallelism mostly comes from full CKKS slots and from kernels over RNS towers/coefficient arrays. Conversion examples also batch many LWE ciphertexts into one RLWE/CKKS ciphertext (`32768` in `repack.cu`). Source: `RLWE/phantom/examples/3_ckks.cu`, `conversion/example/repack.cu`.

## Supported Capabilities

| Area | Supported in local tree | Notes | Sources |
|---|---|---|---|
| CKKS SIMD-FHE | yes | Top README says SIMD-FHE: CKKS. Phantom examples cover encode/decode, encrypt/decrypt, add, multiply-plain, multiply+relin, rotate, scalar helpers. | `README.md`; `RLWE/phantom/examples/3_ckks.cu` |
| BFV/BGV | yes in Phantom core | Phantom README says BGV, BFV, CKKS. Top README focuses current FlyHE support on CKKS and TFHE; BFV/BGV examples are still present. | `RLWE/phantom/README.md`; `RLWE/phantom/examples/1_bfv.cu`; `RLWE/phantom/examples/2_bgv.cu` |
| TFHE / LWE | yes | Top README says SISD-FHE: TFHE; cuTFHEpp exposes homomorphic add/sub/not/shift, key switching, gate/prog bootstrapping entries. | `README.md`; `LWE/cuTFHEpp/src/cutfhe++.cu` |
| CKKS bootstrap | yes in FlyHE `RLWE/boot`, not in Phantom core | Boot example uses A100-constrained parameters, generates minimax polynomials and linear-transform coefficients, then calls `bootstrap_3`. | `RLWE/phantom/README.md`; `RLWE/boot/examples/bootstrapping.cu` |
| Scheme conversion | yes, experimental | README says scheme conversion; code has LWE extraction and LWE-to-RLWE repack. Some behavior is inferred from code, not README prose. | `README.md`; `conversion/include/conversion.cuh`; `conversion/example/*.cu` |
| Nonlinear helpers | present | `CKKSEvaluator` exposes `sgn_eval`, `invert_sqrt`, `exp`, `inverse`; boot layer has GELU, softmax, layer norm, argmax sources. Verify per function before using. | `RLWE/phantom/include/ckks_evaluator.cuh`; `RLWE/boot/src/*.cu` |

## FlyHE Gotchas

- Research code, not production: top README says it is research-oriented and under active development. Rule: do not hide FlyHE behind a production API without independent tests and parameter review. Source: `README.md`.
- Build is environment-pinned: top CMake hardcodes GCC/G++ 13 and CUDA 12.4 paths. Rule: record your build environment in any experiment. Source: `CMakeLists.txt`.
- Phantom README license conflict risk: top README says modified Phantom code is MIT, while local Phantom README says Phantom is GPLv3. This needs legal/source-history confirmation before redistribution [需人工确认 license status]. Source: `README.md`, `RLWE/phantom/README.md`.
- CKKS scale checks are partly loose: ciphertext-ciphertext multiply has scale checks commented out, while add-plain checks scale. Rule: track scale manually and rescale/mod-switch according to the design note. Source: `RLWE/phantom/src/evaluate.cu`.
- CKKS `mod_switch_to_next` is not `rescale_to_next`: CKKS mod-switch drops a prime without scale division, while rescale divides by the dropped prime. Rule: map operations to `02-op-semantics.md`, not by name alone. Source: `RLWE/phantom/src/evaluate.cu`.
- CKKS ciphertexts normally must be NTT-form. Rule: after any custom coefficient-domain operation, restore expected `is_ntt_form` before calling evaluator operations. Source: `RLWE/phantom/src/evaluate.cu`.
- Rotation may synchronize. Rule: count each rotation as expensive and potentially a pipeline barrier in this wrapper. Source: `RLWE/phantom/include/ckks_evaluator.cuh`.
- Galois key coverage is exact/NAF-dependent. Rule: generate all rotations in the layout and bootstrap plan; missing keys throw `Galois key not present`. Source: `RLWE/phantom/src/evaluate.cu`, `RLWE/boot/src/bootstrapping/Bootstrapper.cu`.
- Bootstrapping memory is a first-class parameter. The example explicitly reduces `logN`/slots because parameters were adjusted to fit A100 memory. Rule: estimate ciphertext/key/LT-coefficient memory before choosing `logN`, levels, and sparse slots. Source: `RLWE/boot/examples/bootstrapping.cu`.
- Sparse slots affect boot/conversion behavior. Rule: always write `slot_count`, `sparse_slots`, and replication pattern in the design note. Source: `RLWE/boot/examples/bootstrapping.cu`, `conversion/example/extract.cu`.
- Debug helpers synchronize and decrypt. Rule: use them only for correctness checkpoints, not performance claims. Source: `RLWE/phantom/include/ckks_evaluator.cuh`, `conversion/include/rlwe.cuh`.

## Example Walkthroughs

### CKKS core: `RLWE/phantom/examples/3_ckks.cu`

1. Function: demonstrate CKKS encode/decode, symmetric/asymmetric encryption, add, multiply-plain, multiply+relin, rotation, scalar helpers, and mod raising experiments.
2. Circuit: add depth 0; multiply-plain increases scale and normally requires rescale later; ciphertext multiply creates size-3 then `relinearize_inplace`; rotations use key switching.
3. Packing: full `encoder.slot_count()` complex or double slots. Comments warn to encode the longest message first because sparse message encoding may be inferred from the first encoding.
4. Parameters: main example uses `N = 1 << 15`, scale `2^40`, a chain like `{60, many 40s, 60}` for alpha 1, with other `special_modulus_size` variants.
5. Keys: public key, relin key, and broad Galois key set.
6. Implementation: calls `multiply`, `relinearize_inplace`, then in deeper examples `rescale_to_next_inplace`; manual `mod_switch_to_next_inplace` is used for benchmarking across levels.
7. Verification: examples print/decrypt selected slots; many correctness checks are commented out in performance loops. Add explicit plaintext-reference max error for new code.
Source: `RLWE/phantom/examples/3_ckks.cu`.

### Kernel fusing / BFV-BGV performance: `RLWE/phantom/examples/4_kernel_fusing.cu`

- This example is not CKKS-focused; it benchmarks BFV/BGV parameter sets and multiply/relin behavior across large `N`.
- Engineering use: read it for GPU memory warnings and key memory comments. It says Galois keys can use a lot of memory and take long to generate.
- Do not copy BFV/BGV modulus choices into CKKS.
Source: `RLWE/phantom/examples/4_kernel_fusing.cu`.

### CKKS bootstrap: `RLWE/boot/examples/bootstrapping.cu`

1. Function: refresh a sparse CKKS ciphertext.
2. Circuit: subsum if sparse, ModRaise, CoeffToSlot, modular reduction by minimax polynomial, SlotToCoeff.
3. Packing: `logN = 14`, `logn = 8`, `sparse_slots = 2^logn`; input repeats sparse values across full slot count.
4. Parameters: example uses `logp = 46`, `logq = 51`, special prime `51`, `remaining_level = 16`, `boot_level = 14`, and hamming weight `192`; comment says parameters were adjusted for A100 memory.
5. Keys: relin key plus generated Galois steps: step `0`, powers of two, and BSGS linear-transform rotations from `addLeftRotKeys_Linear_to_vector_3`.
6. Implementation: `prepare_mod_polynomial`, generate boot keys, `generate_LT_coefficient_3`, switch input down through `total_level`, then call `bootstrap_3`.
7. Verification: decrypt before/after and report mean absolute error over sparse slots.
Source: `RLWE/boot/examples/bootstrapping.cu`.

### Conversion extract/repack: `conversion/example/*.cu`

- `extract.cu`: encrypts CKKS/RLWE slots, generates an extract key, reverse-bits extract indices because of fast-transform layout, and extracts selected slots into GPU LWE ciphertexts.
- `repack.cu`: creates many TFHE LWE ciphertexts, initializes Phantom RLWE, generates LWE2RLWE Galois keys, calls `conver::repack`, and prints the resulting CKKS ciphertext level/chain.
- Rule: conversion changes representation and often scale; track LWE scale, CKKS scale, extract indices, reverse-bit layout, and generated conversion keys separately from normal CKKS keys.
Source: `conversion/example/extract.cu`, `conversion/example/repack.cu`, `conversion/include/conversion.cuh`.

## Quick Checklist

- Pin GPU architecture, CUDA version, compiler paths, and `PHANTOM_USE_CUDA_PTX`.
- Write memory budget: ciphertext words = `components * coeff_modulus_size * N * 8`, then add relin/Galois/bootstrap keys and LT plaintexts.
- Keep ciphertexts on device; avoid decode/print/serialize inside timed sections.
- Generate exact relin/Galois/bootstrap/conversion keys before evaluation.
- Assert `chain_index`, `scale`, `is_ntt_form`, `size`, and `sparse_slots` at every custom boundary.
- Prefer hoisting for many rotations of the same ciphertext.
- Treat CKKS bootstrap and rotation wrappers as synchronization points unless verified otherwise.
