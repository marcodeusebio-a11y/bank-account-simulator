#import <XCTest/XCTest.h>

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

#include "../Bank Account Simulator/Utilities.h"
#include "../Bank Account Simulator/Bank.h"
#include "../Bank Account Simulator/CheckingAccount.h"
#include "../Bank Account Simulator/SavingsAccount.h"

namespace {
class TestAccount final : public Account {
public:
    TestAccount(std::string_view ownerName, MoneyCents balanceCents, int accountNumber)
        : Account(ownerName, 0.0, accountNumber, "Test Account") {
        m_balance = balanceCents;
    }
};

class ScopedStandardStreamRedirect final {
public:
    ScopedStandardStreamRedirect(std::istream& input, std::ostream& output, std::string_view inputText)
        : m_input(input),
          m_output(output),
          m_inputBuffer(std::string(inputText)),
          m_originalInputBuffer(input.rdbuf(m_inputBuffer.rdbuf())),
          m_originalOutputBuffer(output.rdbuf(m_outputBuffer.rdbuf())) {}

    ~ScopedStandardStreamRedirect() {
        m_input.rdbuf(m_originalInputBuffer);
        m_output.rdbuf(m_originalOutputBuffer);
    }

    [[nodiscard]]
    std::string output() const {
        return m_outputBuffer.str();
    }

private:
    std::istream& m_input;
    std::ostream& m_output;
    std::istringstream m_inputBuffer;
    std::ostringstream m_outputBuffer;
    std::streambuf* m_originalInputBuffer;
    std::streambuf* m_originalOutputBuffer;
};
} // namespace

@interface BankAccountSimulatorTests : XCTestCase
@end

@implementation BankAccountSimulatorTests

- (void)testAutomaticAccountNumberingIsSequential {
    SavingsAccount firstSavings("Test Savings A", 500.0, 0.03);
    SavingsAccount secondSavings("Test Savings B", 250.0, 0.04);

    XCTAssertEqual(secondSavings.getAccountNumber(), firstSavings.getAccountNumber() + 1);
}

- (void)testExplicitAccountNumberAdvancesFutureAutomaticAssignments {
    SavingsAccount baselineSavings("Baseline Savings", 100.0, 0.01);
    const int elevatedAccountNumber = baselineSavings.getAccountNumber() + 50;
    SavingsAccount seededSavings("Seeded Savings", 500.0, 0.03, elevatedAccountNumber);
    SavingsAccount autoNumberedSavings("Auto Savings", 250.0, 0.04);
    SavingsAccount seededLowerSavings("Seeded Lower Savings", 500.0, 0.03, baselineSavings.getAccountNumber());
    SavingsAccount continuedAutoSavings("Continued Auto Savings", 125.0, 0.02);

    XCTAssertEqual(seededSavings.getAccountNumber(), elevatedAccountNumber);
    XCTAssertEqual(autoNumberedSavings.getAccountNumber(), elevatedAccountNumber + 1);
    XCTAssertEqual(seededLowerSavings.getAccountNumber(), baselineSavings.getAccountNumber());
    XCTAssertEqual(continuedAutoSavings.getAccountNumber(), autoNumberedSavings.getAccountNumber() + 1);
}

- (void)testDepositUpdatesBalance {
    SavingsAccount savings("Deposit Test", 500.0, 0.03);

    TransactionResult result = savings.deposit(dollarsToCents(25.0));

    XCTAssertTrue(result.succeeded());
    XCTAssertEqual(result.amount, dollarsToCents(25.0));
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, dollarsToCents(525.0));
    XCTAssertEqual(savings.getBalanceInCents(), dollarsToCents(525.0));
}

- (void)testDepositRejectsZeroAmount {
    SavingsAccount savings("Deposit Test", 500.0, 0.03);

    TransactionResult result = savings.deposit(0);

    XCTAssertEqual(result.status, TransactionStatus::invalidAmount);
    XCTAssertEqual(result.amount, 0);
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, dollarsToCents(500.0));
}

- (void)testDepositReportsBalanceLimitOverflow {
    TestAccount account("Deposit Overflow", std::numeric_limits<MoneyCents>::max() - 25, 9601);

    TransactionResult result = account.deposit(50);

    XCTAssertEqual(result.status, TransactionStatus::invalidAmount);
    XCTAssertEqual(result.amount, 50);
    XCTAssertEqual(result.balance, std::numeric_limits<MoneyCents>::max() - 25);
    XCTAssertEqual(result.message, std::string("The deposit would exceed the maximum supported account balance."));
    XCTAssertEqual(account.getBalanceInCents(), std::numeric_limits<MoneyCents>::max() - 25);
}

- (void)testCheckingOverdraftRequiresApprovalBeforeCompletion {
    CheckingAccount checking("Checking Test", 300.0, -20.0);

    TransactionResult pendingResult = checking.withdraw(dollarsToCents(310.0));
    XCTAssertEqual(pendingResult.status, TransactionStatus::overdraftConfirmationRequired);
    XCTAssertEqual(pendingResult.amount, dollarsToCents(310.0));
    XCTAssertFalse(pendingResult.hasSecondaryBalance);
    XCTAssertEqual(pendingResult.balance, dollarsToCents(-10.0));

    TransactionResult approvedResult = checking.withdraw(dollarsToCents(310.0), true);
    XCTAssertTrue(approvedResult.succeeded());
    XCTAssertEqual(approvedResult.amount, dollarsToCents(310.0));
    XCTAssertFalse(approvedResult.hasSecondaryBalance);
    XCTAssertEqual(approvedResult.balance, dollarsToCents(-10.0));
    XCTAssertEqual(checking.getBalanceInCents(), dollarsToCents(-10.0));
}

- (void)testCheckingRejectsAmountsBeyondOverdraftLimit {
    CheckingAccount checking("Checking Test", 100.0, -20.0);

    TransactionResult result = checking.withdraw(dollarsToCents(130.0));

    XCTAssertEqual(result.status, TransactionStatus::overdraftLimitExceeded);
    XCTAssertEqual(result.amount, dollarsToCents(130.0));
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, dollarsToCents(100.0));
}

- (void)testCheckingNormalizesPositiveOverdraftInput {
    CheckingAccount checking("Checking Test", 100.0, 20.0);

    TransactionResult result = checking.withdraw(dollarsToCents(110.0));

    XCTAssertEqual(result.status, TransactionStatus::overdraftConfirmationRequired);
}

- (void)testCheckingOverdraftAccessorsReturnNormalizedLimit {
    CheckingAccount checking("Checking Test", 100.0, 20.0);

    XCTAssertEqual(checking.getOverdraftLimit(), -20.0);
    XCTAssertEqual(checking.getOverdraftLimitInCents(), dollarsToCents(-20.0));
}

- (void)testBankIgnoresNullAccounts {
    Bank bank;

    XCTAssertFalse(bank.addAccount(nullptr));

    XCTAssertEqual(bank.getAccountCount(), static_cast<std::size_t>(0));
}

- (void)testBankIgnoresDuplicateAccountNumbers {
    Bank bank;

    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Transfer A", 100.0, 0.03, 9001)));
    XCTAssertTrue(bank.hasAccount(9001));
    XCTAssertFalse(bank.hasAccount(9999));
    XCTAssertFalse(bank.addAccount(std::make_unique<CheckingAccount>("Duplicate Transfer A", 80.0, -20.0, 9001)));

    XCTAssertEqual(bank.getAccountCount(), static_cast<std::size_t>(1));
}

- (void)testBankAcceptsSecondUniqueAccount {
    Bank bank;

    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Transfer A", 100.0, 0.03, 9001)));
    XCTAssertTrue(bank.addAccount(std::make_unique<CheckingAccount>("Transfer B", 50.0, -20.0, 9002)));
    XCTAssertTrue(bank.hasAccount(9002));
    XCTAssertEqual(bank.getAccountCount(), static_cast<std::size_t>(2));
}

- (void)testBankFindAccountReturnsExpectedPointers {
    Bank bank;

    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Transfer A", 100.0, 0.03, 9001)));
    XCTAssertNotEqual(bank.findAccount(9001), nullptr);
    XCTAssertEqual(bank.findAccount(9999), nullptr);

    const Bank &constBank = bank;
    XCTAssertNotEqual(constBank.findAccount(9001), nullptr);
    XCTAssertEqual(constBank.findAccount(9999), nullptr);
}

- (void)testBankDepositUpdatesTargetAccount {
    Bank bank;

    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Bank Deposit", 100.0, 0.03, 9101)));

    TransactionResult result = bank.deposit(9101, dollarsToCents(15.0));
    const Account *account = bank.findAccount(9101);

    XCTAssertTrue(result.succeeded());
    XCTAssertEqual(result.amount, dollarsToCents(15.0));
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, dollarsToCents(115.0));
    XCTAssertNotEqual(account, nullptr);
    XCTAssertEqual(account->getBalanceInCents(), dollarsToCents(115.0));
}

- (void)testBankWithdrawUpdatesTargetAccount {
    Bank bank;

    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Bank Withdraw", 100.0, 0.03, 9101)));

    TransactionResult result = bank.withdraw(9101, dollarsToCents(20.0));
    const Account *account = bank.findAccount(9101);

    XCTAssertTrue(result.succeeded());
    XCTAssertEqual(result.amount, dollarsToCents(20.0));
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, dollarsToCents(80.0));
    XCTAssertNotEqual(account, nullptr);
    XCTAssertEqual(account->getBalanceInCents(), dollarsToCents(80.0));
}

- (void)testBankDepositReportsMissingAccount {
    Bank bank;

    TransactionResult result = bank.deposit(9999, dollarsToCents(10.0));

    XCTAssertEqual(result.status, TransactionStatus::accountNotFound);
    XCTAssertEqual(result.amount, dollarsToCents(10.0));
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, 0);
}

- (void)testBankDepositRejectsInvalidAmount {
    Bank bank;

    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Bank Deposit", 95.0, 0.03, 9101)));

    TransactionResult result = bank.deposit(9101, 0);

    XCTAssertEqual(result.status, TransactionStatus::invalidAmount);
    XCTAssertEqual(result.amount, 0);
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, dollarsToCents(95.0));
}

- (void)testBankWithdrawReportsMissingAccount {
    Bank bank;

    TransactionResult result = bank.withdraw(9999, dollarsToCents(10.0));

    XCTAssertEqual(result.status, TransactionStatus::accountNotFound);
    XCTAssertEqual(result.amount, dollarsToCents(10.0));
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, 0);
}

- (void)testBankWithdrawReportsInsufficientFunds {
    Bank bank;

    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Bank Withdraw", 95.0, 0.03, 9101)));

    TransactionResult result = bank.withdraw(9101, dollarsToCents(500.0));

    XCTAssertEqual(result.status, TransactionStatus::insufficientFunds);
    XCTAssertEqual(result.amount, dollarsToCents(500.0));
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, dollarsToCents(95.0));
}

- (void)testBankWithdrawRequestsOverdraftConfirmation {
    Bank bank;

    XCTAssertTrue(bank.addAccount(std::make_unique<CheckingAccount>("Bank Overdraft", 300.0, -20.0, 9201)));

    TransactionResult result = bank.withdraw(9201, dollarsToCents(310.0));

    XCTAssertEqual(result.status, TransactionStatus::overdraftConfirmationRequired);
    XCTAssertEqual(result.amount, dollarsToCents(310.0));
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, dollarsToCents(-10.0));
}

- (void)testBankWithdrawAppliesApprovedOverdraft {
    Bank bank;

    XCTAssertTrue(bank.addAccount(std::make_unique<CheckingAccount>("Bank Overdraft", 300.0, -20.0, 9201)));

    TransactionResult result = bank.withdraw(9201, dollarsToCents(310.0), true);
    const Account *account = bank.findAccount(9201);

    XCTAssertTrue(result.succeeded());
    XCTAssertEqual(result.amount, dollarsToCents(310.0));
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, dollarsToCents(-10.0));
    XCTAssertNotEqual(account, nullptr);
    XCTAssertEqual(account->getBalanceInCents(), dollarsToCents(-10.0));
}

- (void)testCashDepositPromptRejectsCentAmounts {
    Bank bank;
    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Cash Deposit", 100.0, 0.03, 9401)));

    ScopedStandardStreamRedirect redirectedStreams(std::cin, std::cout, "9401\n1\n1\n10.50\n10\n");

    bank.depositPrompt();

    const Account *account = bank.findAccount(9401);
    XCTAssertNotEqual(account, nullptr);
    XCTAssertEqual(account->getBalanceInCents(), dollarsToCents(110.0));
    XCTAssertTrue(redirectedStreams.output().find("Please enter a whole-dollar amount with no cents.") != std::string::npos);
}

- (void)testCashWithdrawPromptRejectsCentAmounts {
    Bank bank;
    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Cash Withdraw", 100.0, 0.03, 9402)));

    ScopedStandardStreamRedirect redirectedStreams(std::cin, std::cout, "9402\n1\n10.25\n10\n2\n");

    bank.withdrawPrompt();

    const Account *account = bank.findAccount(9402);
    XCTAssertNotEqual(account, nullptr);
    XCTAssertEqual(account->getBalanceInCents(), dollarsToCents(90.0));
    XCTAssertTrue(redirectedStreams.output().find("Please enter a whole-dollar amount with no cents.") != std::string::npos);
}

- (void)testTransferUpdatesBothAccountBalances {
    Bank bank;
    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Transfer A", 100.0, 0.03, 9001)));
    XCTAssertTrue(bank.addAccount(std::make_unique<CheckingAccount>("Transfer B", 50.0, -20.0, 9002)));

    TransactionResult result = bank.transfer(9001, 9002, dollarsToCents(40.0));
    const Account *fromAccount = bank.findAccount(9001);
    const Account *toAccount = bank.findAccount(9002);

    XCTAssertTrue(result.succeeded());
    XCTAssertEqual(result.amount, dollarsToCents(40.0));
    XCTAssertTrue(result.hasSecondaryBalance);
    XCTAssertNotEqual(fromAccount, nullptr);
    XCTAssertNotEqual(toAccount, nullptr);
    XCTAssertEqual(result.balance, dollarsToCents(60.0));
    XCTAssertEqual(result.secondaryBalance, dollarsToCents(90.0));
    XCTAssertEqual(fromAccount->getBalanceInCents(), dollarsToCents(60.0));
    XCTAssertEqual(toAccount->getBalanceInCents(), dollarsToCents(90.0));
}

- (void)testTransferRejectsSameAccountRequests {
    Bank bank;
    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Transfer A", 100.0, 0.03, 9001)));

    TransactionResult result = bank.transfer(9001, 9001, dollarsToCents(10.0));

    XCTAssertEqual(result.status, TransactionStatus::sameAccount);
    XCTAssertFalse(result.hasSecondaryBalance);
}

- (void)testTransferRejectsSameAccountRequestsBeforeAccountLookup {
    Bank bank;

    TransactionResult result = bank.transfer(9999, 9999, dollarsToCents(10.0));

    XCTAssertEqual(result.status, TransactionStatus::sameAccount);
    XCTAssertEqual(result.amount, dollarsToCents(10.0));
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, 0);
}

- (void)testTransferReportsMissingAccount {
    Bank bank;
    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Transfer A", 100.0, 0.03, 9001)));

    TransactionResult result = bank.transfer(9001, 9999, dollarsToCents(10.0));

    XCTAssertEqual(result.status, TransactionStatus::accountNotFound);
    XCTAssertFalse(result.hasSecondaryBalance);
}

- (void)testTransferRejectsInvalidAmountBeforeAccountLookup {
    Bank bank;

    TransactionResult result = bank.transfer(9999, 8888, 0);

    XCTAssertEqual(result.status, TransactionStatus::invalidAmount);
    XCTAssertEqual(result.amount, 0);
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, 0);
}

- (void)testTransferReportsInsufficientFunds {
    Bank bank;
    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Transfer A", 60.0, 0.03, 9001)));
    XCTAssertTrue(bank.addAccount(std::make_unique<CheckingAccount>("Transfer B", 50.0, -20.0, 9002)));

    TransactionResult result = bank.transfer(9001, 9002, dollarsToCents(500.0));

    XCTAssertEqual(result.status, TransactionStatus::insufficientFunds);
    XCTAssertEqual(result.amount, dollarsToCents(500.0));
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, dollarsToCents(60.0));
}

- (void)testTransferRejectsInvalidAmountWithoutChangingBalances {
    Bank bank;
    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Transfer A", 60.0, 0.03, 9001)));
    XCTAssertTrue(bank.addAccount(std::make_unique<CheckingAccount>("Transfer B", 50.0, -20.0, 9002)));

    TransactionResult result = bank.transfer(9001, 9002, 0);
    const Account *fromAccount = bank.findAccount(9001);
    const Account *toAccount = bank.findAccount(9002);

    XCTAssertEqual(result.status, TransactionStatus::invalidAmount);
    XCTAssertEqual(result.amount, 0);
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertNotEqual(fromAccount, nullptr);
    XCTAssertNotEqual(toAccount, nullptr);
    XCTAssertEqual(result.balance, dollarsToCents(60.0));
    XCTAssertEqual(fromAccount->getBalanceInCents(), dollarsToCents(60.0));
    XCTAssertEqual(toAccount->getBalanceInCents(), dollarsToCents(50.0));
}

- (void)testTransferRejectsDestinationOverflowWithoutChangingBalances {
    Bank bank;
    XCTAssertTrue(bank.addAccount(std::make_unique<TestAccount>("Transfer Source", 500, 9501)));
    XCTAssertTrue(bank.addAccount(std::make_unique<TestAccount>(
        "Transfer Destination",
        std::numeric_limits<MoneyCents>::max() - 25,
        9502
    )));

    TransactionResult result = bank.transfer(9501, 9502, 50);
    const Account *fromAccount = bank.findAccount(9501);
    const Account *toAccount = bank.findAccount(9502);

    XCTAssertEqual(result.status, TransactionStatus::invalidAmount);
    XCTAssertEqual(result.amount, 50);
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertNotEqual(fromAccount, nullptr);
    XCTAssertNotEqual(toAccount, nullptr);
    XCTAssertEqual(result.balance, 500);
    XCTAssertEqual(result.message, std::string("The transfer would exceed the destination account's maximum supported balance."));
    XCTAssertEqual(fromAccount->getBalanceInCents(), 500);
    XCTAssertEqual(toAccount->getBalanceInCents(), std::numeric_limits<MoneyCents>::max() - 25);
}

- (void)testTransferRequestsOverdraftConfirmationForCheckingAccounts {
    Bank bank;
    XCTAssertTrue(bank.addAccount(std::make_unique<CheckingAccount>("Transfer Source", 300.0, -20.0, 9301)));
    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Transfer Destination", 50.0, 0.03, 9302)));

    TransactionResult result = bank.transfer(9301, 9302, dollarsToCents(310.0));
    const Account *sourceAccount = bank.findAccount(9301);
    const Account *destinationAccount = bank.findAccount(9302);

    XCTAssertEqual(result.status, TransactionStatus::overdraftConfirmationRequired);
    XCTAssertEqual(result.amount, dollarsToCents(310.0));
    XCTAssertFalse(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, dollarsToCents(-10.0));
    XCTAssertNotEqual(sourceAccount, nullptr);
    XCTAssertNotEqual(destinationAccount, nullptr);
    XCTAssertEqual(sourceAccount->getBalanceInCents(), dollarsToCents(300.0));
    XCTAssertEqual(destinationAccount->getBalanceInCents(), dollarsToCents(50.0));
}

- (void)testTransferAppliesApprovedOverdraftForCheckingAccounts {
    Bank bank;
    XCTAssertTrue(bank.addAccount(std::make_unique<CheckingAccount>("Transfer Source", 300.0, -20.0, 9301)));
    XCTAssertTrue(bank.addAccount(std::make_unique<SavingsAccount>("Transfer Destination", 50.0, 0.03, 9302)));

    TransactionResult result = bank.transfer(9301, 9302, dollarsToCents(310.0), true);
    const Account *sourceAccount = bank.findAccount(9301);
    const Account *destinationAccount = bank.findAccount(9302);

    XCTAssertTrue(result.succeeded());
    XCTAssertEqual(result.amount, dollarsToCents(310.0));
    XCTAssertTrue(result.hasSecondaryBalance);
    XCTAssertEqual(result.balance, dollarsToCents(-10.0));
    XCTAssertEqual(result.secondaryBalance, dollarsToCents(360.0));
    XCTAssertNotEqual(sourceAccount, nullptr);
    XCTAssertNotEqual(destinationAccount, nullptr);
    XCTAssertEqual(sourceAccount->getBalanceInCents(), dollarsToCents(-10.0));
    XCTAssertEqual(destinationAccount->getBalanceInCents(), dollarsToCents(360.0));
}

- (void)testSavingsInterestAccrualUpdatesBalance {
    SavingsAccount savings("Interest Test", 200.0, 0.05);

    savings.addInterest();

    XCTAssertEqual(savings.getBalanceInCents(), dollarsToCents(210.0));
}

- (void)testSavingsInterestRoundsFromFractionalCents {
    SavingsAccount savings("Interest Test", 10.10, 0.05);

    XCTAssertEqual(savings.getAccruedInterest(), 51);

    savings.addInterest();

    XCTAssertEqual(savings.getBalanceInCents(), dollarsToCents(10.61));
}

- (void)testSavingsInterestDoesNotChangeZeroBalanceAccount {
    SavingsAccount savings("Interest Test", 0.0, 0.05);

    XCTAssertEqual(savings.getAccruedInterest(), 0);

    savings.addInterest();

    XCTAssertEqual(savings.getBalanceInCents(), 0);
}

- (void)testSavingsInterestSaturatesAtMaximumBalance {
    SavingsAccount savings("Interest Test", std::numeric_limits<double>::max(), 0.05);

    savings.addInterest();

    XCTAssertEqual(savings.getBalanceInCents(), std::numeric_limits<MoneyCents>::max());
}

- (void)testSavingsNormalizesNegativeInterestInput {
    SavingsAccount savings("Interest Test", 200.0, -0.05);

    XCTAssertEqual(savings.getInterestRate(), 0.0);
    XCTAssertEqual(savings.getAccruedInterest(), 0);
}

- (void)testSavingsNormalizesNonFiniteInterestInput {
    SavingsAccount savings("Interest Test", 200.0, std::numeric_limits<double>::infinity());

    XCTAssertEqual(savings.getInterestRate(), 0.0);
    XCTAssertEqual(savings.getAccruedInterest(), 0);
}

- (void)testValidatedIntRejectsTrailingCharacters {
    ScopedStandardStreamRedirect redirectedStreams(std::cin, std::cout, "12abc\n15\n");

    int value = getValidatedInt(0);

    XCTAssertEqual(value, 15);
    XCTAssertTrue(redirectedStreams.output().find("Invalid entry.") != std::string::npos);
}

- (void)testValidatedMoneyRejectsTrailingCharacters {
    ScopedStandardStreamRedirect redirectedStreams(std::cin, std::cout, "25.00usd\n25.75\n");

    MoneyCents value = getValidatedMoneyCents(kMinimumTransactionAmountCents);

    XCTAssertEqual(value, dollarsToCents(25.75));
    XCTAssertTrue(redirectedStreams.output().find("Invalid entry.") != std::string::npos);
}

- (void)testValidatedMoneyRejectsMoreThanTwoDecimalPlaces {
    ScopedStandardStreamRedirect redirectedStreams(std::cin, std::cout, "25.999\n25.75\n");

    MoneyCents value = getValidatedMoneyCents(kMinimumTransactionAmountCents);

    XCTAssertEqual(value, dollarsToCents(25.75));
    XCTAssertTrue(redirectedStreams.output().find("Invalid entry.") != std::string::npos);
}

- (void)testValidatedMoneyAcceptsLeadingDecimalSyntax {
    ScopedStandardStreamRedirect redirectedStreams(std::cin, std::cout, ".50\n");

    MoneyCents value = getValidatedMoneyCents(kMinimumTransactionAmountCents);

    XCTAssertEqual(value, 50);
    XCTAssertTrue(redirectedStreams.output().empty());
}

- (void)testValidatedChoiceRejectsOutOfRangeAndMixedInput {
    ScopedStandardStreamRedirect redirectedStreams(std::cin, std::cout, "2xyz\n5\n2\n");

    int value = getValidatedChoice(1, 3);

    XCTAssertEqual(value, 2);
    XCTAssertTrue(redirectedStreams.output().find("Invalid entry.") != std::string::npos);
}

- (void)testTryAddMoneyCentsDetectsOverflow {
    MoneyCents result = 0;

    XCTAssertFalse(tryAddMoneyCents(std::numeric_limits<MoneyCents>::max(), 1, result));
    XCTAssertTrue(tryAddMoneyCents(125, 75, result));
    XCTAssertEqual(result, 200);
}

- (void)testTrySubtractMoneyCentsDetectsUnderflow {
    MoneyCents result = 0;

    XCTAssertFalse(trySubtractMoneyCents(std::numeric_limits<MoneyCents>::min(), 1, result));
    XCTAssertFalse(trySubtractMoneyCents(0, std::numeric_limits<MoneyCents>::min(), result));
    XCTAssertTrue(trySubtractMoneyCents(125, 75, result));
    XCTAssertEqual(result, 50);
}

- (void)testDollarsToCentsNormalizesNonFiniteInput {
    XCTAssertEqual(dollarsToCents(std::numeric_limits<double>::quiet_NaN()), 0);
    XCTAssertEqual(dollarsToCents(std::numeric_limits<double>::infinity()), 0);
    XCTAssertEqual(dollarsToCents(-std::numeric_limits<double>::infinity()), 0);
}

- (void)testDollarsToCentsSaturatesOversizedInput {
    XCTAssertEqual(dollarsToCents(std::numeric_limits<double>::max()), std::numeric_limits<MoneyCents>::max());
    XCTAssertEqual(dollarsToCents(-std::numeric_limits<double>::max()), std::numeric_limits<MoneyCents>::min());
}

- (void)testCalculatePercentageOfCentsNormalizesNonFiniteRates {
    XCTAssertEqual(calculatePercentageOfCents(1000, std::numeric_limits<double>::quiet_NaN()), 0);
    XCTAssertEqual(calculatePercentageOfCents(1000, std::numeric_limits<double>::infinity()), 0);
    XCTAssertEqual(calculatePercentageOfCents(1000, -std::numeric_limits<double>::infinity()), 0);
}

- (void)testCalculatePercentageOfCentsSaturatesOversizedRates {
    XCTAssertEqual(calculatePercentageOfCents(1000, std::numeric_limits<double>::max()), std::numeric_limits<MoneyCents>::max());
    XCTAssertEqual(calculatePercentageOfCents(-1000, std::numeric_limits<double>::max()), std::numeric_limits<MoneyCents>::min());
}

- (void)testFormatCentsPlacesNegativeSignBeforeCurrencySymbol {
    XCTAssertEqual(formatCents(dollarsToCents(-20.5)), std::string("-$20.50"));
    XCTAssertEqual(formatCents(dollarsToCents(20.5)), std::string("$20.50"));
}

- (void)testFormatCentsPreservesSingleCentPrecision {
    XCTAssertEqual(formatCents(1), std::string("$0.01"));
    XCTAssertEqual(formatCents(-1), std::string("-$0.01"));
}

@end
