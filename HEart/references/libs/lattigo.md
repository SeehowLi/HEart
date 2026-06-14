# Lattigo CKKS Notes

Pinned version: Lattigo 6.2.0, local source tree `lattigo-6.2.0`.

中文摘要: This file maps the core CKKS mental model and operation semantics to the local Lattigo 6.2.0 source. The important version check is that this tree uses `schemes/ckks` for CKKS and `circuits/ckks/bootstrapping` for bootstrapping; the CKKS scale type is `core/rlwe.Scale`. Do not write v6.2.0 code from memory using `he/hefloat` unless the local source tree is changed.

## Repository Map

- CKKS scheme implementation: `schemes/ckks/`
  - `ckks.go`: constructors for `ckks.NewPlaintext`, `ckks.NewCiphertext`, `ckks.NewEncryptor`, `ckks.NewDecryptor`, and `ckks.NewKeyGenerator` wrappers around RLWE objects.
  - `params.go`: `ckks.ParametersLiteral`, `NewParametersFromLiteral`, `LogDefaultScale`, `PrecisionMode`, `LevelsConsumedPerRescaling`, slot and Galois-element helpers.
  - `evaluator.go`: CKKS evaluator operations: add/sub, scalar/plaintext/ciphertext multiplication, rescale, drop level, rotations, conjugation, inner sums, and key swapping via `WithKey`.
  - `encoder.go`: CKKS slot/coefficient encoding.
  - `README.md`: parameter-selection notes and 128-bit total-modulus bounds.
- Generic RLWE layer: `core/rlwe/`
  - `scale.go`: `rlwe.Scale`, a 128-bit precision scale tracker backed by `big.Float`.
  - `keygenerator.go`: secret/public key generation, relinearization keys, Galois keys, and generic evaluation keys.
  - `keys.go`: `rlwe.EvaluationKeySet` and `rlwe.NewMemEvaluationKeySet`.
  - `evaluator.go`: key switching, automorphisms, relinearization, output validation, NTT-domain checks, and evaluation-key lookup.
- CKKS circuits:
  - `circuits/ckks/polynomial/`: Chebyshev/minimax polynomial evaluation helpers.
  - `circuits/ckks/lintrans/`: diagonal linear transforms, BSGS support, and Galois-element discovery for transforms.
  - `circuits/ckks/bootstrapping/`: bootstrapping parameters, evaluation-key generation, evaluator, and bootstrap pipeline.

CKKS-related example files:

`examples/singleparty/templates/ckks/main.go`, `examples/singleparty/tutorials/ckks/main.go`, `examples/singleparty/ckks_vectorized_polynomial_evaluation/main.go`, `examples/singleparty/ckks_sigmoid_minimax/main.go`, `examples/singleparty/ckks_sigmoid_chebyshev/main.go`, `examples/singleparty/ckks_bootstrapping/basics/main.go`, `examples/singleparty/ckks_bootstrapping/slim/main.go`, `examples/singleparty/ckks_bootstrapping/high_precision/main.go`, `examples/singleparty/ckks_scheme_switching/main.go`.

Sources: `schemes/ckks/*`, `core/rlwe/*`, `circuits/ckks/*`, `examples/singleparty/*/main.go`.

## Setup Boilerplate

Minimal single-party CKKS setup with explicit parameters:

```go
import (
    "github.com/tuneinsight/lattigo/v6/core/rlwe"
    "github.com/tuneinsight/lattigo/v6/schemes/ckks"
)

params, err := ckks.NewParametersFromLiteral(ckks.ParametersLiteral{
    LogN:            14,
    LogQ:            []int{55, 45, 45, 45, 45, 45, 45, 45},
    LogP:            []int{61},
    LogDefaultScale: 45,
})
if err != nil {
    panic(err)
}

kgen := rlwe.NewKeyGenerator(params)
sk, pk := kgen.GenKeyPairNew()
rlk := kgen.GenRelinearizationKeyNew(sk)

galEls := []uint64{
    params.GaloisElementForRotation(1),
    params.GaloisElementForRotation(-2),
    params.GaloisElementForComplexConjugation(),
}
gks := kgen.GenGaloisKeysNew(galEls, sk)
evk := rlwe.NewMemEvaluationKeySet(rlk, gks...)

encoder := ckks.NewEncoder(params)
encryptor := rlwe.NewEncryptor(params, pk)
decryptor := rlwe.NewDecryptor(params, sk)
eval := ckks.NewEvaluator(params, evk)

pt := ckks.NewPlaintext(params, params.MaxLevel())
if err := encoder.Encode(values, pt); err != nil {
    panic(err)
}
ct, err := encryptor.EncryptNew(pt)
if err != nil {
    panic(err)
}
_ = decryptor
_ = eval
```

Parameter rules:

- `LogN` is the ring degree exponent; slots are `params.MaxSlots()` or `1 << params.LogMaxSlots()`. Standard CKKS packs up to `N/2` complex slots; `ring.ConjugateInvariant` packs real slots and does not support `Conjugate`. Sources: `schemes/ckks/README.md`, `schemes/ckks/params.go`.
- `LogQ` is the ciphertext modulus chain. The first prime stores the final result and message headroom; each later scaling prime usually corresponds to one rescale in `PREC64`. Sources: `schemes/ckks/example_parameters.go`, `schemes/ckks/README.md`.
- `LogP` is auxiliary key-switching modulus; include `Q*P` in security checks. A single `P` works for small chains; several `P` primes reduce hybrid key-switch decomposition width but increase security cost. Source: `schemes/ckks/example_parameters.go`.
- `LogDefaultScale` initializes `params.DefaultScale()`. `LogDefaultScale <= 64` gives `PREC64` and one prime per `Rescale`; `LogDefaultScale > 64` gives `PREC128` and two primes per `Rescale`. `LogDefaultScale > 128` is rejected. Source: `schemes/ckks/params.go`.

## API Mapping From Core Operation Semantics

| Core primitive | Lattigo 6.2.0 API | Key requirement | Scale/level note | Sources |
|---|---|---|---|---|
| Add | `eval.Add(ct0, ctOrPlainOrScalar, out)`, `eval.AddNew(...)` | none | Uses the minimum input level; tries to align unequal scales by multiplying the lower-scale operand by an integer ratio. This only works when one scale is an integer multiple of the other. | `schemes/ckks/evaluator.go`, `examples/singleparty/tutorials/ckks/main.go` |
| AddPlain | Same `eval.Add`/`AddNew` with `*rlwe.Plaintext`, scalar, or vector operand | none | Scalar/vector operands are encoded at the ciphertext scale; pre-encoded plaintexts must already have compatible metadata. | `schemes/ckks/evaluator.go` |
| MulConst / MulPlain | `eval.Mul(ct, scalarOrPlainOrVector, out)`, `eval.MulNew(...)`; `eval.MulRelin` is accepted but relin is only relevant for ciphertext operands | none for plaintext/scalar; relin key only for ct-ct `MulRelin` | Non-Gaussian-integer scalar/vector/plaintext multiplication increases scale; schedule `eval.Rescale` before the next multiplication unless deliberately accumulating a fused pattern. Gaussian integer scalar multiplication uses scale 1 and does not consume a rescale level. | `schemes/ckks/evaluator.go`, `examples/singleparty/tutorials/ckks/main.go` |
| Mul | `eval.Mul(ct0, ct1, out)` for degree-2 output; `eval.MulRelin(ct0, ct1, out)` / `MulRelinNew` for normal degree-1 output | `rlk` for `MulRelin`; none for raw extended `Mul` | Output scale is `ct0.Scale * ct1.Scale`; output level is the minimum input/output level. Raw `Mul` produces degree 2 and must be relinearized before ordinary further multiplication/rotation patterns. | `schemes/ckks/evaluator.go` |
| Relinearize | `eval.Relinearize(op0, opOut)`, `eval.RelinearizeNew(op0)` inherited through embedded `*rlwe.Evaluator`; `eval.MulRelin` fuses multiply and relin | `kgen.GenRelinearizationKeyNew(sk)` stored in `rlwe.NewMemEvaluationKeySet` | Scale and CKKS level are unchanged; key switching adds error and uses `P`/decomposition internally. | `schemes/ckks/evaluator.go`, `core/rlwe/evaluator.go`, `core/rlwe/keygenerator.go` |
| Rescale / ModReduce | `eval.Rescale(op0, opOut)`, `eval.RescaleTo(op0, minScale, opOut)`, `eval.SetScale(ct, scale)` | none | Divides by the last `Q` prime, repeated `params.LevelsConsumedPerRescaling()` times. `SetScale` multiplies by a ratio and then `RescaleTo`, so it spends a level. | `schemes/ckks/evaluator.go`, `schemes/ckks/params.go` |
| Rotate | `eval.Rotate(ct, k, out)`, `eval.RotateNew(ct, k)`, `eval.RotateHoisted` | Galois key for `params.GaloisElementForRotation(k)` or `params.GaloisElement(k)` | Scale and level unchanged; adds key-switching error. `RotateHoisted` amortizes decomposition for many rotations. | `schemes/ckks/evaluator.go`, `core/rlwe/evaluator.go` |
| Conjugate | `eval.Conjugate(ct, out)`, `eval.ConjugateNew(ct)` | Galois key for `params.GaloisElementForComplexConjugation()` | Not supported for `ring.ConjugateInvariant`; scale/level unchanged in standard CKKS. | `schemes/ckks/evaluator.go`, `schemes/ckks/params.go` |
| ModSwitch / DropLevel | `eval.DropLevel(ct, levels)`, `eval.DropLevelNew(ct, levels)` | none | Drops `Q` primes by resizing the ciphertext; no scale division. Use for level alignment, not for post-multiply scale normalization. | `schemes/ckks/evaluator.go` |
| Bootstrap | `bootstrapping.NewParametersFromLiteral`, `btpParams.GenEvaluationKeys(sk)`, `bootstrapping.NewEvaluator(btpParams, evk)`, `eval.Bootstrap(ct)` / `BootstrapMany` | bootstrapping `EvaluationKeys`: relin, many Galois keys, optional ring/ring-type switch keys, dense/sparse encapsulation keys | Requires residual and bootstrapping parameters. If input is at level 0, scale must fit the bootstrap message-ratio constraints; at level >= 1 the evaluator can spend a level for scale matching. | `circuits/ckks/bootstrapping/*`, `examples/singleparty/ckks_bootstrapping/basics/main.go` |

## Scale Management Model

- Lattigo does not expose OpenFHE-style `FIXEDMANUAL` / `FIXEDAUTO` / `FLEXIBLEAUTO` scaling modes in this local v6.2.0 CKKS path. The evaluator tracks `rlwe.Scale` on every plaintext/ciphertext, and the code must explicitly call `Rescale` or `RescaleTo` when the circuit schedule requires scale reduction. Sources: `core/rlwe/scale.go`, `schemes/ckks/evaluator.go`.
- `rlwe.Scale` is a 128-bit precision `big.Float` wrapper with arithmetic helpers (`Mul`, `Div`, `Cmp`, `Equal`, `Log2`). It is not a plain `float64`; use `rlwe.NewScale(...)` and keep metadata coherent. Source: `core/rlwe/scale.go`.
- Add/Sub scale alignment is limited: Lattigo multiplies the lower-scale operand by an integer ratio when possible. If scales differ by a non-integer/fractional ratio, addition may introduce approximation error proportional to the mismatch. Decision rule: before fan-in additions, bring operands to scales that are equal or integer multiples. Sources: `schemes/ckks/evaluator.go`, `examples/singleparty/tutorials/ckks/main.go`.
- Ciphertext-ciphertext multiplication sets `out.Scale = op0.Scale * op1.Scale`. `MulRelin` handles degree reduction but not rescaling. Decision rule: after a nontrivial multiplication, call `eval.Rescale(out, out)` before the next multiplication unless a local circuit intentionally delays rescale. Sources: `schemes/ckks/evaluator.go`, `examples/singleparty/tutorials/ckks/main.go`.
- Plaintext/scalar/vector multiplication can also require rescale. Lattigo helps by encoding scalar/vector operands at the current modulus prime so the following rescale returns near the previous working scale. Pre-encoded plaintexts are your responsibility. Source: `schemes/ckks/evaluator.go`.
- `PREC128` consumes two primes per `Rescale`; count depth as `params.MaxLevel() / params.LevelsConsumedPerRescaling()`, not simply `len(LogQ)-1`, when `LogDefaultScale > 64`. Source: `schemes/ckks/params.go`.
- `DropLevel` is the scale-preserving operation. It is the Lattigo mapping for core `ModSwitch / DropLevel`, not a replacement for `Rescale`. Source: `schemes/ckks/evaluator.go`.

## Key Generation

- Basic keys: `kgen := rlwe.NewKeyGenerator(params)` or `ckks.NewKeyGenerator(params)`; both route to the RLWE key generator in this tree. Use `GenSecretKeyNew`, `GenPublicKeyNew(sk)`, or `GenKeyPairNew()`. Sources: `schemes/ckks/ckks.go`, `core/rlwe/keygenerator.go`.
- Relinearization: `rlk := kgen.GenRelinearizationKeyNew(sk)`, then store it in `rlwe.NewMemEvaluationKeySet(rlk, ...)`. Required by `MulRelin`, polynomial evaluation, and most production ciphertext-ciphertext multiplication schedules. Source: `core/rlwe/keygenerator.go`.
- Rotations: generate exact Galois elements, not raw step integers: `kgen.GenGaloisKeysNew([]uint64{params.GaloisElementForRotation(k)}, sk)`. For inner sums use `params.GaloisElementsForInnerSum(batch, n)`; for replicate use `params.GaloisElementsForReplicate(batch, n)`; for linear transforms use `lintrans.GaloisElements(params, ltparams)`. Sources: `schemes/ckks/params.go`, `examples/singleparty/tutorials/ckks/main.go`.
- Conjugation: add `params.GaloisElementForComplexConjugation()` for standard CKKS. Do not request it for `ring.ConjugateInvariant`; `eval.Conjugate` returns an error there. Source: `schemes/ckks/evaluator.go`.
- Evaluation key set: `evk := rlwe.NewMemEvaluationKeySet(rlk, gks...)`. `eval.WithKey(newEvk)` returns an evaluator with a different key set; the implementation says the returned evaluator shares temporary buffers with the receiver and cannot be used concurrently with it. Source: `core/rlwe/keys.go`, `schemes/ckks/evaluator.go`.

## Bootstrapping Usage

Minimal pattern:

```go
import (
    "github.com/tuneinsight/lattigo/v6/circuits/ckks/bootstrapping"
    "github.com/tuneinsight/lattigo/v6/core/rlwe"
    "github.com/tuneinsight/lattigo/v6/schemes/ckks"
)

params, err := ckks.NewParametersFromLiteral(ckks.ParametersLiteral{
    LogN:            16,
    LogQ:            []int{55, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40},
    LogP:            []int{61, 61, 61},
    LogDefaultScale: 40,
})
if err != nil {
    panic(err)
}

logN := params.LogN()
btpLit := bootstrapping.ParametersLiteral{
    LogN: &logN,
    LogP: []int{61, 61, 61, 61},
    Xs:   params.Xs(),
}

btpParams, err := bootstrapping.NewParametersFromLiteral(params, btpLit)
if err != nil {
    panic(err)
}

kgen := rlwe.NewKeyGenerator(params)
sk, pk := kgen.GenKeyPairNew()
_ = pk

btpKeys, _, err := btpParams.GenEvaluationKeys(sk)
if err != nil {
    panic(err)
}

btpEval, err := bootstrapping.NewEvaluator(btpParams, btpKeys)
if err != nil {
    panic(err)
}

ctBoot, err := btpEval.Bootstrap(ct)
if err != nil {
    panic(err)
}
_ = ctBoot
```

Bootstrap strategy:

- The package uses residual parameters for the ciphertext outside bootstrap and separate bootstrapping parameters for the internal circuit. Always print/check both `LogQP()` values and security assumptions. Source: `examples/singleparty/ckks_bootstrapping/basics/main.go`.
- The full circuit is `ScaleDown -> ModUp -> CoeffsToSlots -> EvalMod -> SlotsToCoeffs`; `Parameters.Depth()` decomposes into `DepthCoeffsToSlots + DepthEvalMod + DepthSlotsToCoeffs`. Source: `circuits/ckks/bootstrapping/evaluator.go`, `circuits/ckks/bootstrapping/parameters.go`.
- `GenEvaluationKeys` creates relin and Galois keys for the bootstrapping circuit, optional ring-degree/ring-type switching keys, and dense/sparse encapsulation keys. The generated keys can dominate memory and generation time. Source: `circuits/ckks/bootstrapping/keys.go`.
- `Parameters.GaloisElements` collects rotations for `SubSum`, CoeffsToSlots, SlotsToCoeffs, and conjugation. Do not guess bootstrap rotations manually; use `GenEvaluationKeys` or the helper. Source: `circuits/ckks/bootstrapping/parameters.go`.
- Scale precondition: the example states that the ciphertext scale must be equal or very close to `params.DefaultScale()` before bootstrap. `eval.SetScale(ciphertext, params.DefaultScale())` can equalize scale at the expense of one level; if input level is at least one, the bootstrapper can use that level for matching. Sources: `examples/singleparty/ckks_bootstrapping/basics/main.go`, `circuits/ckks/bootstrapping/evaluator.go`.

## Core Implementation Strategies

### EvalMult and Relinearization

`eval.Mul` accepts ciphertext, plaintext, scalar, or vector second operands. For ciphertext operands it checks that total input degree is at most 2, resizes output at the minimum available level, and calls `mulRelin(..., relin=false)`. That raw path can produce a degree-2 ciphertext. `eval.MulRelin` calls the same internal path with `relin=true` for ciphertext operands and requires a relinearization key; for scalar/plain/vector operands it routes through `Mul`. Sources: `schemes/ckks/evaluator.go`.

Engineering rule: treat `MulRelin` as "Mul + key switch, no rescale." Treat `Mul` as an extended-ciphertext primitive; follow with `Relinearize` before ordinary further ct-ct multiplication unless the circuit intentionally uses lazy accumulation.

### Rescale and DropLevel

`eval.Rescale` divides by the last modulus prime and repeats this `params.LevelsConsumedPerRescaling()` times. It updates `ct.Scale` by dividing by the dropped prime(s) and performs `DivRoundByLastModulusManyNTT`. `eval.DropLevel` only resizes the ciphertext down to a lower level; no scale division is applied. Sources: `schemes/ckks/evaluator.go`, `schemes/ckks/params.go`.

Engineering rule: use `Rescale` after scale-growing multiplication; use `DropLevel` only to align levels while preserving scale semantics.

### Key Switching and Automorphisms

The embedded `rlwe.Evaluator` owns key switching, automorphisms, and relinearization. `InitOutputBinaryOp` checks metadata, batching, NTT-domain compatibility, total degree, and selects the minimum level. `CheckAndGetRelinearizationKey` and `CheckAndGetGaloisKey` fail early when keys are missing. `Rotate` and `Conjugate` call automorphisms using Galois elements derived from parameters. Sources: `core/rlwe/evaluator.go`, `schemes/ckks/evaluator.go`.

Engineering rule: Galois keys are indexed by Galois element, not by human-readable rotation step. Generate from `params` helpers and keep the helper call next to the layout design note.

### KeyGen

`GenRelinearizationKeyNew(sk)` creates the switching material needed to bring degree-2 multiplication output back under the normal secret-key form. `GenGaloisKeysNew(galEls, sk)` creates automorphism keys. `GenEvaluationKeyNew(skInput, skOutput)` supports nonstandard key switching and ring-degree bridging. Sources: `core/rlwe/keygenerator.go`.

Engineering rule: generate only keys listed by the design note: one relin key if any ct-ct multiplication/polynomial/bootstrap is used, Galois keys for exact rotations/conjugation, and bootstrap keys through the bootstrapping parameter object.

### Bootstrap

`bootstrapping.NewEvaluator` validates the consistency of CoeffsToSlots, EvalMod, and SlotsToCoeffs levels and checks that required Galois and encapsulation keys are present. `Bootstrap` calls `Evaluate`; `Evaluate` either runs a single bootstrap or an iterative/high-precision path. The internal `bootstrap` function runs `ScaleDown`, `ModUp`, `CoeffsToSlots`, `EvalMod` on real/imag parts, and `SlotsToCoeffs`. Sources: `circuits/ckks/bootstrapping/evaluator.go`.

Engineering rule: budget bootstrap as a separate circuit with its own `LogQP`, depth, Galois key set, and security check. It refreshes levels; it does not make approximate CKKS output exact.

## Lattigo-Specific Gotchas

- Package path hazard: this local v6.2.0 source uses `github.com/tuneinsight/lattigo/v6/schemes/ckks`, not `he/hefloat`. The tutorial comment still mentions `he/float`, but the compiled examples import `schemes/ckks`. Sources: `go.mod`, `examples/singleparty/tutorials/ckks/main.go`.
- Scale type hazard: use `rlwe.Scale`, not a float. Read/write `ct.Scale`, `pt.Scale`, and `params.DefaultScale()` deliberately. Source: `core/rlwe/scale.go`.
- Rescale is explicit. Lattigo will not automatically insert OpenFHE-style AUTO rescale after `MulRelin`; examples call `eval.Rescale` after scale-growing multiplication and before further multiplications. Source: `examples/singleparty/tutorials/ckks/main.go`.
- Add scale alignment is partial. If one scale is not an integer multiple of the other, Lattigo cannot fully reconcile it; expect numerical error. Source: `examples/singleparty/tutorials/ckks/main.go`.
- `DropLevel` is not `Rescale`. It removes primes without changing scale; using it after multiplication leaves scale too large. Source: `schemes/ckks/evaluator.go`.
- Missing Galois keys fail at runtime with a key-not-found error. Generate exact keys from `params.GaloisElementForRotation(k)`, `params.GaloisElementsForInnerSum`, `params.GaloisElementsForReplicate`, or circuit helpers. Sources: `core/rlwe/evaluator.go`, `schemes/ckks/params.go`.
- `eval.WithKey` shares temporary buffers with the original evaluator and the source says the receiver and returned evaluator cannot be used concurrently. Create separate evaluators if you need concurrent evaluation with different key sets. Source: `schemes/ckks/evaluator.go`.
- `ring.ConjugateInvariant` gives real packing and disables `Conjugate`; it can require ring-switching keys in bootstrap. Sources: `schemes/ckks/README.md`, `circuits/ckks/bootstrapping/keys.go`.
- Examples marked `-short` or comments about insecure parameters are for speed only. Production code must verify `LogQP` including `P` and any bootstrapping parameters. Sources: `examples/singleparty/ckks_bootstrapping/basics/main.go`, `schemes/ckks/README.md`.
- Bootstrapping parameters are not automatically guaranteed secure for your modified setting; the example explicitly assigns security checking to the user. Source: `examples/singleparty/ckks_bootstrapping/basics/main.go`.

## Example Walkthroughs With 7-Step Design Discipline

### 1. Basic template: `examples/singleparty/templates/ckks/main.go`

1. Function: encrypt a packed vector of real values in `[-1, 1]`, decrypt, decode, and report precision.
2. Arithmetic circuit: no homomorphic arithmetic; multiplicative depth 0.
3. Packing: `values := make([]float64, params.MaxSlots())`; default plaintext metadata is batched slots with `params.DefaultScale()` and max log dimensions.
4. Parameters: `LogN=14`, `LogQ={55, 45 x7}`, `LogP={61}`, `LogDefaultScale=45`; comments label it 128-bit secure with depth 7.
5. Keys: secret key only in the template; public key omitted because encryption uses the secret key. No relin or Galois keys needed.
6. Implementation: allocate `ckks.NewPlaintext(params, params.MaxLevel())`, `encoder.Encode`, `rlwe.NewEncryptor(params, sk).EncryptNew`, decrypt and decode.
7. Verification: `ckks.GetPrecisionStats(params, ecd, dec, have, want, 0, false)` after decrypt/decode. Source: `examples/singleparty/templates/ckks/main.go`.

### 2. Arithmetic, rotations, and linear transforms: `examples/singleparty/tutorials/ckks/main.go`

1. Function: demonstrate add, multiply, rescale, rotation, conjugation, polynomial evaluation, inner sums, rotate-and-add, replicate, and diagonal linear transforms on packed complex values.
2. Arithmetic circuit: additions depth 0; ct-ct and nontrivial ct-plain/scalar multiplications grow scale and need rescale; degree-63 Chebyshev polynomial uses depth budget after an initial change-of-basis level; linear transform is mostly rotations, plaintext multiplications, adds, and one final rescale.
3. Packing: standard CKKS with `LogSlots := params.LogMaxSlots()` and `Slots := 1 << LogSlots`; inner sum uses `(batch, n)` and requires `n*batch` constraints; diagonal matrix uses nonzero diagonal indices and BSGS ratio.
4. Parameters: `LogN=14`, `LogQ={55, 45 x7}`, `LogP={61}`, `LogDefaultScale=45`; tutorial checks `params.LogQP()` against the `LogN=14` 128-bit bound.
5. Keys: relin key initially; later update evaluator with Galois keys for rotation/conjugation, `params.GaloisElementsForInnerSum`, `params.GaloisElementsForReplicate`, and `lintrans.GaloisElements`.
6. Implementation: use `eval.AddNew`, `eval.MulRelinNew`, explicit `eval.Rescale`, `eval.RotateNew`, `eval.ConjugateNew`, `polynomial.NewEvaluator(params, eval).Evaluate`, `eval.InnerSum`, `eval.RotateAndAdd`, and `lintrans.NewEvaluator`.
7. Verification: compute plaintext `want` vectors slotwise and call `ckks.GetPrecisionStats`. Source: `examples/singleparty/tutorials/ckks/main.go`.

### 3. Nonlinear approximation: `examples/singleparty/ckks_sigmoid_minimax/main.go`

1. Function: approximate sigmoid over real inputs sampled in `[-25, 25]` under `ring.ConjugateInvariant`.
2. Arithmetic circuit: change basis to Chebyshev interval via scalar multiply/add/rescale, then evaluate a degree-63 minimax Chebyshev polynomial.
3. Packing: real slots from conjugate-invariant CKKS; `values := make([]float64, pt.Slots())`.
4. Parameters: `LogN=14`, `LogQ={55, 45 x7}`, `LogP={61}`, `LogDefaultScale=45`, `RingType=ring.ConjugateInvariant`.
5. Keys: relin key only; no rotations in this example.
6. Implementation: build minimax polynomial with `bignum.NewRemez`, wrap with `polynomial.NewPolynomial`, apply `poly.ChangeOfBasis()`, call `eval.Mul`, `eval.Add`, `eval.Rescale`, then `polynomial.NewEvaluator(params, eval).Evaluate(ct, poly, params.DefaultScale())`.
7. Verification: evaluate the same polynomial on plaintext values and print precision stats. Source: `examples/singleparty/ckks_sigmoid_minimax/main.go`.

### 4. Bootstrapping basics: `examples/singleparty/ckks_bootstrapping/basics/main.go`

1. Function: bootstrap level-0 CKKS ciphertexts to regain usable levels.
2. Arithmetic circuit: bootstrap circuit is `ScaleDown -> ModUp -> CoeffsToSlots -> EvalMod -> SlotsToCoeffs`; it refreshes modulus capacity but remains approximate.
3. Packing: standard CKKS complex slots with values in `[-1, 1]` for real and imaginary parts; bootstrapping `LogSlots` defaults to `LogN-1`.
4. Parameters: residual params use `LogN=16`, `LogQ={55, 40 x10}`, `LogP={61 x3}`, `LogDefaultScale=40`, `Xs=ring.Ternary{H:192}`. Bootstrapping params specify `LogN`, `LogP={61 x4}`, and matching `Xs`; comments warn security must be checked by the user.
5. Keys: residual key pair; bootstrap keys from `btpParams.GenEvaluationKeys(sk)`, including relin, bootstrap Galois keys, and optional switching/encapsulation keys.
6. Implementation: instantiate `bootstrapping.NewEvaluator(btpParams, evk)`, encrypt at level 0, call `eval.Bootstrap(ciphertext)`. Ensure scale is equal or very close to `params.DefaultScale()`; use `SetScale` if necessary and level is available.
7. Verification: decrypt before/after bootstrap, print level, logQ, scale, sample slots, and `ckks.GetPrecisionStats`. Source: `examples/singleparty/ckks_bootstrapping/basics/main.go`.

## Quick Budget Checklist For Codex-Generated Lattigo CKKS Code

- Use imports from `schemes/ckks`, `core/rlwe`, and `circuits/ckks/...`; do not assume `he/hefloat` for this local v6.2.0 tree.
- Start every design note with `LogN`, `LogQ`, `LogP`, `LogDefaultScale`, `RingType`, expected depth, and all rotations.
- Count `Rescale` explicitly. Both ciphertext-ciphertext multiplication and nontrivial plaintext/scalar/vector multiplication can require it in the CKKS scale schedule.
- Use `params.LevelsConsumedPerRescaling()` when `LogDefaultScale > 64`.
- Generate relin keys for ct-ct multiplication, polynomial evaluation, and bootstrap.
- Generate Galois keys from parameter helpers, not handwritten automorphism integers.
- For bootstrap, treat residual and bootstrapping parameters as separate security surfaces and include all bootstrap key material in memory/time estimates.
- Verify with plaintext reference, per-slot max abs/rel error or `ckks.GetPrecisionStats`, and inspect final `ct.Level()` and `ct.Scale.Log2()`.
