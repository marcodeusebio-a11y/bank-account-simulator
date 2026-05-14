//
//  SavingsAccount.h
//  Bank Account
//
//  Created by Marco D’Eusebio on 4/27/26.
//

#ifndef SAVINGSACCOUNT_H
#define SAVINGSACCOUNT_H

#include "Account.h"

class SavingsAccount final : public Account {
private:
    double m_interestRate = 0.0;

public:
    SavingsAccount(std::string_view ownerName, double balance, double interestRate, int accountNumber = 0);
    
    void addInterest();
    
    [[nodiscard]]
    double getInterestRate() const noexcept;
    [[nodiscard]]
    MoneyCents getAccruedInterest() const noexcept;
    
    void display() const override;
};

#endif
