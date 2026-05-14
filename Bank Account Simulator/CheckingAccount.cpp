//
//  CheckingAccount.cpp
//  Bank Account
//
//  Created by Marco D’Eusebio on 4/27/26.
//

#include "CheckingAccount.h"
#include "Utilities.h"
#include <cmath>
#include <iostream>

CheckingAccount::CheckingAccount(
    std::string_view ownerName,
    double balance,
    double overdraftLimit,
    int accountNumber
                                )
    : Account(ownerName, balance, accountNumber, "Checking Account"),
      m_overdraftLimit(dollarsToCents(-std::abs(overdraftLimit))) {}

double CheckingAccount::getOverdraftLimit() const noexcept {
    return centsToDollars(m_overdraftLimit);
}

MoneyCents CheckingAccount::getOverdraftLimitInCents() const noexcept {
    return m_overdraftLimit;
}

TransactionResult CheckingAccount::withdraw(MoneyCents amountCents, bool allowOverdraft) {
    if (amountCents <= 0) {
        return TransactionResult::withBalance(
            TransactionStatus::invalidAmount,
            amountCents,
            m_balance,
            "The withdrawal could not be completed for the amount requested."
        );
    }

    if (quickWithdraw(amountCents)) {
        return TransactionResult::withBalance(
            TransactionStatus::success,
            amountCents,
            m_balance,
            "Withdrawal processed successfully."
        );
    }

    MoneyCents projectedBalance = 0;
    if (!trySubtractMoneyCents(m_balance, amountCents, projectedBalance)) {
        return TransactionResult::withBalance(
            TransactionStatus::overdraftLimitExceeded,
            amountCents,
            m_balance,
            "The amount requested exceeds the checking account overdraft limit."
        );
    }

    if (projectedBalance < getOverdraftLimitInCents()) {
        return TransactionResult::withBalance(
            TransactionStatus::overdraftLimitExceeded,
            amountCents,
            m_balance,
            "The amount requested exceeds the checking account overdraft limit."
        );
    }

    if (allowOverdraft) {
        m_balance = projectedBalance;
        return TransactionResult::withBalance(
            TransactionStatus::success,
            amountCents,
            m_balance,
            "Withdrawal processed using the available overdraft limit."
        );
    }

    return TransactionResult::withBalance(
        TransactionStatus::overdraftConfirmationRequired,
        amountCents,
        projectedBalance,
        "This withdrawal requires overdraft confirmation."
    );
}

void CheckingAccount::display() const {
    printDisplaySummary(std::cout);
    std::cout << " | Overdraft Limit: " << formatCents(getOverdraftLimitInCents()) << "\n";
}
