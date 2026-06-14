# Library Comparison

Scope: OpenFHE 1.5.1 CPU (`openfhe.md`), Lattigo 6.2.0 CPU (`lattigo.md`), and FlyHE/Phantom GPU (`phantom-flyhe.md`).

Chinese summary: Use CPU libraries when correctness, maintainability, bootstrap maturity, and deployment portability dominate. Use FlyHE/Phantom when the workload is large enough to amortize GPU memory, transfers, keys, and synchronization, and when research-code risk is acceptable.

## Quick Decision Rules

| Decision | Choose | When | Main risks | Details |
|---|---|---|---|---|
| CPU vs GPU | CPU OpenFHE/Lattigo | Small/medium circuits, service deployment, CI-friendly builds, correctness-first prototypes, bootstrap-heavy development where mature APIs matter. | Lower peak throughput on large batch workloads. | [openfhe.md](./openfhe.md), [lattigo.md](./lattigo.md) |
| CPU vs GPU | GPU FlyHE/Phantom | Very large `N`, full-slot or many-ciphertext batches, throughput experiments, GPU already available, host-device transfers can be minimized. | Memory pressure, environment pinning, synchronization barriers, research-code instability, license/version uncertainty. | [phantom-flyhe.md](./phantom-flyhe.md), [gpu-considerations.md](../core/gpu-considerations.md) |
| CPU internal | OpenFHE | Need mature CKKS bootstrap, functional/interactive bootstrap examples, AUTO scale modes, C++ stack, broad FHE feature surface. | AUTO scale can hide level/scale transitions; naming differs from SEAL/Lattigo. | [openfhe.md](./openfhe.md) |
| CPU internal | Lattigo | Need Go integration, explicit scale/level control, server-side Go deployment, readable typed CKKS/RLWE APIs, circuit composition in Go. | Rescale is explicit; package path/version details matter; bootstrap security/parameters remain user responsibility. | [lattigo.md](./lattigo.md) |

## CPU vs GPU

### Choose CPU When

- **Data scale is modest**: a few ciphertexts, underfilled slots, or latency-sensitive single requests. GPU launch overhead and H2D/D2H transfer can dominate small workloads. Source: [gpu-considerations.md](../core/gpu-considerations.md).
- **Bootstrap maturity matters more than throughput**: OpenFHE and Lattigo both expose CKKS bootstrap APIs and documented examples. FlyHE has CKKS bootstrap in `RLWE/boot`, but its own Phantom core README says Phantom supports CKKS without bootstrapping, and FlyHE boot code is more experimental. Source: [openfhe.md](./openfhe.md), [lattigo.md](./lattigo.md), [phantom-flyhe.md](./phantom-flyhe.md).
- **Deployment environment is ordinary CPU infrastructure**: CI, containers, cloud services without fixed NVIDIA GPUs, or customer-side installs. OpenFHE and Lattigo avoid CUDA architecture, driver, and compiler pinning. Source: [openfhe.md](./openfhe.md), [lattigo.md](./lattigo.md).
- **You need safer iteration**: CPU libraries make it easier to print/decrypt during development; GPU debug printing synchronizes and moves data back to host. Source: [gpu-considerations.md](../core/gpu-considerations.md).
- **Security and parameter validation are central**: OpenFHE has built-in HE-standard-oriented parameter generation; Lattigo docs/examples expose `LogQP` checks but still require user review. FlyHE examples include speed/research choices and A100 memory constraints. Source: [openfhe.md](./openfhe.md), [lattigo.md](./lattigo.md), [phantom-flyhe.md](./phantom-flyhe.md).

### Choose GPU When

- **The workload is throughput-bound and batched**: full CKKS slots, many ciphertexts, many LWE ciphertexts repacked/extracted, or large `N * primes` arithmetic. Source: [phantom-flyhe.md](./phantom-flyhe.md), [gpu-considerations.md](../core/gpu-considerations.md).
- **The pipeline can stay on device**: encode/encrypt once, run many operations on GPU, decode once. If every step returns to CPU, do not expect GPU wins. Source: [gpu-considerations.md](../core/gpu-considerations.md).
- **NTT/key-switch cost dominates**: FlyHE/Phantom has CUDA 2D radix-8 NTT/iNTT, GPU key-switch inner products, and hoisting for many rotations of one ciphertext. Source: [phantom-flyhe.md](./phantom-flyhe.md).
- **Deployment has controlled NVIDIA hardware**: fixed CUDA version, architecture, compiler paths, enough VRAM for ciphertexts, keys, bootstrap LT plaintexts, and temporaries. Source: [phantom-flyhe.md](./phantom-flyhe.md), [gpu-considerations.md](../core/gpu-considerations.md).
- **Research code is acceptable**: FlyHE top README says it is research-oriented and under active development; local version/tag and license status need confirmation. Source: [phantom-flyhe.md](./phantom-flyhe.md).

### GPU Red Flags

- Underfilled slots or one-off requests.
- Frequent decrypt/decode/print between operations.
- Many rotations without hoisting or with wrapper synchronization in the hot loop.
- Bootstrap parameters chosen without VRAM budgeting.
- Need for reproducible deployment on machines without matching CUDA/GCC/nvcc.

Source: [phantom-flyhe.md](./phantom-flyhe.md), [gpu-considerations.md](../core/gpu-considerations.md).

## OpenFHE vs Lattigo

### Choose OpenFHE When

- You want **automatic scale/level management** in ordinary CKKS code. `FIXEDAUTO`, `FLEXIBLEAUTO`, and `FLEXIBLEAUTOEXT` can hide most explicit `Rescale` placement; use `FIXEDMANUAL` only for deliberate manual schedules. Source: [openfhe.md](./openfhe.md).
- You need **feature breadth** around CKKS: bootstrapping, functional bootstrapping, interactive bootstrapping, noise flooding decrypt, composite scaling examples, and many C++ examples. Source: [openfhe.md](./openfhe.md).
- You prefer **C++ integration** or already use C++/CMake in the target system. Source: [openfhe.md](./openfhe.md).
- You want **parameter generation with security-level hooks** instead of hand-maintaining every `LogQ/LogP` entry. Still validate special modulus `P` and bootstrap settings. Source: [openfhe.md](./openfhe.md).

Main traps:

- `ModReduce`/`Rescale` naming is OpenFHE-specific; it is not SEAL/Lattigo `mod_switch`.
- AUTO modes can make code look depth-free; still track level/scale in the design note.
- Public `ModReduceInPlace` only acts in `FIXEDMANUAL`; AUTO modes call internals.
- Exact rotation keys are still required.

Source: [openfhe.md](./openfhe.md).

### Choose Lattigo When

- You want **Go-native deployment**: services, libraries, testing, and concurrency patterns in Go. Source: [lattigo.md](./lattigo.md).
- You want **explicit CKKS control**: every `Rescale`, `DropLevel`, `Scale`, Galois element, and bootstrap parameter is visible. Source: [lattigo.md](./lattigo.md).
- You need **typed circuit composition** in Go: polynomial/minimax, linear transforms, bootstrapping packages under `circuits/ckks`. Source: [lattigo.md](./lattigo.md).
- You are willing to maintain **explicit parameter literals** and security checks for residual and bootstrapping parameter sets. Source: [lattigo.md](./lattigo.md).

Main traps:

- There is no OpenFHE-style AUTO rescale in the local Lattigo 6.2.0 CKKS path.
- Both ciphertext-ciphertext and nontrivial plaintext/scalar/vector multiplication can require explicit `Rescale`.
- Galois keys are generated from Galois elements, not raw step integers.
- This local tree uses `schemes/ckks`, not older/remembered `he/hefloat` paths.
- `eval.WithKey` shares temporary buffers with the source evaluator; do not use both concurrently.

Source: [lattigo.md](./lattigo.md).

## Feature Matrix

| Library | Language / bindings | Scale management | CKKS bootstrap | Typical strengths | Typical traps |
|---|---|---|---|---|---|
| OpenFHE 1.5.1 | C++ library; CMake build. Other bindings not covered in this local note [需人工确认 bindings]. | AUTO modes (`FIXEDAUTO`, `FLEXIBLEAUTO`, `FLEXIBLEAUTOEXT`) plus `FIXEDMANUAL`; normal `Eval*` can handle rescale internally in AUTO modes. | Yes: `EvalBootstrapSetup`, `EvalBootstrapKeyGen`, `EvalBootstrap`; functional and interactive examples exist. | Broad feature surface, mature examples, security-level parameter generation, CKKS bootstrap ecosystem, noise flooding decrypt support. | Naming hazards (`ModReduce` vs `LevelReduce`), AUTO mode hides transitions, bootstrap requires HYBRID and many keys, examples with `HEStd_NotSet` are not production parameters. |
| Lattigo 6.2.0 | Go modules; local CKKS path is `schemes/ckks`; circuits under `circuits/ckks`. | Explicit `rlwe.Scale`; explicit `Rescale`, `RescaleTo`, `DropLevel`, `SetScale`; no OpenFHE-style AUTO rescale in this path. | Yes: `circuits/ckks/bootstrapping` with `GenEvaluationKeys`, `NewEvaluator`, `Bootstrap`, `BootstrapMany`. | Go integration, explicit control, readable parameter literals, polynomial/linear-transform/bootstrap circuit packages, good for service-side Go code. | Must schedule rescale manually, add scale alignment is limited, Galois element generation can trip users, package path/version hazard, bootstrap security checks remain user responsibility. |
| FlyHE / Phantom GPU | C++/CUDA; Phantom-based RLWE core, cuTFHEpp LWE layer, conversion code. No semantic version found [需人工确认 version/tag]. | Manual; CKKS operations expose `scale`, `chain_index`, NTT state. Some scale checks are loose/commented out; user must assert schedule. | Yes in FlyHE `RLWE/boot`; Phantom core README itself says BGV/BFV/CKKS without bootstrapping. | GPU throughput for large batched workloads, CUDA 2D radix-8 NTT/iNTT, GPU key switching, hoisting rotations, CKKS/TFHE conversion experiments. | Research code, CUDA/GCC/nvcc environment pinning, VRAM pressure, host-device transfer/sync costs, rotation/bootstrap synchronization points, license/version uncertainty. |

Sources: [openfhe.md](./openfhe.md), [lattigo.md](./lattigo.md), [phantom-flyhe.md](./phantom-flyhe.md).

## Scenario Playbook

| Scenario | Default pick | Why | Watch |
|---|---|---|---|
| First CKKS prototype with bootstrap | OpenFHE | AUTO scaling and documented bootstrap examples reduce early errors. | Still write design note; do not copy insecure speed examples. |
| Go service needing CKKS inference without GPU | Lattigo | Native Go, explicit parameters, service-friendly packaging. | Explicit rescale/rotation key generation. |
| Deep CKKS circuit where bootstrap placement is central | OpenFHE or Lattigo | Both have CPU bootstrap; pick C++ vs Go and AUTO vs explicit style. | Separate residual vs bootstrap parameters and security budget. |
| Large batched linear algebra on fixed NVIDIA GPU | FlyHE/Phantom | GPU NTT/key-switch throughput and device-resident batches can win. | VRAM, transfers, stream barriers, exact rotation keys. |
| LWE/TFHE + CKKS conversion research | FlyHE | Repository includes cuTFHEpp and conversion extract/repack examples. | Experimental code; track scales and reverse-bit extract layout. |
| Production-facing regulated deployment | OpenFHE or Lattigo | More stable CPU deployment and easier audit path. | Noise flooding/threat model for decrypt outputs; security estimator/HE-standard validation. |

## Selection Checklist

Before choosing a library, answer:

1. Is the circuit bootstrap-free, leveled, or bootstrap-heavy?
2. Is the target runtime C++, Go, or CUDA-controlled research infrastructure?
3. Are inputs large enough to fill slots and amortize keys/transfers?
4. Can the whole evaluation stay on GPU, or will every step cross host-device boundaries?
5. Do you prefer OpenFHE AUTO scale management or Lattigo/FlyHE explicit scale schedules?
6. What exact rotation/Galois keys and bootstrap keys are required?
7. Is `log(Q*P)` security checked for the chosen `N`, including special primes and bootstrap parameters?

Cross references:

- OpenFHE details: [openfhe.md](./openfhe.md)
- Lattigo details: [lattigo.md](./lattigo.md)
- FlyHE/Phantom details: [phantom-flyhe.md](./phantom-flyhe.md)
- GPU workflow details: [gpu-considerations.md](../core/gpu-considerations.md)
