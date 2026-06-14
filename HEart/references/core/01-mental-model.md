# CKKS Mental Model

CKKS is approximate homomorphic arithmetic over real/complex vectors: think of each ciphertext as an encrypted fixed-point vector with a tracked scale, a remaining modulus-chain budget, and an error budget. It is not exact integer HE with a cosmetic decoder. [2016-421]

## Algebraic Object

- Work in the negacyclic ring `R_q = Z_q[X]/(X^N + 1)` for power-of-two `N`; ciphertext polynomials are reduced both by `X^N + 1` and by the active coefficient modulus. [2018-931]
- A CKKS plaintext is a ring element whose canonical embedding `sigma` evaluates it at complex roots; decoding projects conjugate-symmetric coordinates to `C^(N/2)`. Operational invariant: one ciphertext carries at most `N/2` complex SIMD slots. [2016-421]
- The discarded half of the embedding is determined by complex conjugacy. For real-only workloads, treat nonzero imaginary parts after decoding as semantic contamination: symptoms are small imaginary residuals or real outputs drifting after repeated rotations/conjugations; fix by using a real-aware packing convention, explicit conjugation/real extraction when required, and slot-wise error checks on both real and imaginary components. Do not describe this as a security leak without an explicit source. [2016-421][2018-153][需人工确认]
- Slot permutations come from Galois automorphisms of the ring; after an automorphism, key switching is required to return the ciphertext to the original secret key. Rotation and conjugation are therefore key-switch operations, not free metadata changes. [2016-421][2020-1203]

## Ciphertext Structure

- Fresh and relinearized CKKS ciphertexts are two-component objects `(c0, c1)` that decrypt under secret `s` by computing `c0 + c1*s`; equivalently, think `c0 = -c1*s + Delta*m + e`, so decryption returns approximately `Delta*m` plus error. [2016-421][2020-1203]
- A ciphertext-ciphertext multiplication first produces a degree-2 object `(c0, c1, c2)` that decrypts in the basis `(1, s, s^2)`: the third component is why the result is an "extended ciphertext" in operation manuals. [2020-1203]
- Relinearization is key switching from the `s^2` component back to the ordinary secret-key basis `(1, s)`, producing a normal `(c0, c1)` ciphertext again. This is the reason `Mul` is usually budgeted as the bundle `Mul -> Relinearize -> Rescale`, and why multiplying again before relinearization requires explicit lazy-relinearization support. [2020-1203]

## The Three Quantities To Track

- `scale Delta`: the fixed-point denominator. Encoding multiplies an approximate real/complex value by `Delta` before rounding into the ring. Larger `Delta` gives more initial precision but consumes modulus bits and makes scale alignment harder. [2016-421]
- `level`: the number of remaining primes in the active modulus chain. At level `l`, the modulus is `Q_l = product_{i=0}^l q_i`; each rescale normally removes the top prime and spends one unit of multiplicative-depth budget. Level numbering conventions differ across libraries; this document is internally consistent, and library-specific mappings belong in `lib-*.md`. [2018-931]
- `noise/error`: CKKS deliberately treats encryption noise, encoding rounding, rescale rounding, key-switch error, and approximation error as one numerical error budget. Correctness means `error / scale` stays below the requested tolerance, not that decrypted values equal exact plaintexts. [2016-421]
- Multiplication multiplies scales and grows error; rescale divides the ciphertext by a chain prime, drops a level, and brings the scale back near the intended working scale. The design invariant is usually `Mul -> Relinearize -> Rescale`, then continue with similar scales. [2016-421][2018-931][2020-1203]
- Addition is cheap only when level and scale are compatible. If scales differ, implementations may scale the smaller operand or require manual alignment; if levels differ, the operation is performed at the shared smaller modulus, discarding unavailable primes. [2020-1203]

## RNS / Double-CRT View

- Full-RNS CKKS represents the active modulus as an RNS basis `{q_0, ..., q_l}` and stores each polynomial by residues modulo these word-size primes. This avoids big-integer arithmetic in the hot path. [2018-931]
- The "prime chain" is not arbitrary bookkeeping: it is the sequence of moduli that will be consumed by rescale/modulus management. Choose enough primes for the circuit depth, bootstrapping plan, precision, and security target. [2018-931]
- NTT-friendly primes satisfy `q_i = 1 mod 2N`, allowing efficient negacyclic polynomial multiplication by NTT on each RNS limb. Full-RNS implementations are designed so arithmetic can stay in RNS and NTT form as much as possible. [2018-931]
- Key switching and modulus conversion need basis-extension/reduction machinery. Hybrid key switching exists to balance BV-style digit decomposition against GHS-style modulus extension: fewer digits and smaller keys reduce memory/time, but the auxiliary modulus and decomposition choices affect noise and security. [2012-099][2021-204]
- Engineering invariant: do not reason about a ciphertext as a single big integer polynomial when implementing performance-sensitive CKKS. Reason as `(level, scale, RNS limbs, NTT state, key material)` plus the mathematical slot vector it approximates. [2018-931][2020-1203]

## Security Co-Design

- Approximate decryption is a security boundary. Li-Micciancio show passive key-recovery attacks from approximate CKKS decryption results, with experiments against HEAAN, SEAL, HElib, PALISADE, and later Lattigo; the indexed source is ePrint `2020/1533`, EUROCRYPT 2021 venue still marked in the index as `[需人工确认]`. Rule: never return raw CKKS decryption output to an untrusted party; use noise flooding before exposure; include "who can see decryptions" in the threat model. This is distinct from imaginary-part contamination, which is a numerical/semantic issue, not the same attack surface. [2020-1533][openfhe-development-1.5.1/openfhe-development-1.5.1/src/pke/include/scheme/gen-cryptocontext-params.h]
- Security has a hard modulus budget. For fixed ring dimension `N`, the total active modulus size must remain below the target security upper bound; OpenFHE's parameter comments state that BV security depends on `Q`, HYBRID security depends on `P*Q`, and the selected security level gives an upper bound on the highest modulus. Decision rule: if the design needs more depth than the `N`-fixed modulus budget permits, increase `N`, reduce the circuit depth, or bootstrap. Detailed parameter tables belong in `05-security-parameters.md`. [openfhe-development-1.5.1/openfhe-development-1.5.1/src/pke/include/scheme/gen-cryptocontext-params.h][2020-1533][需人工确认 HE-standard/lattice-estimator citation]

## Approximate Correctness

- CKKS decryption returns an approximate vector. Validation must compare decoded slots against a plaintext reference with `max_abs_error`, `max_rel_error`, and, for complex packing, separate real/imaginary diagnostics. Exact equality is the wrong acceptance test. [2016-421]
- A result can be mathematically correct but operationally unusable if its remaining level is too low for the next multiplication, its scale is incompatible with the next operand, or its imaginary residual is large for a real-only pipeline. Track these as first-class state, not as comments after the fact. [2016-421][2020-1203]
- Bootstrapping refreshes a low-level ciphertext by approximately evaluating a modular reduction/decryption-related circuit, returning a ciphertext with a larger usable modulus budget. It replaces one error profile with bootstrapping approximation error; it does not make the ciphertext exact. [2018-153][2020-1203][2020-552]
- Decision rule: before writing CKKS code, write down `N`, slots, scale schedule, modulus chain, required levels, all rotations, relin/conjugation/bootstrap keys, and the final error tolerance. If any entry is unknown, the design is not ready for implementation. [2016-421][2018-931][2020-1203]

## Common Failure Modes

- Scale drift: additions fail or silently lose precision because operands have different scales. Fix by rescaling/mod-switching according to the library's alignment rules and keeping an explicit scale schedule. [2020-1203]
- Level starvation: a circuit decrypts correctly midway but cannot evaluate the last multiplication. Fix by adding chain primes, lowering polynomial depth, changing evaluation strategy, or inserting bootstrapping earlier. [2016-421][2018-153]
- Rotation-key miss: code reaches a linear transform and fails because the Galois key for a required step was not generated. Fix by enumerating all rotation steps from the packing layout before key generation. [2018-153][2020-1203]
- Bootstrap precision loss: the refreshed ciphertext has more depth but fewer reliable bits than expected. Fix by checking the bootstrap scaling factor, approximation interval, polynomial degree, and post-bootstrap error empirically against a plaintext reference. [2018-153][2020-552]
