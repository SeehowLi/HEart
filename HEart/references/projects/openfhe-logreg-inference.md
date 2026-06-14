# OpenFHE CKKS Logistic Regression Inference

中文摘要: This note gives a practical OpenFHE 1.5.1 / CPU / C++ pattern for encrypted-vector logistic regression inference. The feature vector is encrypted, the model weights and bias are plaintext, and the output probability remains encrypted. The example uses CKKS approximate arithmetic and validates the decrypted result against a plaintext reference.

## Design Note

Target: OpenFHE 1.5.1, CKKS, CPU, C++.

1. Function and ranges
   - Function: `f(x) = sigmoid(<w, x> + b)`.
   - Input shape: one encrypted real vector `x` of dimension `d`.
   - Model shape: plaintext real vector `w` of dimension `d`, plaintext scalar bias `b`.
   - Output shape: one encrypted probability replicated in the active slots after `EvalSum`.
   - Example default: `d = 8`.
   - Range contract: features and weights must be normalized so the logit `z = <w, x> + b` lies inside `[-5, 5]`. The real production range must be checked from the trained model and data distribution [needs manual confirmation].

2. Arithmetic circuit
   - Elementwise weighted features: `ct_weighted = EvalMult(ct_x, pt_w)`.
   - Dot product: `ct_dot = EvalSum(ct_weighted, d)`.
   - Bias: `ct_logit = EvalAdd(ct_dot, b)`.
   - Sigmoid: `ct_prob = EvalLogistic(ct_logit, -5, 5, 16)`.
   - Multiplicative depth budget: plaintext-ciphertext multiply uses about 1 level; OpenFHE's degree-16 logistic example budgets 6 levels; total `multiplicativeDepth = 7`.
   - Total high-cost operations: one plaintext-ciphertext multiply, one slot-sum rotation tree, and one degree-16 Chebyshev logistic approximation.
   - No ciphertext-dependent branch is used.

3. Packing layout and rotations
   - Put `x[0..d-1]` in CKKS slots `0..d-1`; pad the rest with zero.
   - Encode `w[0..d-1]` in the same slots; pad the rest with zero.
   - `EvalSum(ct, d)` reduces the first `d` slots. For `d = 8`, the equivalent rotation steps are `{1, 2, 4}`.
   - If `d` is not a power of two, pad to the next power of two with zero-valued features and weights.

4. Parameters and scale/level schedule
   - `SetSecurityLevel(HEStd_128_classic)`.
   - `SetMultiplicativeDepth(7)`.
   - `SetScalingModSize(50)`.
   - `SetFirstModSize(60)`.
   - `SetBatchSize(8)` for this example.
   - Let OpenFHE select the ring dimension `N` from the security target and modulus budget; require `d <= slots <= N/2`.
   - Scale management: use OpenFHE AUTO scaling in normal `Eval*` calls. Do not manually call `Rescale` in this code path.
   - Schedule at the design level:
     - `ct_x`: fresh ciphertext, level 0 in OpenFHE's consumed-level convention, working scale near `2^50`.
     - `ct_weighted`: after `EvalMult(ct_x, pt_w)`, OpenFHE internally adjusts scale/level as needed.
     - `ct_dot`: after `EvalSum`, scale/level are preserved except key-switching noise.
     - `ct_logit`: after adding plaintext bias, scale/level are aligned by OpenFHE.
     - `ct_prob`: after `EvalLogistic`, about 6 additional multiplicative levels are consumed internally by the polynomial evaluator.
   - Bootstrapping: not used; this is a leveled circuit.

5. Keys
   - PKE key pair: `KeyGen()`.
   - Multiplication/evaluation key: `EvalMultKeyGen(secretKey)`.
   - Slot-sum keys: `EvalSumKeyGen(secretKey)`.
   - Bootstrap keys: none.

## C++ Example

Save as a standalone OpenFHE example source file such as `openfhe_logreg_inference.cpp`.

```cpp
#include "openfhe.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

using namespace lbcrypto;

namespace {

double Sigmoid(double z) {
    return 1.0 / (1.0 + std::exp(-z));
}

double Dot(const std::vector<double>& a, const std::vector<double>& b) {
    double acc = 0.0;
    for (size_t i = 0; i < a.size(); ++i) {
        acc += a[i] * b[i];
    }
    return acc;
}

void PrintCtState(const char* name, const Ciphertext<DCRTPoly>& ct) {
    std::cout << name << ": level=" << ct->GetLevel()
              << ", noiseScaleDeg=" << ct->GetNoiseScaleDeg()
              << ", scale=" << std::setprecision(6) << ct->GetScalingFactor()
              << std::endl;
}

void ReportError(const std::vector<std::complex<double>>& got,
                 double expected,
                 size_t slotsToCheck) {
    double maxAbs = 0.0;
    double maxRel = 0.0;
    double maxImag = 0.0;
    const double denom = std::max(std::abs(expected), 1e-12);

    for (size_t i = 0; i < slotsToCheck; ++i) {
        const double absErr = std::abs(got[i].real() - expected);
        const double relErr = absErr / denom;
        maxAbs = std::max(maxAbs, absErr);
        maxRel = std::max(maxRel, relErr);
        maxImag = std::max(maxImag, std::abs(got[i].imag()));
    }

    std::cout << "max_abs_error=" << maxAbs << std::endl;
    std::cout << "max_rel_error=" << maxRel << std::endl;
    std::cout << "max_imag_abs=" << maxImag << std::endl;
}

}  // namespace

int main() {
    constexpr uint32_t featureDim = 8;
    constexpr uint32_t batchSize = 8;
    constexpr uint32_t multDepth = 7;
    constexpr uint32_t scaleModSize = 50;
    constexpr uint32_t firstModSize = 60;
    constexpr uint32_t logisticDegree = 16;
    constexpr double logisticLower = -5.0;
    constexpr double logisticUpper = 5.0;

    std::vector<double> x = {
        0.20, -0.10, 0.40, 0.70, -0.30, 0.05, 0.90, -0.60,
    };
    std::vector<double> w = {
        0.80, -0.40, 0.25, 0.30, -0.50, 0.10, 0.20, -0.15,
    };
    const double bias = -0.05;

    const double plainLogit = Dot(w, x) + bias;
    if (plainLogit < logisticLower || plainLogit > logisticUpper) {
        std::cerr << "Plaintext logit " << plainLogit
                  << " is outside the logistic approximation interval ["
                  << logisticLower << ", " << logisticUpper << "]."
                  << std::endl;
        return 1;
    }
    const double plainProb = Sigmoid(plainLogit);

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetSecurityLevel(HEStd_128_classic);
    parameters.SetMultiplicativeDepth(multDepth);
    parameters.SetScalingModSize(scaleModSize);
    parameters.SetFirstModSize(firstModSize);
    parameters.SetBatchSize(batchSize);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    std::cout << "Ring dimension N=" << cc->GetRingDimension()
              << ", slots=" << batchSize << std::endl;

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalSumKeyGen(keys.secretKey);

    Plaintext ptX = cc->MakeCKKSPackedPlaintext(x);
    Plaintext ptW = cc->MakeCKKSPackedPlaintext(w);
    auto ctX = cc->Encrypt(keys.publicKey, ptX);
    PrintCtState("ct_x", ctX);

    // ct_weighted ~= Enc([x_i * w_i]); OpenFHE AUTO scaling manages rescale.
    auto ctWeighted = cc->EvalMult(ctX, ptW);
    PrintCtState("ct_weighted", ctWeighted);

    // ct_dot has the dot product replicated by EvalSum semantics.
    auto ctDot = cc->EvalSum(ctWeighted, featureDim);
    PrintCtState("ct_dot", ctDot);

    auto ctLogit = cc->EvalAdd(ctDot, bias);
    PrintCtState("ct_logit", ctLogit);

    // Approximate sigmoid on the declared logit interval.
    auto ctProb = cc->EvalLogistic(ctLogit, logisticLower, logisticUpper, logisticDegree);
    PrintCtState("ct_prob", ctProb);

    Plaintext decProb;
    cc->Decrypt(keys.secretKey, ctProb, &decProb);
    decProb->SetLength(featureDim);
    auto got = decProb->GetCKKSPackedValue();

    std::cout << std::setprecision(12);
    std::cout << "plaintext_logit=" << plainLogit << std::endl;
    std::cout << "plaintext_probability=" << plainProb << std::endl;
    std::cout << "decrypted_probability_slot0=" << got[0].real() << std::endl;
    ReportError(got, plainProb, featureDim);

    return 0;
}
```

## Build Notes

- Build it inside an OpenFHE 1.5.1 CMake project or add it under OpenFHE's `src/pke/examples/` following the existing example style.
- Keep `HEStd_128_classic` for production-oriented tests. Do not copy examples that use `HEStd_NotSet` unless you run and record a lattice-estimator check.
- For a different feature dimension, change `featureDim`, `batchSize`, `x`, and `w` together; keep `featureDim <= batchSize` and prefer power-of-two padding.

## Operational Pitfalls

- The sigmoid approximation is only controlled on the declared interval. If `z` leaves `[-5,5]`, the polynomial may produce poor probabilities; widen the interval or retrain/normalize to bound logits.
- `EvalSum` needs generated sum/rotation keys. If key generation is skipped, inference fails at runtime.
- CKKS output is approximate. Always compare against a plaintext reference and report max absolute, relative, and imaginary errors.
- Do not return raw CKKS decryptions to an untrusted party; if decrypted outputs cross a trust boundary, model approximate-decryption leakage and add a documented mitigation such as noise flooding.

## Sources

- `HEart/references/core/01-mental-model.md`
- `HEart/references/core/02-op-semantics.md`
- `HEart/references/core/03-invariants-gotchas.md`
- `HEart/references/core/04-pattern-cookbook.md`
- `HEart/references/core/05-params-playbook.md`
- `HEart/references/libs/openfhe.md`
- `openfhe-development-1.5.1/openfhe-development-1.5.1/src/pke/examples/function-evaluation.cpp`
- `openfhe-development-1.5.1/openfhe-development-1.5.1/src/pke/examples/inner-product.cpp`
