//
//  Account.cpp
//  Bank Account
//
//  Created by Marco D’Eusebio on 4/27/26.
//

#include "Account.h"
#include "Utilities.h"
#include <iostream>
#include <utility>

int Account::s_nextAccountNumber = 1001;

Account::Account(std::string_view ownerName, double balance, int accountID, std::string accountType)
    : m_accountNumber(accountID > 0 ? accountID : s_nextAccountNumber++),
      m_ownerName(ownerName),
      m_accountType(std::move(accountType)),
      m_balance(dollarsToCents(balance)) {
    if (accountID > 0) {
        if (accountID >= s_nextAccountNumber) {
            s_nextAccountNumber = accountID + 1;
        }
    }
}

bool Account::quickDeposit(MoneyCents amountCents) noexcept {
    if (amountCents > 0) {
        MoneyCents updatedBalance = 0;
        if (tryAddMoneyCents(m_balance, amountCents, updatedBalance)) {
            m_balance = updatedBalance;
            return true;
        }
    }
    
    return false;
}

bool Account::quickWithdraw(MoneyCents amountCents) noexcept {
    if (amountCents > 0 && m_balance >= amountCents) {
        MoneyCents updatedBalance = 0;
        if (trySubtractMoneyCents(m_balance, amountCents, updatedBalance)) {
            m_balance = updatedBalance;
            return true;
        }
    }
    
    return false;
}

TransactionResult Account::deposit(MoneyCents amountCents) {
    if (amountCents <= 0) {
        return TransactionResult::withBalance(
            TransactionStatus::invalidAmount,
            amountCents,
            m_balance,
            "The deposit could not be completed with the amount provided."
        );
    }

    if (quickDeposit(amountCents)) {
        return TransactionResult::withBalance(
            TransactionStatus::success,
            amountCents,
            m_balance,
            "Deposit processed successfully."
        );
    }

    return TransactionResult::withBalance(
        TransactionStatus::invalidAmount,
        amountCents,
        m_balance,
        "The deposit would exceed the maximum supported account balance."
    );
}

TransactionResult Account::withdraw(MoneyCents amountCents, bool allowOverdraft) {
    (void)allowOverdraft;
    if (quickWithdraw(amountCents)) {
        return TransactionResult::withBalance(
            TransactionStatus::success,
            amountCents,
            m_balance,
            "Withdrawal processed successfully."
        );
    }

    if (m_balance < amountCents) {
        return TransactionResult::withBalance(
            TransactionStatus::insufficientFunds,
            amountCents,
            m_balance,
            "The amount requested surpasses the account balance."
        );
    }

    return TransactionResult::withBalance(
        TransactionStatus::invalidAmount,
        amountCents,
        m_balance,
        "The withdrawal could not be completed for the amount requested."
    );
}

const std::string& Account::getAccountType() const noexcept {
    return m_accountType;
}

int Account::getAccountNumber() const noexcept {
    return m_accountNumber;
}

double Account::getBalance() const noexcept {
    return centsToDollars(m_balance);
}

MoneyCents Account::getBalanceInCents() const noexcept {
    return m_balance;
}

const std::string& Account::getOwnerName() const noexcept {
    return m_ownerName;
}

void Account::printDisplaySummary(std::ostream& output) const {
    output << "Account #" << m_accountNumber << " | "
           << "Type: " << getAccountType() << " | "
           << "Owner: " << m_ownerName << " | "
           << "Balance: " << formatCents(m_balance);
}

void Account::display() const {
    printDisplaySummary(std::cout);
    std::cout << "\n";
}
