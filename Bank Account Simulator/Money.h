#ifndef MONEY_H
#define MONEY_H

#include <cmath>
#include <cstdint>
#include <limits>

using MoneyCents = std::int64_t;

constexpr MoneyCents kCentsPerDollar = 100;
constexpr MoneyCents kMinimumTransactionAmountCents = 1;

[[nodiscard]]
inline MoneyCents dollarsToCents(double dollars) noexcept {
    if (!std::isfinite(dollars)) {
        return 0;
    }

    constexpr double kMaxMoneyCentsAsDouble = static_cast<double>(std::numeric_limits<MoneyCents>::max());
    constexpr double kMinMoneyCentsAsDouble = static_cast<double>(std::numeric_limits<MoneyCents>::min());
    const double scaledCents = dollars * static_cast<double>(kCentsPerDollar);
    if (scaledCents >= kMaxMoneyCentsAsDouble) {
        return std::numeric_limits<MoneyCents>::max();
    }
    if (scaledCents <= kMinMoneyCentsAsDouble) {
        return std::numeric_limits<MoneyCents>::min();
    }

    return static_cast<MoneyCents>(std::llround(scaledCents));
}

[[nodiscard]]
inline double centsToDollars(MoneyCents cents) noexcept {
    return static_cast<double>(cents) / static_cast<double>(kCentsPerDollar);
}

[[nodiscard]]
inline MoneyCents calculatePercentageOfCents(MoneyCents amountCents, double rate) noexcept {
    if (!std::isfinite(rate)) {
        return 0;
    }

    const double scaledAmount = static_cast<double>(amountCents) * rate;
    constexpr double kMaxMoneyCentsAsDouble = static_cast<double>(std::numeric_limits<MoneyCents>::max());
    constexpr double kMinMoneyCentsAsDouble = static_cast<double>(std::numeric_limits<MoneyCents>::min());
    if (scaledAmount >= kMaxMoneyCentsAsDouble) {
        return std::numeric_limits<MoneyCents>::max();
    }
    if (scaledAmount <= kMinMoneyCentsAsDouble) {
        return std::numeric_limits<MoneyCents>::min();
    }

    return static_cast<MoneyCents>(std::llround(scaledAmount));
}

[[nodiscard]]
inline bool tryAddMoneyCents(MoneyCents lhs, MoneyCents rhs, MoneyCents& result) noexcept {
    if ((rhs > 0 && lhs > std::numeric_limits<MoneyCents>::max() - rhs)
        || (rhs < 0 && lhs < std::numeric_limits<MoneyCents>::min() - rhs)) {
        return false;
    }

    result = lhs + rhs;
    return true;
}

[[nodiscard]]
inline bool trySubtractMoneyCents(MoneyCents lhs, MoneyCents rhs, MoneyCents& result) noexcept {
    if (rhs < 0) {
        if (rhs == std::numeric_limits<MoneyCents>::min()) {
            return false;
        }

        return tryAddMoneyCents(lhs, -rhs, result);
    }

    if (lhs < std::numeric_limits<MoneyCents>::min() + rhs) {
        return false;
    }

    result = lhs - rhs;
    return true;
}

#endif
