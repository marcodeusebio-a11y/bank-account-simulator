//
//  SavingsAccount.cpp
//  Bank Account
//
//  Created by Marco D’Eusebio on 4/27/26.
//

#include "SavingsAccount.h"
#include "Utilities.h"
#include <algorithm>
#include <cmath>
#include <iostream>

SavingsAccount::SavingsAccount(
    std::string_view ownerName,
    double balance,
    double interestRate,
    int accountNumber
                               )
    : Account(ownerName, balance, accountNumber, "Savings Account"),
      m_interestRate(std::isfinite(interestRate) ? std::max(0.0, interestRate) : 0.0) {}

void SavingsAccount::addInterest() {
    const MoneyCents accruedInterest = getAccruedInterest();
    if (accruedInterest > 0) {
        MoneyCents updatedBalance = 0;
        if (tryAddMoneyCents(m_balance, accruedInterest, updatedBalance)) {
            m_balance = updatedBalance;
        } else {
            m_balance = std::numeric_limits<MoneyCents>::max();
        }
    }
}

double SavingsAccount::getInterestRate() const noexcept {
    return m_interestRate;
}

MoneyCents SavingsAccount::getAccruedInterest() const noexcept {
    return calculatePercentageOfCents(m_balance, m_interestRate);
}

void SavingsAccount::display() const {
    const MoneyCents accruedInterest = getAccruedInterest();

    printDisplaySummary(std::cout);
    std::cout << " | "
              << "Interest Rate: " << formatPercentage(m_interestRate) << " | "
              << "Accrued Interest: " << formatCents(accruedInterest) << "\n";
}
