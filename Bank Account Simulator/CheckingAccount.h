//
//  CheckingAccount.h
//  Bank Account
//
//  Created by Marco D’Eusebio on 4/27/26.
//

#ifndef CHECKINGACCOUNT_H
#define CHECKINGACCOUNT_H

#include "Account.h"

class CheckingAccount final : public Account {
private:
    MoneyCents m_overdraftLimit = dollarsToCents(-20.0);

public:
    CheckingAccount(std::string_view ownerName, double balance, double overdraftLimit, int accountNumber = 0);

    [[nodiscard]]
    double getOverdraftLimit() const noexcept;
    [[nodiscard]]
    MoneyCents getOverdraftLimitInCents() const noexcept;
    
    [[nodiscard]]
    TransactionResult withdraw(MoneyCents amountCents, bool allowOverdraft = false) override;

    void display() const override;
};

#endif
