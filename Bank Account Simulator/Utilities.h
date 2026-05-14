#ifndef UTILITIES_H
#define UTILITIES_H

#include "Money.h"
#include <string>

[[nodiscard]]
int getValidatedInt(int minValue = 0);
[[nodiscard]]
MoneyCents getValidatedMoneyCents(MoneyCents minValue = 0);
[[nodiscard]]
bool isWholeDollarAmount(MoneyCents amountCents) noexcept;
[[nodiscard]]
std::string formatCents(MoneyCents amountCents);
[[nodiscard]]
std::string formatPercentage(double decimalValue, int decimalPlaces = 2);
[[nodiscard]]
int getValidatedChoice(int min, int max);

#endif
