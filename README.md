# Bank Account Simulator

A menu-driven C++ banking simulator built in Xcode.

## Overview

This project simulates common bank account operations through a console-based interface. It supports viewing accounts, making deposits, withdrawing money, transferring funds between accounts, and running built-in self-checks.

The simulator models both checking and savings accounts and displays account-specific details such as balances, overdraft limits, interest rates, and accrued interest.

## Features

- Display all accounts
- Deposit funds into an account
- Withdraw funds from an account
- Transfer funds between accounts
- Run self-checks to verify behavior
- Support for both checking and savings accounts
- Account confirmation prompts before transactions
- Withdrawal denomination selection
- Execution duration reporting

## Example operations shown

The current implementation demonstrates:

- listing large sets of accounts
- successful deposit processing
- successful withdrawal processing
- successful transfer processing
- passing self-checks

Example output includes:

- account number lookup and confirmation
- check/cash deposit handling
- withdrawal balance updates
- transfer source and destination balance updates
- self-check validation

## Project structure

Main source files include:

- `main.cpp`
- `Account.h` / `Account.cpp`
- `Bank.h` / `Bank.cpp`
- `CheckingAccount.h` / `CheckingAccount.cpp`
- `SavingsAccount.h` / `SavingsAccount.cpp`
- `Utilities.h` / `Utilities.cpp`
- `Money.h`
- `SelfChecks.h`

## Build and run

### In Xcode
1. Open the Xcode project.
2. Select the `Bank Account Simulator` scheme.
3. Build and run the project.
4. Interact with the simulator through the console.

## Screenshot

![Bank Account Simulator running in Xcode] <img width="1680" height="1050" alt="Screenshot 2026-05-13 at 19 57 04" src="https://github.com/user-attachments/assets/6f342c8c-f536-4d81-9224-756aeb8a5c8d" />

## Notes

This project is designed as a C++ practice project focused on object-oriented design, account modeling, and transaction workflows in a console application.
