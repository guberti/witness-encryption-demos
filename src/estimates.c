/*
Code for estimating n, the number of primes p_i to use.  Mostly adapted from the
SAGE code provided by Tancrède Lepoint in his thesis (Figures 7.2 and 7.3).
*/

#include "estimates.h"
#include "utils.h"

#include <math.h>
#include <stdbool.h>

static double
time_LLL(double m, double gamma)
{
    return log2(0.06 * pow(m, 4) * gamma);
}

static double
time_BKZ20(double m, double gamma)
{
    return log2(0.36 * pow(m, 4.2) * gamma);
}

static double
m_min(size_t lambda, size_t eta, double gamma, double hermite_factor)
{
    double lh, result;
    lh = log2(hermite_factor);
    result = (eta - sqrt(eta * eta - 4 * lh * (gamma - lambda)))/(2 * lh);
    if (result > 0)
        return ceil(result);
    else
        return pow(2, lambda);
}

static double
gamma_from_orthogonal_attack(size_t lambda, size_t eta, bool conservative)
{
    double gamma = ceil(lambda + eta * eta / 4 / log2(1.012));
    if (!conservative) {
        while (gamma > 1) {
            double m1, m2;
            gamma /= 1.1;
            m1 = time_LLL(m_min(lambda, eta, gamma, 1.021), gamma);
            m2 = time_BKZ20(m_min(lambda, eta, gamma, 1.013), gamma);
            if (min(m1, m2) < lambda) {
                gamma *= 1.1;
                break;
            }
        }
    }
    return gamma;
}

static double
gamma_from_orthogonal_attack_2(size_t lambda, size_t eta, double hermite_factor,
                               bool conservative)
{
    double gamma = ceil(lambda + eta * eta / 4 / log2(hermite_factor));
    if (!conservative) {
        while (gamma > 1) {
            double m;
            gamma /= 1.1;
            m = time_LLL(m_min(lambda, eta, gamma, hermite_factor), gamma);
            if (m < lambda) {
                gamma *= 1.1;
                break;
            }
        }
    }
    return gamma;
}

size_t
estimate_n(size_t lambda, size_t eta, size_t flags)
{
    bool conservative = flags & CLT_FLAG_SEC_CONSERVATIVE;
    double gamma;
    if (flags & CLT_FLAG_SEC_IMPROVED_BKZ) {
        gamma = gamma_from_orthogonal_attack_2(lambda, eta, 1.005, conservative);
    } else {
        gamma = gamma_from_orthogonal_attack(lambda, eta, conservative);
    }
    return (size_t) ceil(gamma / eta);
}
