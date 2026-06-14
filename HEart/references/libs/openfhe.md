# OpenFHE CKKS RNS Notes

Pinned version: OpenFHE 1.5.1, local source tree `openfhe-development-1.5.1/openfhe-development-1.5.1`.

中文摘要: This file maps the core CKKS mental model and operation semantics to OpenFHE 1.5.1. The main engineering rule is: use `FIXEDMANUAL` only when you deliberately want to place `Rescale/ModReduce`; use `FIXEDAUTO`, `FLEXIBLEAUTO`, or `FLEXIBLEAUTOEXT` when you want OpenFHE to handle scale/level adjustment in normal `Eval*` calls.

## Repository Map

- CKKS scheme implementation: `src/pke/lib/scheme/ckksrns/`
  - `ckksrns-leveledshe.cpp`: CKKS-specific scalar/plaintext multiply, `ModReduceInternalInPlace`, `LevelReduceInternalInPlace`, scale/level adjustment.
  - `ckksrns-fhe.cpp`: CKKS bootstrapping, functional bootstrapping, CoeffsToSlots/SlotsToCoeffs linear transforms, bootstrap rotation-index discovery.
  - `ckksrns-pke.cpp`: CKKS decryption and noise-flooding decrypt path.
  - `ckksrns-parametergeneration.cpp`: modulus chain, `FLEXIBLEAUTO*`, `COMPOSITESCALING*`, security-bound checks.
  - `ckksrns-cryptoparameters.cpp`: precomputed CRT tables and per-level scaling factors.
- Generic RNS/base implementation:
  - `src/pke/lib/schemerns/rns-leveledshe.cpp`: RNS `EvalAdd`, `EvalMult`, `ModReduce`, `LevelReduce`, `AdjustForAddOrSub`, `AdjustForMult`.
  - `src/pke/lib/schemebase/base-leveledshe.cpp`: relinearization, generic ciphertext-ciphertext multiplication shape, automorphism key generation.
  - `src/pke/lib/keyswitch/keyswitch-hybrid.cpp` and `keyswitch-bv.cpp`: HYBRID/BV key-switch internals.
  - `src/pke/include/cryptocontext.h`: public API surface.
- Main CKKS examples live in `src/pke/examples/`.

CKKS-related example/docs files in `src/pke/examples/`:

`advanced-ckks-bootstrapping.cpp`, `advanced-real-numbers.cpp`, `advanced-real-numbers-128.cpp`, `CKKS_BOOTSTRAPPING.md`, `CKKS_FUNCTIONAL_BOOTSTRAPING.md`, `CKKS_NOISE_FLOODING.md`, `ckks-noise-flooding.cpp`, `COMPOSITE_SCALING.md`, `FUNCTION_EVALUATION.md`, `functional-bootstrapping-ckks.cpp`, `function-evaluation.cpp`, `inner-product.cpp`, `INTERACTIVE_BOOTSTRAPPING.md`, `interactive-bootstrapping.cpp`, `iterative-ckks-bootstrapping.cpp`, `iterative-ckks-bootstrapping-composite-scaling.cpp`, `linearwsum-evaluation.cpp`, `polynomial-evaluation.cpp`, `polynomial-evaluation-high-precision-composite-scaling.cpp`, `rotation.cpp`, `scheme-switching.cpp`, `scheme-switching-serial.cpp`, `simple-ckks-bootstrapping.cpp`, `simple-ckks-bootstrapping-composite-scaling.cpp`, `simple-complex-numbers.cpp`, `simple-composite-scaling-manual.cpp`, `simple-real-numbers.cpp`, `simple-real-numbers-composite-scaling.cpp`, `simple-real-numbers-serial.cpp`, `tckks-interactive-mp-bootstrapping.cpp`, `tckks-interactive-mp-bootstrapping-Chebyshev.cpp`.

Sources: `src/pke/lib/scheme/ckksrns/*`, `src/pke/lib/schemerns/rns-leveledshe.cpp`, `src/pke/lib/schemebase/base-leveledshe.cpp`, `src/pke/lib/keyswitch/*`, `src/pke/examples/README.md`.

## Setup Boilerplate

Minimal CKKS setup:

```cpp
#include "openfhe.h"
using namespace lbcrypto;

uint32_t multDepth = 3;
uint32_t scaleModSize = 50;
uint32_t batchSize = 8;

CCParams<CryptoContextCKKSRNS> parameters;
parameters.SetMultiplicativeDepth(multDepth);
parameters.SetScalingModSize(scaleModSize);
parameters.SetBatchSize(batchSize);
// Optional but usually explicit in serious code:
// parameters.SetScalingTechnique(FLEXIBLEAUTOEXT);
// parameters.SetKeySwitchTechnique(HYBRID);
// parameters.SetSecurityLevel(HEStd_128_classic);
// parameters.SetFirstModSize(60);

CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
cc->Enable(PKE);
cc->Enable(KEYSWITCH);
cc->Enable(LEVELEDSHE);
// Add ADVANCEDSHE for EvalPoly/EvalChebyshev/EvalSum-like advanced APIs.
// Add FHE for EvalBootstrap.

auto keys = cc->KeyGen();
cc->EvalMultKeyGen(keys.secretKey);              // relinearization
cc->EvalRotateKeyGen(keys.secretKey, {1, -2});   // exact rotation set

std::vector<double> x = {0.25, 0.5, 0.75, 1.0};
Plaintext ptxt = cc->MakeCKKSPackedPlaintext(x);
auto ct = cc->Encrypt(keys.publicKey, ptxt);
```

Bootstrap setup pattern:

```cpp
CCParams<CryptoContextCKKSRNS> parameters;
parameters.SetSecretKeyDist(UNIFORM_TERNARY);
parameters.SetScalingTechnique(FLEXIBLEAUTO); // 64-bit examples; 128-bit uses FIXEDAUTO.
parameters.SetKeySwitchTechnique(HYBRID);

std::vector<uint32_t> levelBudget = {4, 4};
uint32_t postBootLevels = 10;
uint32_t depth = postBootLevels + FHECKKSRNS::GetBootstrapDepth(levelBudget, UNIFORM_TERNARY);
parameters.SetMultiplicativeDepth(depth);

auto cc = GenCryptoContext(parameters);
cc->Enable(PKE);
cc->Enable(KEYSWITCH);
cc->Enable(LEVELEDSHE);
cc->Enable(ADVANCEDSHE);
cc->Enable(FHE);

uint32_t numSlots = cc->GetRingDimension() / 2;
cc->EvalBootstrapSetup(levelBudget, {0, 0}, numSlots);
auto keys = cc->KeyGen();
cc->EvalMultKeyGen(keys.secretKey);
cc->EvalBootstrapKeyGen(keys.secretKey, numSlots);
```

Sources: `src/pke/examples/simple-real-numbers.cpp`, `src/pke/examples/simple-ckks-bootstrapping.cpp`, `src/pke/include/cryptocontext.h`.

## API Mapping From Core Operation Semantics

| Core primitive | OpenFHE 1.5.1 API | Key requirement | Scale/level note | Sources |
|---|---|---|---|---|
| Add | `cc->EvalAdd(ct1, ct2)`, `EvalAddInPlace` | none | RNS path aligns levels/scales before add unless `NORESCALE`; additions do not spend multiplicative depth. | `src/pke/lib/schemerns/rns-leveledshe.cpp` |
| AddPlain | `cc->EvalAdd(ct, plaintext)`, `cc->EvalAdd(ct, double)`, complex overloads | none | `MorphPlaintext` adapts plaintext to ciphertext level/format; wrong plaintext scale is still a design bug. | `src/pke/lib/schemerns/rns-leveledshe.cpp`, `src/pke/include/cryptocontext.h` |
| MulConst / MulPlain | `cc->EvalMult(ct, double)`, `cc->EvalMult(ct, std::complex<double>)`, `cc->EvalMult(ct, plaintext)` | none | CKKS scale grows. In AUTO modes OpenFHE manages later reduction; in `FIXEDMANUAL`, call `cc->Rescale`/`cc->ModReduce` when your schedule requires it. | `src/pke/lib/scheme/ckksrns/ckksrns-leveledshe.cpp`, `src/pke/lib/schemerns/rns-leveledshe.cpp` |
| Mul | `cc->EvalMult(ct1, ct2)`, `cc->EvalSquare(ct)` | `EvalMultKeyGen` | Public `EvalMult` uses the stored relinearization key and returns a 2-component ciphertext. `EvalMultNoRelin` exists for lazy patterns. | `src/pke/include/cryptocontext.h`, `src/pke/lib/schemebase/base-leveledshe.cpp` |
| Relinearize | `cc->Relinearize(ct)`, `cc->RelinearizeInPlace(ct)` | `EvalMultKeyGen`; more keys if `maxRelinSkDeg > 2` | Key-switches components `c2, c3, ...` back into `(c0,c1)`. | `src/pke/lib/schemebase/base-leveledshe.cpp`, `src/pke/include/cryptocontext.h` |
| Rescale / ModReduce | `cc->Rescale(ct)`, `cc->RescaleInPlace(ct)`, `cc->ModReduce(ct)`, `cc->ModReduceInPlace(ct)` | none | OpenFHE `Rescale` is an alias to the ModReduce path. Public `ModReduceInPlace` performs only in `FIXEDMANUAL`; AUTO paths call internal reduction as needed. | `src/pke/include/cryptocontext.h`, `src/pke/lib/schemerns/rns-leveledshe.cpp`, `src/pke/lib/scheme/ckksrns/ckksrns-leveledshe.cpp` |
| Rotate | `cc->EvalRotate(ct, step)`, alias keygen `EvalRotateKeyGen(sk, steps)` | rotation/Galois key for each step | The key-generation path maps rotation indices into automorphism keys. Missing step raises a key-not-found exception. | `src/pke/include/cryptocontext.h`, `src/pke/lib/schemebase/base-leveledshe.cpp`, `src/pke/examples/rotation.cpp` |
| Conjugate | CKKS bootstrapping uses `FHECKKSRNS::Conjugate`; lower-level public path is `EvalAutomorphism` with the conjugation automorphism key (`M-1`) [需人工确认 preferred public wrapper] | conjugation/automorphism key | Same CKKS semantics as core `Conjugate`: scale and level unchanged; key switching adds error. Bootstrap key generation inserts the conjugation key. | `src/pke/lib/scheme/ckksrns/ckksrns-fhe.cpp`, `src/pke/include/cryptocontext.h` |
| ModSwitch / DropLevel | `cc->LevelReduce(ct, evalKey, levels)` / `LevelReduceInPlace`; internal `LevelReduceInternalInPlace` | none; public signature carries unused `evalKey` in RNS path | Drops towers without changing scale semantics. Public `LevelReduceInPlace` performs in `FIXEDMANUAL`; AUTO alignment uses internals. | `src/pke/lib/schemerns/rns-leveledshe.cpp`, `src/pke/lib/scheme/ckksrns/ckksrns-leveledshe.cpp` |
| Bootstrap | `cc->EvalBootstrapSetup`, `cc->EvalBootstrapKeyGen`, `cc->EvalBootstrapPrecompute`, `cc->EvalBootstrap` | HYBRID key switching, bootstrap rotations, conjugation, relin | Setup precomputes linear transforms; keygen computes rotation/conjugation keys; evaluation runs ModRaise, CoeffsToSlots, EvalMod/sine, SlotsToCoeffs. | `src/pke/lib/scheme/ckksrns/ckksrns-fhe.cpp`, `src/pke/examples/simple-ckks-bootstrapping.cpp` |

## Scale Management Model

- `scaleModSize` is a bit-size target, not necessarily an exact global scale. The examples state that the actual scale is implementation-specific and may vary between ciphertexts in flexible modes. Source: `src/pke/examples/simple-real-numbers.cpp`.
- `FIXEDMANUAL`: use when you want explicit schedule control. After ciphertext-ciphertext or scaled plaintext-ciphertext multiplication, call `cc->Rescale`/`cc->ModReduce` according to the design note. OpenFHE still performs automatic level reduction for some operand alignment, but it does not automatically place rescale for you. Sources: `src/pke/examples/advanced-real-numbers.cpp`, `src/pke/lib/schemerns/rns-leveledshe.cpp`.
- `FIXEDAUTO`: automatic version of fixed/manual scaling. It hides depth tracking and rescale calls for normal `EvalMult` style code while keeping the fixed-scaling approximation model. It is the default CKKS scaling technique for `NATIVEINT == 128`. Sources: `src/pke/examples/advanced-real-numbers-128.cpp`, `src/pke/include/scheme/gen-cryptocontext-params-defaults.h`.
- `FLEXIBLEAUTO`: tracks actual per-level scaling factors and automatically prepares operands before multiplications. It can be more accurate for deep computations but may be slower; modulus generation tries to keep level scales close and throws if the ratio diverges too much. Sources: `src/pke/examples/advanced-real-numbers.cpp`, `src/pke/lib/scheme/ckksrns/ckksrns-parametergeneration.cpp`, `src/pke/lib/scheme/ckksrns/ckksrns-cryptoparameters.cpp`.
- `FLEXIBLEAUTOEXT`: flexible auto plus an internal extra prime `q'` used by `*EXT` methods. It is the default CKKS scaling technique for non-128-bit builds. Sources: `src/pke/include/scheme/gen-cryptocontext-params-defaults.h`, `src/pke/include/scheme/gen-cryptocontext-params.h`, `src/pke/lib/scheme/ckksrns/ckksrns-parametergeneration.cpp`.
- Do not blindly insert `Rescale` after every multiply in AUTO modes. The normal idiom is to call `EvalMult`, `EvalSquare`, `EvalPoly`, `EvalBootstrap`, etc., and let OpenFHE perform internal `AdjustForMult`, `AdjustLevelsAndDepthToOne`, and `ModReduceInternal` steps. Manual `Rescale` belongs mainly to `FIXEDMANUAL` code or very explicit low-level experiments. Sources: `src/pke/lib/schemerns/rns-leveledshe.cpp`, `src/pke/lib/scheme/ckksrns/ckksrns-leveledshe.cpp`, `src/pke/examples/advanced-real-numbers.cpp`.
- Restrictions/gotchas:
  - 128-bit CKKS rejects `FLEXIBLEAUTO`, `FLEXIBLEAUTOEXT`, and composite scaling in generator checks; use `FIXEDMANUAL` or `FIXEDAUTO`. Source: `src/pke/include/scheme/ckksrns/gen-cryptocontext-ckksrns-internal.h`, `src/pke/examples/advanced-real-numbers-128.cpp`.
  - CKKS bootstrapping requires HYBRID key switching. 128-bit CKKS bootstrapping supports `FIXEDMANUAL` and `FIXEDAUTO` only. Source: `src/pke/lib/scheme/ckksrns/ckksrns-fhe.cpp`.
  - For HYBRID key switching, security depends on `Q*P`, not just `Q`; parameter generation adds the estimated `P` size into HE-standard ring-dimension selection. Source: `src/pke/examples/README.md`, `src/pke/lib/scheme/ckksrns/ckksrns-parametergeneration.cpp`.

## Core Implementation Strategies

### EvalMult and Relinearization

`cc->EvalMult(ct1, ct2)` retrieves the evaluation key vector generated by `EvalMultKeyGen` and calls the scheme multiplication with the first relin key. The no-relinearization variant `EvalMultNoRelin` returns an extended ciphertext and is for explicit lazy relinearization. Relinearization key generation creates a private key under `s^2` and key-switches it back to `s`; relinearization applies `KeySwitchCore` to each component from index 2 onward and folds the result into `(c0,c1)`. Sources: `src/pke/include/cryptocontext.h`, `src/pke/lib/schemebase/base-leveledshe.cpp`.

Engineering rule: budget OpenFHE `EvalMult` as `Mul + Relinearize`; in AUTO modes, also expect hidden pre-multiply scale/level adjustment when an operand is at noise-scale degree 2.

### Rescale / ModReduce and LevelReduce

CKKS `ModReduceInternalInPlace` drops one or more RNS towers with `DropLastElementAndScale`, increments `ciphertext->GetLevel()`, decreases `NoiseScaleDeg`, and divides the stored scaling factor by the removed modulus factor. `LevelReduceInternalInPlace` only drops towers and increments level. Sources: `src/pke/lib/scheme/ckksrns/ckksrns-leveledshe.cpp`.

Public `ModReduceInPlace`/`LevelReduceInPlace` only act for `FIXEDMANUAL`; AUTO modes rely on internal calls made by `EvalMult`, `EvalSquare`, add/sub alignment, bootstrap, and advanced polynomial evaluators. Sources: `src/pke/lib/schemerns/rns-leveledshe.cpp`.

### Key Switching

Default CKKS key switching is HYBRID. HYBRID extends the new secret key from basis `Q` to `QP`, partitions `Q` into parts, samples `a/e`, and stores evaluation-key vectors over `QP` with `PModq * sOld` injected only for the active part. Runtime key switching decomposes/precomputes the component, performs fast key-switch core over `QP`, and mods down. Sources: `src/pke/lib/keyswitch/keyswitch-hybrid.cpp`.

BV exists and uses digit decomposition; `digitSize` matters for BV. HYBRID uses `numLargeDigits` and introduces `P`, which must be included in security budgeting. Sources: `src/pke/examples/README.md`, `src/pke/examples/advanced-real-numbers.cpp`.

### KeyGen

Use `auto keys = cc->KeyGen()` for public/secret keys. Then generate only the evaluation keys required by the design note:

- Relinearization: `cc->EvalMultKeyGen(keys.secretKey)`.
- Rotations: `cc->EvalRotateKeyGen(keys.secretKey, rotationSteps)`; negative steps rotate in the opposite direction.
- Slot sum: `cc->EvalSumKeyGen(keys.secretKey)` if using `EvalSum`.
- Bootstrap: `cc->EvalBootstrapKeyGen(keys.secretKey, numSlots)`, after `EvalBootstrapSetup`.
- Raw key switching: `cc->KeySwitchGen(oldSk, newSk)` if implementing nonstandard switching.

Sources: `src/pke/examples/simple-real-numbers.cpp`, `src/pke/examples/rotation.cpp`, `src/pke/include/cryptocontext.h`, `src/pke/lib/cryptocontext.cpp`.

### Bootstrap

OpenFHE CKKS bootstrapping has three public setup steps:

1. `EvalBootstrapSetup(levelBudget, dim1, slots, correctionFactor, precompute, BTSlotsEncoding)`: validates HYBRID, chooses correction factor, prepares CoeffsToSlots/SlotsToCoeffs linear-transform plaintexts or FFT decompositions.
2. `EvalBootstrapKeyGen(sk, slots)`: finds bootstrap rotation indices, generates rotation keys, adds conjugation key `M-1`, and adds sparse-secret switching keys when using sparse encapsulation.
3. `EvalBootstrap(ct, numIterations, precision)`: reduces to depth 1, raises modulus, applies CoeffsToSlots, approximates modular reduction with Chebyshev sine plus double-angle iterations, and applies SlotsToCoeffs.

Sources: `src/pke/lib/scheme/ckksrns/ckksrns-fhe.cpp`, `src/pke/examples/simple-ckks-bootstrapping.cpp`.

## OpenFHE-Specific Gotchas

- `cc->Rescale` and `cc->ModReduce` map to the same OpenFHE ModReduce path. This is not SEAL `mod_switch_to_next`; use `LevelReduce` for scale-preserving tower drop. Source: `src/pke/include/cryptocontext.h`.
- Public `ModReduceInPlace` does nothing in AUTO modes. If AUTO code appears not to consume levels immediately after a multiply, that is expected; OpenFHE reduces when needed by the next operation or internal evaluator. Source: `src/pke/lib/schemerns/rns-leveledshe.cpp`.
- `EvalMult` requires `EvalMultKeyGen`; otherwise `cryptocontext.h` throws "Evaluation key has not been generated for EvalMult". Source: `src/pke/include/cryptocontext.h`.
- `EvalRotate` requires exactly generated rotation indices; missing keys raise an exception. Source: `src/pke/lib/schemebase/base-leveledshe.cpp`.
- `EvalAutomorphism` requires a 2-component ciphertext; relinearize before rotating extended ciphertexts. Source: `src/pke/lib/schemebase/base-leveledshe.cpp`.
- Full packing default: if `batchSize` is zero, CKKS batch size becomes `N/2`; explicit `SetBatchSize` must be a power of two and no larger than `N/2`. Source: `src/pke/lib/scheme/ckksrns/ckksrns-parametergeneration.cpp`.
- For examples using `HEStd_NotSet` and explicit `SetRingDim`, do not copy into production without re-enabling HE-standard security or running an estimator. Source: `src/pke/examples/simple-ckks-bootstrapping.cpp`.
- `ClearStaticMapsAndVectors()` appears in bootstrapping examples to clear static key/precompute maps between demos; keep this in mind when writing multi-context tests. Source: `src/pke/examples/simple-ckks-bootstrapping.cpp`.
- Noise flooding decrypt exists for CKKS via `NOISE_FLOODING_DECRYPT` and two execution modes; it requires estimating noise before evaluation. Source: `src/pke/examples/README.md`, `src/pke/lib/scheme/ckksrns/ckksrns-pke.cpp`.

## Example Walkthroughs With 7-Step Design Discipline

### 1. Basic arithmetic: `simple-real-numbers.cpp`

1. Function: compute slotwise `x1+x2`, `x1-x2`, `4*x1`, `x1*x2`, and rotations of `x1`.
2. Arithmetic circuit: add/sub depth 0; scalar multiply has no relin but changes scale; ciphertext multiply has depth 1; rotations depth 0.
3. Packing: `batchSize = 8`, vectors occupy 8 CKKS slots. Rotation steps are `{1, -2}`.
4. Parameters: `multiplicativeDepth = 1`, `scaleModSize = 50`, ring dimension chosen from security defaults. Slots are bounded by `N/2`.
5. Keys: public/secret key, relin key via `EvalMultKeyGen`, rotation keys via `EvalRotateKeyGen({1, -2})`.
6. Implementation: `MakeCKKSPackedPlaintext`, `Encrypt`, `EvalAdd`, `EvalSub`, `EvalMult` for scalar and ciphertext multiply, `EvalRotate`. No manual `Rescale` because default CKKS scaling is AUTO on typical 64-bit builds.
7. Verification: decrypt each output, `SetLength(batchSize)`, print precision using `GetLogPrecision` for selected outputs. Source: `src/pke/examples/simple-real-numbers.cpp`.

### 2. Rotation/packing: `rotation.cpp`

1. Function: demonstrate CKKS slot rotations for many indices.
2. Arithmetic circuit: rotation-only, multiplicative depth 0; example sets depth 2 for context capacity.
3. Packing: `n = cyclotomicOrder / 4 = N/2` CKKS slots; vector is resized to `n`, and output length is truncated for display.
4. Parameters: `multiplicativeDepth = 2`, `scalingModSize = 40`.
5. Keys: `KeyGen`, then `EvalRotateKeyGen` with every planned index: `{2,3,4,5,6,7,8,9,10,-n+2,-n+3,n-1,n-2,-1,-2,-3,-4,-5}`.
6. Implementation: loop over indices and call `EvalRotate(ciphertext, index)`. Rotation keys are generated before encryption/evaluation.
7. Verification: decrypt each rotated ciphertext and set displayed length. Source: `src/pke/examples/rotation.cpp`.

### 3. Bootstrap: `simple-ckks-bootstrapping.cpp`

1. Function: refresh a depleted CKKS ciphertext so more multiplicative levels become available.
2. Arithmetic circuit: bootstrap itself is a deep approximate circuit: ModRaise, CoeffsToSlots, Chebyshev sine/EvalMod plus double-angle iterations, SlotsToCoeffs. Application payload is identity on the encrypted vector.
3. Packing: full packing uses `numSlots = ringDim / 2`; StC-first variant reserves enough pre-bootstrap levels for SlotsToCoeffs.
4. Parameters: `UNIFORM_TERNARY`; examples use `HEStd_NotSet` with `ringDim = 1 << 12` for speed. 64-bit example uses `FLEXIBLEAUTO`; 128-bit example path uses `FIXEDAUTO`. Depth is `levelsAvailableAfterBootstrap + FHECKKSRNS::GetBootstrapDepth(levelBudget, secretKeyDist)`.
5. Keys: enable `FHE`; call `EvalBootstrapSetup`; then `KeyGen`, `EvalMultKeyGen`, and `EvalBootstrapKeyGen`.
6. Implementation: encode at a high initial level with `MakeCKKSPackedPlaintext(x, 1, depth - 1)` so ciphertext starts depleted; call `EvalBootstrap(ciph)`. For StC-first, pass `BTSlotsEncoding=true` in setup and encode with `depth - 1 - levelBudget[1]`.
7. Verification: print remaining levels before/after and decrypt slotwise. Production code must replace `HEStd_NotSet` with a real security target or estimator run. Source: `src/pke/examples/simple-ckks-bootstrapping.cpp`, `src/pke/lib/scheme/ckksrns/ckksrns-fhe.cpp`.

### 4. Polynomial/nonlinear template: `polynomial-evaluation.cpp`

1. Function: evaluate two plaintext-coefficient polynomials on packed CKKS inputs.
2. Arithmetic circuit: depth depends on degree and evaluator strategy; example sets `multiplicativeDepth = 6`.
3. Packing: complex input vector of length 5 in CKKS slots.
4. Parameters: `scaleModSize = 50`; default security and scaling technique.
5. Keys: `EvalMultKeyGen` only; no rotations.
6. Implementation: enable `ADVANCEDSHE`; call `EvalPoly(ciphertext, coefficients)`. OpenFHE chooses linear or Paterson-Stockmeyer style internals via advanced SHE APIs.
7. Verification: decrypt, set length, compare to expected numeric values printed by the example. Source: `src/pke/examples/polynomial-evaluation.cpp`, `src/pke/include/cryptocontext.h`.

## Quick Budget Checklist For Codex-Generated OpenFHE CKKS Code

- Before writing code, list multiplicative depth, plaintext-ciphertext multiplications that increase scale, and all rotations.
- If using default/AUTO CKKS, do not sprinkle `Rescale` after every multiply. Let `EvalMult`/`EvalSquare`/`EvalPoly` manage the schedule unless a local example in the same scaling mode does otherwise.
- If using `FIXEDMANUAL`, explicitly name every `Rescale`/`ModReduce` in the design note and implementation.
- Generate `EvalMultKeyGen` before any ciphertext-ciphertext multiplication, square, polynomial evaluation, or bootstrap.
- Generate `EvalRotateKeyGen` for exactly the layout-derived rotation set; include bootstrap rotations via `EvalBootstrapKeyGen`, not by guessing them manually.
- For HYBRID, include special modulus `P` in the security budget.
- Validate with plaintext reference, `SetLength`, max absolute/relative error, and `GetLogPrecision` where useful.
