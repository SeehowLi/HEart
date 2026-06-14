<!--
 * @Author: SeehowLi lsh0126@nudt.edu.cn
 * @Date: 2026-06-13 21:29:25
 * @LastEditors: SeehowLi lsh0126@nudt.edu.cn
 * @LastEditTime: 2026-06-13 21:39:46
 * @FilePath: \HEart研发\HEart\references\core\05-params-playbook.md
 * @Description: 
 * 
 * Copyright (c) 2026 by $SeehowLi lsh0126@nudt.edu.cn, All Rights Reserved. 
-->
# CKKS Parameter Playbook

This is a decision tree for choosing CKKS parameters. It is intentionally conservative: start from the application contract, then derive the modulus/scale/depth budget, then check security.

## Inputs First

- Define the function `f`, input/output shapes, slot count, real/complex convention, and dynamic range for every nonlinear input. If a comparison/sign is used, define the undecidable margin around zero. [2016-421][2019-1234][2022-280]
- Compute the arithmetic circuit: number of ciphertext-ciphertext multiplications, polynomial degrees, rescale count, rotations, and whether bootstrapping appears inside the circuit. [2016-421][2020-1203]
- Set a target output error: absolute error, relative error, or significant bits. CKKS correctness is approximate; parameters should be judged by error tolerance, not exact equality. [2016-421][2022-816]

## Decision Tree

1. Choose slots and `N`.
   - Need `slots <= N/2`; `N` is a power of two for the standard CKKS ring `Z_Q[X]/(X^N+1)`. Larger `N` gives more slots and a larger security modulus budget, but every NTT, key, ciphertext, and rotation gets more expensive. [2016-421][2018-931]

2. Choose the initial working scale `Delta`.
   - Rule of thumb: `log2(Delta)` should cover desired precision plus headroom for approximation/rescale/key-switch errors. Larger `Delta` improves numerical precision but consumes one similarly sized scaling prime per multiplication level. [2016-421][2018-931]

3. Estimate multiplicative levels.
   - Count one level for each normal `Mul -> Relin -> Rescale` and for each plaintext-ciphertext multiply that uses a scaled plaintext and then rescale. Polynomial evaluation should use Chebyshev/Paterson-Stockmeyer or scale-propagated BSGS to minimize levels. [2016-421][2020-1203]

4. Build the coefficient-modulus chain `Q`.
   - Use a chain of RNS primes `q_i` with `q_i = 1 mod 2N` for NTT support. Scaling primes are usually near `log2(Delta)` bits so rescale brings the scale back near the working scale; include first/last primes according to the library's CKKS convention and precision needs. [2018-931][2020-1203]

5. Add special primes / auxiliary modulus `P`.
   - Key switching, relinearization, rotations, and hybrid methods use auxiliary/special primes. Security and memory must account for `Q*P` when the key-switching method extends the modulus. [2018-931][2021-204][2022-915]

6. Check the fixed-`N` security ceiling.
   - For a fixed ring dimension `N`, there is an upper bound on the total modulus size compatible with a target security level. If `log(Q*P)` exceeds the 128-bit security budget, do not "just add one more prime"; increase `N`, reduce depth/scale, change the polynomial strategy, or bootstrap. [2016-421][2018-931][2022-915]

7. Decide whether to bootstrap.
   - Bootstrap when required depth exceeds the safe modulus chain, when long iterative circuits would otherwise force too large `N`, or when a reusable pipeline benefits from refreshed capacity. Avoid bootstrap if a leveled circuit fits the security budget and the bootstrap approximation/key cost dominates. [2018-153][2020-1203][2020-552]

8. Decide where to bootstrap.
   - Place bootstrap before level and precision are exhausted, not after. The input must satisfy bootstrap assumptions: scale, `Q0/Delta`, slot count, secret-key-density assumptions, and EvalMod interval. [2018-153][2020-1203][2020-552]

9. Allocate bootstrap internal primes if used.
   - Budget levels for CoeffsToSlots, EvalMod/EvalSine, and SlotsToCoeffs. Tune sine/cosine/minimax/arcsine degrees to meet output precision; allocate linear-transform moduli so their errors do not dominate. [2020-1203][2020-552]

10. Generate keys from the final layout.
    - Required keys normally include relin, all Galois rotations, conjugation if complex/real extraction or bootstrap needs it, and bootstrap keys if used. Generate only after the packing layout and BSGS steps are fixed. [2018-153][2018-1073][2020-1203]

11. Validate empirically.
    - Run a plaintext reference, decrypt slot-wise, and report max absolute/relative error. For real-only workloads, also report max imaginary magnitude. For exposed decryptions, validate after noise flooding, not before. [2016-421][2020-1533][2022-816]

## Chain Sketch

- Leveled non-bootstrap CKKS: `Q = q_L * ... * q_1 * q_0`, with most middle `q_i` near `Delta` bit-size and enough levels for the circuit. [2018-931]
- Hybrid/key-switch-aware CKKS: reason about the largest modulus as `Q*P` during key switching; special primes `P` reduce key-switch error/cost tradeoffs but consume security budget. [2018-931][2021-204]
- Bootstrapped CKKS: split `Q` into pre-bootstrap capacity, bootstrap internal levels, and post-bootstrap residual capacity. The post-bootstrap level and output scale are design targets, not automatic constants. [2020-1203][2020-552]

## Practical Rules

- If precision fails but security has room: increase `Delta` or add precision primes, then re-check `log(Q*P)`. [2016-421][2018-931]
- If depth fails but precision is fine: use lower-depth polynomial evaluation, reduce degree, insert bootstrap, or increase `N`. [2020-1203][2022-280]
- If security fails: increasing `Q` is not an option at fixed `N`; increase `N`, reduce levels, reduce special-prime budget, or bootstrap. [2018-931][2022-915]
- If runtime/key memory fails: reduce rotations with BSGS/hoisting, reduce slots if possible, use sparse packing, or simplify linear transforms. [2018-1073][2020-1203]
- If comparison fails near equality: increase the comparison margin or treat near-zero cases as ties; do not expect polynomial sign approximation to resolve discontinuity at zero. [2019-1234][2020-834]

## 128-bit Security Check With a Lattice Estimator

1. Collect the exact RLWE/RNS parameters: `N`, secret distribution, error distribution, all `q_i` in the active chain, and all special primes `p_i` used in key switching or bootstrap. Include `P` when the method's security depends on `Q*P`. [2018-931][2021-204][2022-915]
2. Compute `log2(Q)` for pure ciphertext modulus and `log2(Q*P)` for hybrid/key-switch-extended settings. Use the larger modulus seen by the relevant RLWE samples. [2016-421][2021-204]
3. Run a current lattice estimator or the library's estimator wrapper; OpenFHE points users to `openfhe-lattice-estimator`, while earlier CKKS/RNS work used Albrecht-style LWE estimates. Exact estimator command lines are environment-specific and must be recorded in project notes. [2014-062][2018-931][2022-915]
4. Accept parameters only if the reported classical/quantum security meets the project target, normally at least 128-bit classical unless the project explicitly states otherwise. If not, revise `N`, `Q`, `P`, depth, or bootstrap placement. [2022-816][2022-915]

