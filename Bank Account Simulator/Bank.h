//
//  Bank.h
//  Bank Account
//
//  Created by Marco D’Eusebio on 4/27/26.
//

#ifndef BANK_H
#define BANK_H

#include <cstddef>
#include <memory>
#include <string_view>
#include <vector>
#include "Account.h"

class Bank {
private:
    std::vector<std::unique_ptr<Account>> allAccounts;
    [[nodiscard]]
    Account* promptForAccount(std::string_view actionText);
    [[nodiscard]]
    Account* promptForDifferentAccount(std::string_view actionText, int disallowedAccountNumber);
    [[nodiscard]]
    MoneyCents promptForAmount(std::string_view promptText) const;
    [[nodiscard]]
    MoneyCents promptForWholeDollarAmount(std::string_view promptText) const;
    [[nodiscard]]
    bool promptYesNo(std::string_view questionText) const;
    [[nodiscard]]
    bool validateCashDenominationSelection(MoneyCents withdrawAmount) const;
    void printTransactionResult(const TransactionResult& result) const;

public:
    [[nodiscard]]
    bool addAccount(std::unique_ptr<Account> account);
    [[nodiscard]]
    std::size_t getAccountCount() const noexcept;

    [[nodiscard]]
    Account* findAccount(int accountNumber) noexcept;
    [[nodiscard]]
    const Account* findAccount(int accountNumber) const noexcept;
    [[nodiscard]]
    bool hasAccount(int accountNumber) const noexcept;
    
    void depositPrompt();
    void withdrawPrompt();
    void transferPrompt();
    
    [[nodiscard]]
    TransactionResult deposit(int accountNumber, MoneyCents amountCents);
    [[nodiscard]]
    TransactionResult withdraw(int accountNumber, MoneyCents amountCents, bool allowOverdraft = false);
    [[nodiscard]]
    TransactionResult transfer(
        int fromAccountNumber,
        int toAccountNumber,
        MoneyCents amountCents,
        bool allowOverdraft = false
    );

    void displayAllAccounts() const;
};

#endif
