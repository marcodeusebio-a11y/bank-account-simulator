#ifndef SELFCHECKS_H
#define SELFCHECKS_H

#include <memory>
#include <limits>
#include <string>
#include <vector>
#include <iostream>
#include "Bank.h"
#include "CheckingAccount.h"
#include "SavingsAccount.h"
#include "Utilities.h"

[[nodiscard]]
inline std::vector<std::string> collectSelfCheckFailures() {
    std::vector<std::string> failures;

    SavingsAccount firstSavings("Test Savings A", 500.0, 0.03);
    SavingsAccount secondSavings("Test Savings B", 250.0, 0.04);
    if (secondSavings.getAccountNumber() != firstSavings.getAccountNumber() + 1) {
        failures.push_back("Automatic account numbering is not sequential.");
    }

    SavingsAccount baselineSavings("Baseline Savings", 100.0, 0.01);
    const int elevatedAccountNumber = baselineSavings.getAccountNumber() + 50;
    SavingsAccount seededSavings("Seeded Savings", 500.0, 0.03, elevatedAccountNumber);
    SavingsAccount autoNumberedSavings("Auto Savings", 250.0, 0.04);
    if (seededSavings.getAccountNumber() != elevatedAccountNumber
        || autoNumberedSavings.getAccountNumber() != elevatedAccountNumber + 1) {
        failures.push_back("Explicit account numbering did not advance future automatic assignments when given a higher ID.");
    }

    SavingsAccount seededLowerSavings("Seeded Lower Savings", 200.0, 0.02, baselineSavings.getAccountNumber());
    SavingsAccount continuedAutoSavings("Continued Auto Savings", 125.0, 0.02);
    if (continuedAutoSavings.getAccountNumber() != autoNumberedSavings.getAccountNumber() + 1) {
        failures.push_back("Explicit account numbering rewound future automatic assignments when given a lower ID.");
    }

    TransactionResult depositResult = firstSavings.deposit(dollarsToCents(25.0));
    if (!depositResult.succeeded()
        || depositResult.amount != dollarsToCents(25.0)
        || depositResult.hasSecondaryBalance
        || firstSavings.getBalanceInCents() != dollarsToCents(525.0)) {
        failures.push_back("Savings deposit did not update the balance correctly.");
    }

    TransactionResult invalidDeposit = firstSavings.deposit(0);
    if (invalidDeposit.status != TransactionStatus::invalidAmount
        || invalidDeposit.amount != 0
        || invalidDeposit.hasSecondaryBalance
        || invalidDeposit.balance != dollarsToCents(525.0)) {
        failures.push_back("Deposits did not reject a zero-dollar amount.");
    }

    CheckingAccount depositOverflowAccount("Deposit Overflow", std::numeric_limits<double>::max(), -20.0, 9701);
    TransactionResult overflowDeposit = depositOverflowAccount.deposit(50);
    if (overflowDeposit.status != TransactionStatus::invalidAmount
        || overflowDeposit.amount != 50
        || overflowDeposit.balance != std::numeric_limits<MoneyCents>::max()
        || overflowDeposit.message != "The deposit would exceed the maximum supported account balance."
        || depositOverflowAccount.getBalanceInCents() != std::numeric_limits<MoneyCents>::max()) {
        failures.push_back("Deposits did not report balance-limit overflow correctly.");
    }

    SavingsAccount interestSavings("Interest Test", 200.0, 0.05);
    interestSavings.addInterest();
    if (interestSavings.getBalanceInCents() != dollarsToCents(210.0)) {
        failures.push_back("Savings interest accrual did not update the balance correctly.");
    }

    SavingsAccount roundedInterestSavings("Rounded Interest", 10.10, 0.05);
    if (roundedInterestSavings.getAccruedInterest() != 51) {
        failures.push_back("Savings interest accrual did not round fractional cents correctly.");
    }
    roundedInterestSavings.addInterest();
    if (roundedInterestSavings.getBalanceInCents() != dollarsToCents(10.61)) {
        failures.push_back("Savings interest application did not update the rounded balance correctly.");
    }

    SavingsAccount zeroBalanceSavings("Zero Balance", 0.0, 0.05);
    zeroBalanceSavings.addInterest();
    if (zeroBalanceSavings.getAccruedInterest() != 0
        || zeroBalanceSavings.getBalanceInCents() != 0) {
        failures.push_back("Savings interest changed a zero-balance account unexpectedly.");
    }

    SavingsAccount maximumBalanceSavings("Maximum Balance", std::numeric_limits<double>::max(), 0.05);
    maximumBalanceSavings.addInterest();
    if (maximumBalanceSavings.getBalanceInCents() != std::numeric_limits<MoneyCents>::max()) {
        failures.push_back("Savings interest did not saturate at the maximum supported balance.");
    }

    SavingsAccount normalizedSavings("Normalized Savings", 200.0, -0.05);
    if (normalizedSavings.getInterestRate() != 0.0
        || normalizedSavings.getAccruedInterest() != 0) {
        failures.push_back("Savings accounts did not normalize negative interest input correctly.");
    }

    SavingsAccount nonFiniteSavings("Non-Finite Savings", 200.0, std::numeric_limits<double>::infinity());
    if (nonFiniteSavings.getInterestRate() != 0.0
        || nonFiniteSavings.getAccruedInterest() != 0) {
        failures.push_back("Savings accounts did not normalize non-finite interest input correctly.");
    }

    CheckingAccount checking("Test Checking", 300.0, -20.0);
    TransactionResult overdraftPrompt = checking.withdraw(dollarsToCents(310.0));
    if (overdraftPrompt.status != TransactionStatus::overdraftConfirmationRequired
        || overdraftPrompt.amount != dollarsToCents(310.0)
        || overdraftPrompt.hasSecondaryBalance
        || overdraftPrompt.balance != dollarsToCents(-10.0)) {
        failures.push_back("Checking overdraft confirmation was not requested when expected.");
    }

    TransactionResult overdraftApproved = checking.withdraw(dollarsToCents(310.0), true);
    if (!overdraftApproved.succeeded()
        || overdraftApproved.amount != dollarsToCents(310.0)
        || overdraftApproved.hasSecondaryBalance
        || checking.getBalanceInCents() != dollarsToCents(-10.0)) {
        failures.push_back("Approved overdraft withdrawal did not produce the expected balance.");
    }

    CheckingAccount strictChecking("Strict Checking", 100.0, -20.0);
    TransactionResult overdraftRejected = strictChecking.withdraw(dollarsToCents(130.0));
    if (overdraftRejected.status != TransactionStatus::overdraftLimitExceeded
        || overdraftRejected.amount != dollarsToCents(130.0)
        || overdraftRejected.hasSecondaryBalance
        || overdraftRejected.balance != dollarsToCents(100.0)) {
        failures.push_back("Checking withdrawals did not reject amounts beyond the overdraft limit.");
    }

    CheckingAccount normalizedChecking("Normalized Checking", 100.0, 20.0);
    TransactionResult normalizedOverdraft = normalizedChecking.withdraw(dollarsToCents(110.0));
    if (normalizedOverdraft.status != TransactionStatus::overdraftConfirmationRequired) {
        failures.push_back("Checking accounts did not normalize positive overdraft input correctly.");
    }
    if (normalizedChecking.getOverdraftLimit() != -20.0
        || normalizedChecking.getOverdraftLimitInCents() != dollarsToCents(-20.0)) {
        failures.push_back("Checking overdraft accessors did not return the normalized limit correctly.");
    }

    Bank transactionBank;
    if (!transactionBank.addAccount(std::make_unique<SavingsAccount>("Bank Deposit", 100.0, 0.03, 9101))) {
        failures.push_back("Bank setup failed for direct deposit and withdrawal checks.");
    } else {
        TransactionResult bankDepositResult = transactionBank.deposit(9101, dollarsToCents(15.0));
        const Account* bankDepositAccount = transactionBank.findAccount(9101);
        if (!bankDepositResult.succeeded()
            || bankDepositResult.amount != dollarsToCents(15.0)
            || bankDepositResult.hasSecondaryBalance
            || bankDepositResult.balance != dollarsToCents(115.0)
            || bankDepositAccount == nullptr
            || bankDepositAccount->getBalanceInCents() != dollarsToCents(115.0)) {
            failures.push_back("Bank deposit did not update the target account correctly.");
        }

        TransactionResult bankWithdrawResult = transactionBank.withdraw(9101, dollarsToCents(20.0));
        const Account* bankWithdrawAccount = transactionBank.findAccount(9101);
        if (!bankWithdrawResult.succeeded()
            || bankWithdrawResult.amount != dollarsToCents(20.0)
            || bankWithdrawResult.hasSecondaryBalance
            || bankWithdrawResult.balance != dollarsToCents(95.0)
            || bankWithdrawAccount == nullptr
            || bankWithdrawAccount->getBalanceInCents() != dollarsToCents(95.0)) {
            failures.push_back("Bank withdrawal did not update the target account correctly.");
        }
    }

    TransactionResult missingDepositResult = transactionBank.deposit(9999, dollarsToCents(10.0));
    if (missingDepositResult.status != TransactionStatus::accountNotFound
        || missingDepositResult.amount != dollarsToCents(10.0)
        || missingDepositResult.hasSecondaryBalance
        || missingDepositResult.balance != 0) {
        failures.push_back("Bank deposit did not report a missing account correctly.");
    }

    TransactionResult invalidBankDepositResult = transactionBank.deposit(9101, 0);
    if (invalidBankDepositResult.status != TransactionStatus::invalidAmount
        || invalidBankDepositResult.amount != 0
        || invalidBankDepositResult.hasSecondaryBalance
        || invalidBankDepositResult.balance != dollarsToCents(95.0)) {
        failures.push_back("Bank deposit did not reject an invalid amount correctly.");
    }

    TransactionResult missingWithdrawResult = transactionBank.withdraw(9999, dollarsToCents(10.0));
    if (missingWithdrawResult.status != TransactionStatus::accountNotFound
        || missingWithdrawResult.amount != dollarsToCents(10.0)
        || missingWithdrawResult.hasSecondaryBalance
        || missingWithdrawResult.balance != 0) {
        failures.push_back("Bank withdrawal did not report a missing account correctly.");
    }

    TransactionResult insufficientWithdrawResult = transactionBank.withdraw(9101, dollarsToCents(500.0));
    if (insufficientWithdrawResult.status != TransactionStatus::insufficientFunds
        || insufficientWithdrawResult.amount != dollarsToCents(500.0)
        || insufficientWithdrawResult.hasSecondaryBalance
        || insufficientWithdrawResult.balance != dollarsToCents(95.0)) {
        failures.push_back("Bank withdrawal did not report insufficient funds correctly.");
    }

    Bank overdraftBank;
    if (!overdraftBank.addAccount(std::make_unique<CheckingAccount>("Bank Overdraft", 300.0, -20.0, 9201))) {
        failures.push_back("Bank setup failed for direct overdraft checks.");
    } else {
        TransactionResult bankOverdraftPrompt = overdraftBank.withdraw(9201, dollarsToCents(310.0));
        if (bankOverdraftPrompt.status != TransactionStatus::overdraftConfirmationRequired
            || bankOverdraftPrompt.amount != dollarsToCents(310.0)
            || bankOverdraftPrompt.hasSecondaryBalance
            || bankOverdraftPrompt.balance != dollarsToCents(-10.0)) {
            failures.push_back("Bank withdrawal did not request overdraft confirmation correctly.");
        }

        TransactionResult bankOverdraftApproved = overdraftBank.withdraw(9201, dollarsToCents(310.0), true);
        const Account* overdraftAccount = overdraftBank.findAccount(9201);
        if (!bankOverdraftApproved.succeeded()
            || bankOverdraftApproved.amount != dollarsToCents(310.0)
            || bankOverdraftApproved.hasSecondaryBalance
            || bankOverdraftApproved.balance != dollarsToCents(-10.0)
            || overdraftAccount == nullptr
            || overdraftAccount->getBalanceInCents() != dollarsToCents(-10.0)) {
            failures.push_back("Bank withdrawal did not apply an approved overdraft correctly.");
        }
    }

    Bank bank;
    if (bank.addAccount(nullptr) || bank.getAccountCount() != 0) {
        failures.push_back("Bank accepted a null account entry.");
    }

    if (!bank.addAccount(std::make_unique<SavingsAccount>("Transfer A", 100.0, 0.03, 9001))) {
        failures.push_back("Bank rejected a valid account entry.");
    }
    if (!bank.hasAccount(9001) || bank.hasAccount(9999)) {
        failures.push_back("Bank account lookup did not report account existence correctly.");
    }
    if (bank.findAccount(9001) == nullptr || bank.findAccount(9999) != nullptr) {
        failures.push_back("Bank account search did not return the expected pointers.");
    }
    const Bank& constBank = bank;
    if (constBank.findAccount(9001) == nullptr || constBank.findAccount(9999) != nullptr) {
        failures.push_back("Const bank account search did not return the expected pointers.");
    }
    if (bank.addAccount(std::make_unique<CheckingAccount>("Duplicate Transfer A", 80.0, -20.0, 9001))
        || bank.getAccountCount() != 1) {
        failures.push_back("Bank accepted a duplicate account number.");
    }

    if (!bank.addAccount(std::make_unique<CheckingAccount>("Transfer B", 50.0, -20.0, 9002))
        || bank.getAccountCount() != 2
        || !bank.hasAccount(9002)) {
        failures.push_back("Bank did not accept a second unique account entry correctly.");
    }
    TransactionResult transferResult = bank.transfer(9001, 9002, dollarsToCents(40.0));
    const Account* fromAccount = bank.findAccount(9001);
    const Account* toAccount = bank.findAccount(9002);
    if (!transferResult.succeeded()
        || transferResult.amount != dollarsToCents(40.0)
        || !transferResult.hasSecondaryBalance
        || fromAccount == nullptr
        || toAccount == nullptr
        || fromAccount->getBalanceInCents() != dollarsToCents(60.0)
        || toAccount->getBalanceInCents() != dollarsToCents(90.0)) {
        failures.push_back("Transfer did not update source and destination balances correctly.");
    }

    TransactionResult sameAccountTransfer = bank.transfer(9001, 9001, dollarsToCents(10.0));
    if (sameAccountTransfer.status != TransactionStatus::sameAccount
        || sameAccountTransfer.hasSecondaryBalance) {
        failures.push_back("Transfer did not reject a same-account transfer request.");
    }

    TransactionResult missingSameAccountTransfer = bank.transfer(9999, 9999, dollarsToCents(10.0));
    if (missingSameAccountTransfer.status != TransactionStatus::sameAccount
        || missingSameAccountTransfer.amount != dollarsToCents(10.0)
        || missingSameAccountTransfer.hasSecondaryBalance
        || missingSameAccountTransfer.balance != 0) {
        failures.push_back("Transfer did not reject a same-account request before account lookup.");
    }

    TransactionResult missingAccountTransfer = bank.transfer(9001, 9999, dollarsToCents(10.0));
    if (missingAccountTransfer.status != TransactionStatus::accountNotFound
        || missingAccountTransfer.hasSecondaryBalance) {
        failures.push_back("Transfer did not report a missing destination account.");
    }

    TransactionResult insufficientTransfer = bank.transfer(9001, 9002, dollarsToCents(500.0));
    if (insufficientTransfer.status != TransactionStatus::insufficientFunds
        || insufficientTransfer.amount != dollarsToCents(500.0)
        || insufficientTransfer.hasSecondaryBalance
        || insufficientTransfer.balance != dollarsToCents(60.0)) {
        failures.push_back("Transfer did not report insufficient source funds correctly.");
    }

    TransactionResult invalidTransferAmount = bank.transfer(9001, 9002, 0);
    if (invalidTransferAmount.status != TransactionStatus::invalidAmount
        || invalidTransferAmount.amount != 0
        || invalidTransferAmount.hasSecondaryBalance
        || invalidTransferAmount.balance != dollarsToCents(60.0)) {
        failures.push_back("Transfer did not reject an invalid amount correctly.");
    }
    if (fromAccount == nullptr
        || toAccount == nullptr
        || fromAccount->getBalanceInCents() != dollarsToCents(60.0)
        || toAccount->getBalanceInCents() != dollarsToCents(90.0)) {
        failures.push_back("Transfer changed account balances after an invalid amount request.");
    }

    TransactionResult missingInvalidTransferAmount = bank.transfer(9999, 8888, 0);
    if (missingInvalidTransferAmount.status != TransactionStatus::invalidAmount
        || missingInvalidTransferAmount.amount != 0
        || missingInvalidTransferAmount.hasSecondaryBalance
        || missingInvalidTransferAmount.balance != 0) {
        failures.push_back("Transfer did not reject an invalid amount before account lookup.");
    }

    Bank overflowTransferBank;
    if (!overflowTransferBank.addAccount(std::make_unique<CheckingAccount>("Overflow Source", 5.0, -20.0, 9401))) {
        failures.push_back("Bank setup failed for transfer overflow checks.");
    } else {
        const MoneyCents nearMaximumTransferBalance = std::numeric_limits<MoneyCents>::max() - 25;
        if (!overflowTransferBank.addAccount(std::make_unique<CheckingAccount>("Overflow Destination", 0.0, -20.0, 9402))) {
            failures.push_back("Bank setup failed for transfer destination overflow checks.");
        } else {
            Account* overflowDestination = overflowTransferBank.findAccount(9402);
            if (overflowDestination == nullptr) {
                failures.push_back("Transfer overflow destination account could not be found.");
            } else {
                const MoneyCents overflowAdjustment = nearMaximumTransferBalance - overflowDestination->getBalanceInCents();
                const bool seededOverflowBalance = overflowAdjustment >= 0
                    && overflowDestination->quickDeposit(overflowAdjustment);
                TransactionResult overflowTransfer = overflowTransferBank.transfer(9401, 9402, 50);
                const Account* overflowSourceAfterTransfer = overflowTransferBank.findAccount(9401);
                const Account* overflowDestinationAfterTransfer = overflowTransferBank.findAccount(9402);
                if (!seededOverflowBalance
                    || overflowTransfer.status != TransactionStatus::invalidAmount
                    || overflowTransfer.amount != 50
                    || overflowTransfer.hasSecondaryBalance
                    || overflowTransfer.balance != 500
                    || overflowSourceAfterTransfer == nullptr
                    || overflowDestinationAfterTransfer == nullptr
                    || overflowSourceAfterTransfer->getBalanceInCents() != 500
                    || overflowDestinationAfterTransfer->getBalanceInCents() != nearMaximumTransferBalance) {
                    failures.push_back("Transfer did not reject a destination overflow without changing balances.");
                }
            }
        }
    }

    Bank overdraftTransferBank;
    if (!overdraftTransferBank.addAccount(std::make_unique<CheckingAccount>("Transfer Overdraft Source", 300.0, -20.0, 9301))
        || !overdraftTransferBank.addAccount(std::make_unique<SavingsAccount>("Transfer Overdraft Destination", 50.0, 0.03, 9302))) {
        failures.push_back("Bank setup failed for transfer overdraft checks.");
    } else {
        TransactionResult transferOverdraftPrompt = overdraftTransferBank.transfer(9301, 9302, dollarsToCents(310.0));
        const Account* transferSourceBeforeApproval = overdraftTransferBank.findAccount(9301);
        const Account* transferDestinationBeforeApproval = overdraftTransferBank.findAccount(9302);
        if (transferOverdraftPrompt.status != TransactionStatus::overdraftConfirmationRequired
            || transferOverdraftPrompt.amount != dollarsToCents(310.0)
            || transferOverdraftPrompt.hasSecondaryBalance
            || transferOverdraftPrompt.balance != dollarsToCents(-10.0)
            || transferSourceBeforeApproval == nullptr
            || transferDestinationBeforeApproval == nullptr
            || transferSourceBeforeApproval->getBalanceInCents() != dollarsToCents(300.0)
            || transferDestinationBeforeApproval->getBalanceInCents() != dollarsToCents(50.0)) {
            failures.push_back("Transfer did not request overdraft confirmation correctly.");
        }

        TransactionResult transferOverdraftApproved = overdraftTransferBank.transfer(
            9301,
            9302,
            dollarsToCents(310.0),
            true
        );
        const Account* transferSourceAfterApproval = overdraftTransferBank.findAccount(9301);
        const Account* transferDestinationAfterApproval = overdraftTransferBank.findAccount(9302);
        if (!transferOverdraftApproved.succeeded()
            || transferOverdraftApproved.amount != dollarsToCents(310.0)
            || !transferOverdraftApproved.hasSecondaryBalance
            || transferOverdraftApproved.balance != dollarsToCents(-10.0)
            || transferOverdraftApproved.secondaryBalance != dollarsToCents(360.0)
            || transferSourceAfterApproval == nullptr
            || transferDestinationAfterApproval == nullptr
            || transferSourceAfterApproval->getBalanceInCents() != dollarsToCents(-10.0)
            || transferDestinationAfterApproval->getBalanceInCents() != dollarsToCents(360.0)) {
            failures.push_back("Transfer did not apply an approved overdraft correctly.");
        }
    }

    if (formatCents(dollarsToCents(-20.5)) != "-$20.50"
        || formatCents(dollarsToCents(20.5)) != "$20.50") {
        failures.push_back("Money formatting did not place the negative sign before the currency symbol.");
    }
    if (formatCents(1) != "$0.01"
        || formatCents(-1) != "-$0.01") {
        failures.push_back("Money formatting did not preserve single-cent precision.");
    }
    if (!isWholeDollarAmount(dollarsToCents(10.0))
        || isWholeDollarAmount(dollarsToCents(10.5))) {
        failures.push_back("Whole-dollar validation did not distinguish cash amounts correctly.");
    }
    MoneyCents arithmeticResult = 0;
    if (tryAddMoneyCents(std::numeric_limits<MoneyCents>::max(), 1, arithmeticResult)
        || !tryAddMoneyCents(125, 75, arithmeticResult)
        || arithmeticResult != 200) {
        failures.push_back("Safe money addition did not guard against overflow correctly.");
    }
    if (trySubtractMoneyCents(std::numeric_limits<MoneyCents>::min(), 1, arithmeticResult)
        || trySubtractMoneyCents(0, std::numeric_limits<MoneyCents>::min(), arithmeticResult)
        || !trySubtractMoneyCents(125, 75, arithmeticResult)
        || arithmeticResult != 50) {
        failures.push_back("Safe money subtraction did not guard against underflow correctly.");
    }
    if (dollarsToCents(std::numeric_limits<double>::quiet_NaN()) != 0
        || dollarsToCents(std::numeric_limits<double>::infinity()) != 0
        || dollarsToCents(-std::numeric_limits<double>::infinity()) != 0) {
        failures.push_back("Money conversion did not normalize non-finite values correctly.");
    }
    if (dollarsToCents(std::numeric_limits<double>::max()) != std::numeric_limits<MoneyCents>::max()
        || dollarsToCents(-std::numeric_limits<double>::max()) != std::numeric_limits<MoneyCents>::min()) {
        failures.push_back("Money conversion did not saturate oversized values correctly.");
    }
    if (calculatePercentageOfCents(1000, std::numeric_limits<double>::quiet_NaN()) != 0
        || calculatePercentageOfCents(1000, std::numeric_limits<double>::infinity()) != 0
        || calculatePercentageOfCents(1000, -std::numeric_limits<double>::infinity()) != 0) {
        failures.push_back("Percentage-based money conversion did not normalize non-finite rates correctly.");
    }
    if (calculatePercentageOfCents(1000, std::numeric_limits<double>::max()) != std::numeric_limits<MoneyCents>::max()
        || calculatePercentageOfCents(-1000, std::numeric_limits<double>::max()) != std::numeric_limits<MoneyCents>::min()) {
        failures.push_back("Percentage-based money conversion did not saturate oversized rates correctly.");
    }

    return failures;
}

[[nodiscard]]
inline bool runSelfChecks() {
    std::vector<std::string> failures = collectSelfCheckFailures();

    if (failures.empty()) {
        std::cout << "Self-checks passed.\n";
        return true;
    }

    std::cout << "Self-checks failed:\n";
    for (const std::string& failure : failures) {
        std::cout << " - " << failure << "\n";
    }

    return false;
}

#endif
