//
//  Account.h
//  Bank Account
//
//  Created by Marco D’Eusebio on 4/27/26.
//

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "Money.h"
#include <iosfwd>
#include <string>
#include <string_view>
#include <utility>

enum class TransactionStatus {
    success,
    invalidAmount,
    insufficientFunds,
    overdraftLimitExceeded,
    overdraftConfirmationRequired,
    cancelled,
    accountNotFound,
    sameAccount
};

struct TransactionResult {
    TransactionStatus status;
    MoneyCents amount = 0;
    MoneyCents balance = 0;
    MoneyCents secondaryBalance = 0;
    bool hasSecondaryBalance = false;
    std::string message;

    [[nodiscard]]
    bool succeeded() const noexcept {
        return status == TransactionStatus::success;
    }

    [[nodiscard]]
    static TransactionResult withBalance(
        TransactionStatus status,
        MoneyCents amount,
        MoneyCents balance,
        std::string message
    ) {
        return {status, amount, balance, 0, false, std::move(message)};
    }

    [[nodiscard]]
    static TransactionResult withTransferBalances(
        TransactionStatus status,
        MoneyCents amount,
        MoneyCents sourceBalance,
        MoneyCents destinationBalance,
        std::string message
    ) {
        return {status, amount, sourceBalance, destinationBalance, true, std::move(message)};
    }
};

class Account {
protected:
    int m_accountNumber;
    std::string m_ownerName;
    std::string m_accountType;
    MoneyCents m_balance;

    void printDisplaySummary(std::ostream& output) const;

public:
    Account(std::string_view ownerName, double balance, int accountID = 0, std::string accountType = "Account");
    virtual ~Account() = default;
    
    [[nodiscard]]
    bool quickDeposit(MoneyCents amountCents) noexcept;
    [[nodiscard]]
    bool quickWithdraw(MoneyCents amountCents) noexcept;
    
    [[nodiscard]]
    TransactionResult deposit(MoneyCents amountCents);
    [[nodiscard]]
    virtual TransactionResult withdraw(MoneyCents amountCents, bool allowOverdraft = false);
    
    [[nodiscard]]
    virtual const std::string& getAccountType() const noexcept;
    [[nodiscard]]
    int getAccountNumber() const noexcept;
    [[nodiscard]]
    double getBalance() const noexcept;
    [[nodiscard]]
    MoneyCents getBalanceInCents() const noexcept;
    [[nodiscard]]
    const std::string& getOwnerName() const noexcept;

    virtual void display() const;

private:
    static int s_nextAccountNumber;
};

#endif
