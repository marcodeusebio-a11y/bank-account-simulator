//
//  Utilities.cpp
//  Bank Account Simulator
//
//  Created by Marco D’Eusebio on 4/27/26.
//

#include "Utilities.h"
#include <cctype>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>

namespace {
[[nodiscard]]
std::string_view trimWhitespace(std::string_view text) {
    std::size_t start = 0;
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start])) != 0) {
        ++start;
    }

    std::size_t end = text.size();
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1])) != 0) {
        --end;
    }

    return text.substr(start, end - start);
}

[[nodiscard]]
bool tryParseMoneyLineExact(std::string_view inputLine, MoneyCents& amountCents) {
    const std::string_view trimmedInput = trimWhitespace(inputLine);
    if (trimmedInput.empty()) {
        return false;
    }

    std::size_t index = 0;
    bool isNegative = false;
    if (trimmedInput[index] == '+' || trimmedInput[index] == '-') {
        isNegative = trimmedInput[index] == '-';
        ++index;
    }

    std::uint64_t wholeDollars = 0;
    bool hasWholeNumberDigits = false;
    while (index < trimmedInput.size() && std::isdigit(static_cast<unsigned char>(trimmedInput[index])) != 0) {
        const std::uint64_t digit = static_cast<unsigned char>(trimmedInput[index]) - static_cast<unsigned char>('0');
        const std::uint64_t maxWholeDollars = (static_cast<std::uint64_t>(std::numeric_limits<MoneyCents>::max()) + 1U)
            / static_cast<std::uint64_t>(kCentsPerDollar);
        if (wholeDollars > ((maxWholeDollars - digit) / 10)) {
            return false;
        }

        wholeDollars = (wholeDollars * 10) + digit;
        hasWholeNumberDigits = true;
        ++index;
    }

    std::uint64_t fractionalCents = 0;
    std::size_t fractionalDigits = 0;
    if (index < trimmedInput.size() && trimmedInput[index] == '.') {
        ++index;
        while (index < trimmedInput.size() && std::isdigit(static_cast<unsigned char>(trimmedInput[index])) != 0) {
            if (fractionalDigits >= 2) {
                return false;
            }

            fractionalCents = (fractionalCents * 10) + (trimmedInput[index] - '0');
            ++fractionalDigits;
            ++index;
        }
    }

    const bool hasAnyDigits = hasWholeNumberDigits || fractionalDigits > 0;
    if (!hasAnyDigits || index != trimmedInput.size()) {
        return false;
    }

    if (fractionalDigits == 1) {
        fractionalCents *= 10;
    }

    const std::uint64_t maxAbsoluteCents = isNegative
        ? static_cast<std::uint64_t>(std::numeric_limits<MoneyCents>::max()) + 1U
        : static_cast<std::uint64_t>(std::numeric_limits<MoneyCents>::max());
    if (wholeDollars > (maxAbsoluteCents - fractionalCents) / static_cast<std::uint64_t>(kCentsPerDollar)) {
        return false;
    }

    const std::uint64_t absoluteCents = (wholeDollars * static_cast<std::uint64_t>(kCentsPerDollar)) + fractionalCents;
    if (isNegative) {
        if (absoluteCents == maxAbsoluteCents) {
            amountCents = std::numeric_limits<MoneyCents>::min();
        } else {
            amountCents = -static_cast<MoneyCents>(absoluteCents);
        }
    } else {
        amountCents = static_cast<MoneyCents>(absoluteCents);
    }

    return true;
}

void resetInputStreamState() {
    std::cin.clear();
}

void printInvalidEntryMessage() {
    std::cout << "Invalid entry.\n";
}

bool readInputLine(std::string& inputLine) {
    resetInputStreamState();
    return static_cast<bool>(std::getline(std::cin, inputLine));
}

bool containsOnlyTrailingWhitespace(std::istringstream& input) {
    input >> std::ws;
    return input.eof();
}

bool tryParseIntLine(const std::string& inputLine, int& value) {
    std::istringstream input(inputLine);
    return static_cast<bool>(input >> value) && containsOnlyTrailingWhitespace(input);
}

} // namespace

int getValidatedInt(int minValue) {
    std::string inputLine;

    while (true) {
        int choice = 0;
        if (readInputLine(inputLine) && tryParseIntLine(inputLine, choice) && choice >= minValue) {
            return choice;
        }

        printInvalidEntryMessage();
        std::cout << "Please enter an integer value of " << minValue << " or greater.\n";
    }
}

MoneyCents getValidatedMoneyCents(MoneyCents minValue) {
    std::string inputLine;

    while (true) {
        MoneyCents amountCents = 0;
        if (readInputLine(inputLine)
            && tryParseMoneyLineExact(inputLine, amountCents)) {
            if (amountCents >= minValue) {
                return amountCents;
            }
        }

        printInvalidEntryMessage();
        std::cout << "Please enter an amount greater than or equal to "
                  << formatCents(minValue) << ".\n";
    }
}

bool isWholeDollarAmount(MoneyCents amountCents) noexcept {
    return (amountCents % kCentsPerDollar) == 0;
}

std::string formatCents(MoneyCents amountCents) {
    std::ostringstream output;
    const bool isNegative = amountCents < 0;
    const std::uint64_t absoluteCents = isNegative
        ? static_cast<std::uint64_t>(-(amountCents + 1)) + 1U
        : static_cast<std::uint64_t>(amountCents);
    const std::uint64_t dollars = absoluteCents / static_cast<std::uint64_t>(kCentsPerDollar);
    const std::uint64_t cents = absoluteCents % static_cast<std::uint64_t>(kCentsPerDollar);

    output << (isNegative ? "-$" : "$")
           << dollars
           << '.'
           << std::setw(2)
           << std::setfill('0')
           << cents;
    return output.str();
}

std::string formatPercentage(double decimalValue, int decimalPlaces) {
    std::ostringstream output;
    output << std::fixed << std::setprecision(decimalPlaces) << (decimalValue * 100.0) << "%";
    return output.str();
}

int getValidatedChoice(int min, int max) {
    std::string inputLine;

    while (true) {
        int choice = 0;
        if (readInputLine(inputLine)
            && tryParseIntLine(inputLine, choice)
            && choice >= min
            && choice <= max) {
            return choice;
        }

        printInvalidEntryMessage();
        if ((max - min) == 1) {
            std::cout << "Please enter " << min
                      << " or " << max << ".\n";
        } else {
            std::cout << "Please enter a number between "
                      << min << " and " << max << ".\n";
        }
    }
}
