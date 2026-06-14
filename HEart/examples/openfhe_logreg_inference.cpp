#include "openfhe.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>
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
