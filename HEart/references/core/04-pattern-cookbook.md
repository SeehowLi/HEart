# CKKS Pattern Cookbook

Each recipe states when to use it, its dominant cost, required keys, packing/range assumptions, and pseudocode. Always instantiate these with an explicit scale/level schedule before coding.

Field order used below: `Applicable` | `Depth cost` | `Required keys` | `Packing/range assumptions` | `Pseudocode`.

## Rotate-And-Sum

- Applicable: reducing values across slots, e.g., summing a vector packed in one ciphertext. Depth cost: 0 multiplicative levels; `log2(n)` rotations/additions for power-of-two `n`. Required keys: rotations by `1, 2, 4, ..., n/2` or the library's equivalent generator set. Packing/range assumptions: active data occupies a power-of-two slot block or masked padding slots are zero. [2016-421]

```text
sum_slots(ct, n):
  for k in 1,2,4,...,n/2:
    ct = Add(ct, Rotate(ct, k))
  return ct
```

## Inner Product

- Applicable: dot products when vectors `x` and `y` share the same slot layout. Depth cost: one multiplication level for elementwise multiply, then rotate-and-sum at depth 0; plaintext-vector times ciphertext costs no relin but still needs rescale if the plaintext has CKKS scale. Required keys: relin key for ciphertext-ciphertext products; rotations for the reduction tree. Packing/range assumptions: both vectors are scale/level-aligned; unused slots are zero or masked. [2016-421][2020-1203]

```text
dot(ct_x, ct_y, n):
  prod = Mul(ct_x, ct_y)
  prod = Relinearize(prod)
  prod = Rescale(prod)
  return sum_slots(prod, n)
```

## Matrix-Vector: Diagonal Method + BSGS

- Applicable: dense linear transforms, convolutions, DFT-like maps, and bootstrap CoeffsToSlots/SlotsToCoeffs. Depth cost: usually 0 or 1 depending on whether diagonal plaintext multiplications are made scale-preserving; rotation count is `O(n)` naively and about `O(sqrt(n))` or `O(r log_r n)` with structured/BSGS methods. Required keys: rotations for baby and giant steps; hoisting support improves repeated rotations from the same ciphertext. Packing/range assumptions: matrix is encoded by diagonals matching the slot layout; diagonal plaintexts are pre-encoded at the right level/scale/domain. [2018-153][2018-1073][2020-1203]

```text
matvec_diagonal(ct, diagonals D[0..n-1]):
  acc = 0
  for k in nonzero_diagonals:
    acc = Add(acc, MulPlain(Rotate(ct, k), D[k]))
  return Rescale(acc)  // if diagonal plaintext scale increased ct scale

matvec_bsgs(ct, D, n1, n2):  // n = n1*n2
  pre = [Rotate(ct, i) for i in baby_steps(n1)]
  acc = 0
  for j in giant_steps(n2):
    block = linear_combination(pre, D[j,*])
    acc = Add(acc, Rotate(block, j*n1))
  return rescale_if_needed(acc)
```

## Polynomial Evaluation: Chebyshev + Paterson-Stockmeyer

- Applicable: smooth functions on a known interval and EvalMod/EvalSine polynomials inside bootstrapping. Depth cost: about `ceil(log2(degree+1))` in the BSGS/PS setting when scheduled carefully; full-RNS scale propagation avoids precision loss from adding different scales. Required keys: relin key; no rotations unless the polynomial is part of a linear-transform pipeline. Packing/range assumptions: input is affine-mapped into the approximation interval; Chebyshev basis is preferred for coefficient stability; all additions are between equal scales. [2016-421][2020-1203][2020-552][2022-280]

```text
eval_cheb_ps(ct_x, coeffs c[0..d], target_scale):
  T = precompute_chebyshev_basis(ct_x)       // T0=1,T1=x,T2=2x^2-1,...
  plan scales backward from target_scale
  recursively split p(x)=q(x)*T_m(x)+r(x)
  left  = eval_cheb_ps(q, planned_scale_left)
  right = eval_cheb_ps(r, planned_scale_right)
  out = Add(Rescale(Mul(left, T_m)), right)  // exact-scale add
  return out
```

## Sign / Comparison: Composite Minimax Polynomials

- Applicable: `a > b`, max, sorting, and piecewise control approximations when a CKKS-only path is required. Depth cost: sum of depths of component polynomials; minimax composite selection can reduce depth/non-scalar multiplications versus generic minimax or earlier composite choices. Required keys: relin key; rotations only if comparing many packed structures that need reductions/rearrangement. Packing/range assumptions: compare `x=a-b` after normalizing to a known interval; guarantee only outside a margin `|x| >= epsilon`; near-zero results are indeterminate. [2019-1234][2020-834][2021-1337]

```text
compare_ckks(ct_a, ct_b, poly_chain p[1..k], epsilon):
  x = Sub(ct_a, ct_b)
  // Caller must guarantee |x| >= epsilon for decided outputs.
  for i in 1..k:
    x = eval_polynomial_minimax_component(x, p[i])
  return MulConst(Add(x, 1), 0.5)
```

## ReLU, Sigmoid, Exp, and Saturating Activations

- Applicable: replacing ML nonlinearities by polynomials. Depth cost: polynomial degree/evaluation strategy dependent; exp/logistic examples in the original CKKS work used low-degree Taylor-style approximations, while large-interval sigmoid/tanh/capped-ReLU-like functions need explicit domain management. Required keys: relin key; rotations only for surrounding linear layers. Packing/range assumptions: input dynamic range is known; for sigmoid/logistic/tanh/ReLU variants over large or outlier-prone ranges, use domain-extension/outlier handling instead of blindly enlarging the minimax interval. [2016-421][2022-280]

```text
activation(ct_x, approx):
  ct_z = affine_map_to_interval(ct_x, approx.domain)
  if approx.uses_domain_extension:
    for dep in approx.domain_extension_chain:
      ct_z = eval_polynomial(ct_z, dep)
  return eval_polynomial(ct_z, approx.main_poly)
```

## Multiplicative Inverse and Inverse Square Root

- Applicable: normalization, division-like operations, reciprocal square-root normalization, and algorithms that can bound inputs away from zero. Depth cost: one or more multiplication levels per iteration/product stage; Newton/Goldschmidt variants trade extra parallel products for faster convergence. Required keys: relin key; rotations only for surrounding reductions. Packing/range assumptions: normalize `x` into a convergence interval before iteration; values near zero are unsafe, and every iteration must preserve scale/level alignment. CKKS original gives inverse-style approximate arithmetic and series/Taylor use cases; inverse-square-root recipes are numerical-analysis patterns that must be checked against the concrete HE paper/library before use. [2016-421][2019-1234]

```text
inverse_ckks(ct_x, p, r):
  // Assume x is encoded near p, set y = p - x and |y/p| <= 1/2.
  y = SubPlain(p, ct_x)
  v = AddPlain(y, p)
  power = y
  for j in 1..r-1:
    power = Rescale(Relinearize(Mul(power, power)))
    v = Rescale(Relinearize(Mul(v, AddPlain(power, p^(2^j)))))
  return v  // scaled inverse; caller rescales/normalizes as planned

newton_inverse(ct_x, ct_y0, iters):
  // y0 approximates 1/x on the declared interval.
  y = ct_y0
  for i in 1..iters:
    t = Rescale(MulPlain(ct_x, y))      // t ~= x*y
    y = Rescale(Relinearize(Mul(y, SubPlain(2, t))))
  return y

goldschmidt_inv_sqrt(ct_x, y0, iters):
  // y0 approximates 1/sqrt(x); constants depend on the normalized interval.
  y = y0
  h = Rescale(MulPlain(ct_x, y))
  h = Rescale(Relinearize(Mul(h, y)))   // h ~= x*y^2
  for i in 1..iters:
    r = SubPlain(1.5, MulConst(h, 0.5))
    y = Rescale(Relinearize(Mul(y, r)))
    h = Rescale(Relinearize(Mul(h, Rescale(Relinearize(Mul(r, r))))))
  return y
```

## Bootstrap Substeps

- Applicable: circuits needing more multiplicative depth than the initial chain can safely provide. Depth cost: many internal levels; EvalMod/EvalSine dominates multiplicative depth, while CoeffsToSlots/SlotsToCoeffs dominate rotations/key-switching cost. Required keys: bootstrap rotation set, conjugation key, relin/switching keys, and scheme-specific bootstrap setup. Packing/range assumptions: input scale, `Q0/Delta`, secret-key density, slot count, and approximation interval satisfy the bootstrap parameterization. [2018-153][2019-688][2020-1203][2020-552]

```text
bootstrap(ct):
  ct = ModRaise(ct)          // raise to large modulus
  ct = SubSum_if_sparse(ct)  // align coefficient ring when 2n != N
  slots = CoeffsToSlots(ct)  // linear transform, many rotations
  slots = EvalMod(slots)     // sine/cosine/minimax/arcsine approximation
  ct = SlotsToCoeffs(slots)  // inverse linear transform
  return ct                 // refreshed level/scale, approximate output
```

## Bootstrap Placement

- Applicable: circuit planning, not post-hoc debugging after implementation. Depth cost: bootstrap is expensive but may reduce total modulus and allow smaller chained segments. Required keys: full bootstrap key set at every planned bootstrap parameter set. Packing/range assumptions: the pre-bootstrap ciphertext still has enough precision and satisfies input range assumptions. [2018-153][2020-1203][2020-552]

```text
place_bootstrap(circuit):
  split circuit into segments by multiplicative depth
  for each segment:
    ensure segment_depth + safety_margin <= available_levels
    ensure input_range_to_bootstrap is bounded
    insert bootstrap before level/precision becomes invalid
  validate with plaintext reference before and after every bootstrap
```
