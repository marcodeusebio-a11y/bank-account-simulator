//
//  Bank.cpp
//  Bank Account
//
//  Created by Marco D’Eusebio on 4/27/26.
//

#include "Bank.h"
#include "Utilities.h"
#include <array>
#include <algorithm>
#include <iostream>
#include <string>

namespace {
constexpr std::array<int, 7> kCashDenominations = {1, 2, 5, 10, 20, 50, 100};

template <typename AccountCollection>
auto findAccountIterator(AccountCollection& accounts, int accountNumber) {
    return std::find_if(accounts.begin(), accounts.end(), [accountNumber](const auto& account) {
        return account->getAccountNumber() == accountNumber;
    });
}

TransactionResult makeAccountNotFoundResult(MoneyCents amountCents, std::string message) {
    return TransactionResult::withBalance(
        TransactionStatus::accountNotFound,
        amountCents,
        0,
        std::move(message)
    );
}

} // namespace

bool Bank::addAccount(std::unique_ptr<Account> account) {
    if (account == nullptr) {
        return false;
    }

    if (findAccount(account->getAccountNumber()) != nullptr) {
        return false;
    }

    allAccounts.push_back(std::move(account));
    return true;
}

std::size_t Bank::getAccountCount() const noexcept {
    return allAccounts.size();
}

Account* Bank::findAccount(int accountNumber) noexcept {
    const auto accountIterator = findAccountIterator(allAccounts, accountNumber);
    return accountIterator != allAccounts.end() ? accountIterator->get() : nullptr;
}

const Account* Bank::findAccount(int accountNumber) const noexcept {
    const auto accountIterator = findAccountIterator(allAccounts, accountNumber);
    return accountIterator != allAccounts.end() ? accountIterator->get() : nullptr;
}

bool Bank::hasAccount(int accountNumber) const noexcept {
    return findAccount(accountNumber) != nullptr;
}

Account* Bank::promptForAccount(std::string_view actionText) {
    int accountNumber;
    Account* account = nullptr;

    do {
        std::cout << "To continue with the " << actionText << ", please input an account number and press <ENTER>.\n";
        if (actionText == "transfer from" || actionText == "transfer to") {
            std::cout << "(For transfers, \"transfer from\" is the source account and \"transfer to\" is the destination.)\n";
        }
        std::cout << "Account number: ";
        accountNumber = getValidatedInt(1);
        std::cout << "You entered:\n";
        std::cout << "Account #" << accountNumber << std::endl;
        if (!promptYesNo("Is that correct?")) {
            continue;
        }

        account = findAccount(accountNumber);
        if (account == nullptr) {
            std::cout << "Error, the account number entered does not exist.\n";
            std::cout << "Please try again.\n";
            continue;
        }
    } while (account == nullptr);

    return account;
}

Account* Bank::promptForDifferentAccount(std::string_view actionText, int disallowedAccountNumber) {
    while (true) {
        Account* account = promptForAccount(actionText);
        const int accountNumber = account->getAccountNumber();
        if (accountNumber != disallowedAccountNumber) {
            return account;
        }

        std::cout << "Error, you must choose a different account.\n";
        std::cout << "Please try again.\n";
    }
}

MoneyCents Bank::promptForAmount(std::string_view promptText) const {
    std::cout << promptText;
    return getValidatedMoneyCents(kMinimumTransactionAmountCents);
}

MoneyCents Bank::promptForWholeDollarAmount(std::string_view promptText) const {
    while (true) {
        const MoneyCents amountCents = promptForAmount(promptText);
        if (isWholeDollarAmount(amountCents)) {
            return amountCents;
        }

        std::cout << "Invalid entry.\n";
        std::cout << "Please enter a whole-dollar amount with no cents.\n";
    }
}

bool Bank::promptYesNo(std::string_view questionText) const {
    std::cout << questionText << "\n";
    std::cout << " 1) Yes\n";
    std::cout << " 2) No\n";

    return getValidatedChoice(1, 2) == 1;
}

bool Bank::validateCashDenominationSelection(MoneyCents withdrawAmount) const {
    std::cout << "Please enter the quantity desired of each specified\n";
    std::cout << "bill denomination and press <ENTER>.\n";

    MoneyCents quantityTotal = 0;
    for (int denomination : kCashDenominations) {
        std::cout << "How many $" << denomination << " bills would you like?\n";
        const int validatedQuantity = getValidatedInt();
        quantityTotal += static_cast<MoneyCents>(validatedQuantity) * (denomination * kCentsPerDollar);

        if (quantityTotal > withdrawAmount) {
            std::cout << "Error, the desired quantity of bill denominations selected\n";
            std::cout << "exceeds the user's prior withdrawal input specification.\n";
            std::cout << "Transaction canceled.\n";
            return false;
        }
    }

    if (quantityTotal != withdrawAmount) {
        std::cout << "Error, the selected bill denominations do not add up\n";
        std::cout << "to the requested withdrawal amount.\n";
        std::cout << "Transaction canceled.\n";
        return false;
    }

    return true;
}

void Bank::printTransactionResult(const TransactionResult& result) const {
    switch (result.status) {
        case TransactionStatus::success:
            std::cout << result.message;
            if (result.amount > 0) {
                std::cout << " Amount: " << formatCents(result.amount) << ".";
            }
            if (result.hasSecondaryBalance) {
                std::cout << " Source balance: " << formatCents(result.balance)
                          << ". Destination balance: " << formatCents(result.secondaryBalance) << ".";
            } else {
                std::cout << " Balance: " << formatCents(result.balance) << ".";
            }
            std::cout << "\n";
            break;
        case TransactionStatus::invalidAmount:
        case TransactionStatus::insufficientFunds:
        case TransactionStatus::overdraftLimitExceeded:
        case TransactionStatus::accountNotFound:
        case TransactionStatus::sameAccount:
            std::cout << "Error, " << result.message << "\n";
            std::cout << "Transaction canceled.\n";
            break;
        case TransactionStatus::cancelled:
            std::cout << result.message << "\n";
            std::cout << "Transaction canceled.\n";
            break;
        case TransactionStatus::overdraftConfirmationRequired:
            std::cout << "Warning, " << result.message << "\n";
            break;
    }
}

void Bank::depositPrompt() {
    Account* account = promptForAccount("deposit");
    
    std::cout << "Please select deposit type:\n";
    std::cout << "1) Cash\n";
    std::cout << "2) Check\n";
    
    int validatedChoice = getValidatedChoice(1, 2);
    
    MoneyCents depositAmount;
    if (validatedChoice == 1) {
        depositAmount = promptForWholeDollarAmount(
            "How much cash would you like to deposit?\n"
            "(Be sure to round to the nearest dollar).\n"
        );
    } else {
        depositAmount = promptForAmount(
            "Please enter the dollar and cent amount exactly as they\n"
            "appear on the front of the check.\n"
        );
    }

    printTransactionResult(deposit(account->getAccountNumber(), depositAmount));
}


void Bank::withdrawPrompt() {
    Account* account = promptForAccount("withdrawal");
    
    const MoneyCents withdrawAmount = promptForWholeDollarAmount("How much cash would you like to withdraw?\n");

    if (promptYesNo("Would you like to choose how you will be receiving your cash?")
        && !validateCashDenominationSelection(withdrawAmount)) {
        return;
    }

    TransactionResult result = withdraw(account->getAccountNumber(), withdrawAmount);
    if (result.status == TransactionStatus::overdraftConfirmationRequired) {
        std::cout << "Warning, the transaction requested surpasses the current account balance.\n";
        std::cout << "Proceeding will result in a balance of " << formatCents(result.balance) << ".\n";
        if (promptYesNo("Would you like to proceed with the withdrawal?")) {
            result = withdraw(account->getAccountNumber(), withdrawAmount, true);
        } else {
            result = TransactionResult::withBalance(
                TransactionStatus::cancelled,
                withdrawAmount,
                account->getBalanceInCents(),
                "The transaction was canceled as requested."
            );
        }
    }
    printTransactionResult(result);
}

void Bank::transferPrompt() {
    MoneyCents transferAmount;
    Account* fromAccount = promptForAccount("transfer from");
    Account* toAccount = promptForDifferentAccount("transfer to", fromAccount->getAccountNumber());

    const int fromAccountNumber = fromAccount->getAccountNumber();
    const int toAccountNumber = toAccount->getAccountNumber();

    transferAmount = promptForAmount(
        "Please enter exactly how much money you would like to transfer\n"
        "from Account #" + std::to_string(fromAccountNumber) +
        " to Account #" + std::to_string(toAccountNumber) +
        " and press <ENTER>.\n"
    );

    TransactionResult result = transfer(fromAccountNumber, toAccountNumber, transferAmount);
    if (result.status == TransactionStatus::overdraftConfirmationRequired) {
        std::cout << "Warning, the transfer requested surpasses the current source account balance.\n";
        std::cout << "Proceeding will result in a source balance of " << formatCents(result.balance) << ".\n";
        if (promptYesNo("Would you like to proceed with the transfer?")) {
            result = transfer(fromAccountNumber, toAccountNumber, transferAmount, true);
        } else {
            result = TransactionResult::withBalance(
                TransactionStatus::cancelled,
                transferAmount,
                fromAccount->getBalanceInCents(),
                "The transaction was canceled as requested."
            );
        }
    }

    printTransactionResult(result);
}

TransactionResult Bank::deposit(int accountNumber, MoneyCents amountCents) {
    if (Account* account = findAccount(accountNumber); account != nullptr) {
        return account->deposit(amountCents);
    }
    
    return makeAccountNotFoundResult(
        amountCents,
        "The account number entered does not exist."
    );
}

TransactionResult Bank::withdraw(int accountNumber, MoneyCents amountCents, bool allowOverdraft) {
    if (Account* account = findAccount(accountNumber); account != nullptr) {
        return account->withdraw(amountCents, allowOverdraft);
    }
    
    return makeAccountNotFoundResult(
        amountCents,
        "The account number entered does not exist."
    );
}

TransactionResult Bank::transfer(
    int fromAccountNumber,
    int toAccountNumber,
    MoneyCents amountCents,
    bool allowOverdraft
) {
    if (fromAccountNumber == toAccountNumber) {
        return TransactionResult::withBalance(
            TransactionStatus::sameAccount,
            amountCents,
            0,
            "You must choose two different accounts."
        );
    }

    Account* fromAccount = findAccount(fromAccountNumber);
    if (amountCents <= 0) {
        return TransactionResult::withBalance(
            TransactionStatus::invalidAmount,
            amountCents,
            fromAccount != nullptr ? fromAccount->getBalanceInCents() : 0,
            "The transfer amount must be greater than zero."
        );
    }

    Account* toAccount = findAccount(toAccountNumber);
    if (fromAccount == nullptr || toAccount == nullptr) {
        return makeAccountNotFoundResult(
            amountCents,
            "One or both account numbers do not exist."
        );
    }

    MoneyCents projectedDestinationBalance = 0;
    if (!tryAddMoneyCents(toAccount->getBalanceInCents(), amountCents, projectedDestinationBalance)) {
        return TransactionResult::withBalance(
            TransactionStatus::invalidAmount,
            amountCents,
            fromAccount->getBalanceInCents(),
            "The transfer would exceed the destination account's maximum supported balance."
        );
    }
    
    TransactionResult withdrawResult = fromAccount->withdraw(amountCents, allowOverdraft);
    if (!withdrawResult.succeeded()) {
        if (withdrawResult.status == TransactionStatus::insufficientFunds) {
            withdrawResult.message = "The transfer amount exceeds the source account balance.";
        } else if (withdrawResult.status == TransactionStatus::overdraftConfirmationRequired) {
            withdrawResult.message = "This transfer requires overdraft confirmation.";
        }

        return withdrawResult;
    }

    if (!toAccount->quickDeposit(amountCents)) {
        static_cast<void>(fromAccount->quickDeposit(amountCents));
        return TransactionResult::withBalance(
            TransactionStatus::invalidAmount,
            amountCents,
            fromAccount->getBalanceInCents(),
            "The transfer could not be completed for the amount requested."
        );
    }

    return TransactionResult::withTransferBalances(
        TransactionStatus::success,
        amountCents,
        fromAccount->getBalanceInCents(),
        toAccount->getBalanceInCents(),
        "Transfer processed successfully."
    );
}

void Bank::displayAllAccounts() const {
    const std::size_t accountCount = getAccountCount();

    if (accountCount == 0) {
        std::cout << "No accounts are currently available.\n";
        return;
    }

    std::cout << "Displaying " << accountCount << " account";
    if (accountCount != 1) {
        std::cout << "s";
    }
    std::cout << ":\n";

    for (const auto& displayAccount : allAccounts) {
        displayAccount->display();
    }
}

