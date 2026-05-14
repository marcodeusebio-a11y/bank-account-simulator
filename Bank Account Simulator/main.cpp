//
//  main.cpp
//  Bank Account
//
//  Created by Marco D’Eusebio on 4/19/26.
//

#include <iostream>
#include <memory>
#include <chrono>

#include <string_view>
#include "SelfChecks.h"
#include "Utilities.h"
#include "Bank.h"
#include "SavingsAccount.h"
#include "CheckingAccount.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

namespace {
[[nodiscard]]
bool seedInitialAccounts(Bank& bank) {

    // Curated Realistic Seed Data
    // Note: Some account owners have savings, some checking, some both
    struct SeedOwner {
        const char* name;
        double savingsBalance;     // 0.0 if none
        double savingsRate;        // ignored if savingsBalance == 0
        double checkingBalance;    // 0.0 if none
        double overdraftLimit;     // ignored if checkingBalance == 0
    };

    const SeedOwner owners[] = {
        {"Avery Lopez",            1820.50, 0.02,    0.0,     0.0},
        {"Mateo Castillo",            0.0,   0.00,  740.00,  -50.0},
        {"Nora Patel",              1200.00, 0.03,  400.00, -100.0}, // both
        {"Zoe Chen",                 350.25, 0.015,   0.0,     0.0},
        {"Liam Johnson",               0.0,   0.00, 2200.00, -200.0},
        {"Isabella Rossi",           9800.00, 0.01,   0.0,     0.0},
        {"Ethan Rodriguez",            60.75, 0.04,   0.0,     0.0},
        {"Sophia Martinez",          1450.00, 0.025, 300.00,  -40.0}, // both
        {"Noah Kim",                    0.0,   0.00,  120.00,  -20.0},
        {"Mia Thompson",              500.00, 0.02,    0.0,     0.0},
        {"Aiden Clark",               250.00, 0.035,   0.0,     0.0},
        {"Olivia Nguyen",               0.0,   0.00,  980.00,  -80.0},
        {"Lucas Brown",               725.00, 0.03,  150.00,  -60.0}, // both
        {"Emma Davis",                 90.10, 0.02,    0.0,     0.0},
        {"Amelia Wilson",            2100.00, 0.018,   0.0,     0.0},
        {"James Garcia",                0.0,   0.00,  450.00,  -50.0},
        {"Benjamin Lee",              320.75, 0.022,   0.0,     0.0},
        {"Charlotte Hernandez",         0.0,   0.00, 1600.00, -150.0},
        {"Elijah Walker",             1300.00, 0.028, 500.00,  -70.0}, // both
        {"Harper Young",              410.00, 0.03,    0.0,     0.0},
        {"Michael Scott",               0.0,   0.00,  300.00,  -30.0},
        {"Ava Rivera",                875.25, 0.015,   0.0,     0.0},
        {"Evelyn Carter",            1525.00, 0.02,    0.0,     0.0},
        {"Henry Perez",                 0.0,   0.00,  50.00,   -20.0},
        {"Sofia Flores",              605.00, 0.027,   0.0,     0.0},
        {"Jackson Price",               0.0,   0.00,  780.00,  -90.0},
        {"Scarlett Brooks",           990.00, 0.031, 200.00,  -40.0}, // both
        {"Sebastian Murphy",          125.00, 0.02,    0.0,     0.0},
        {"Luna Ramirez",             2600.00, 0.012,   0.0,     0.0},
        {"William Torres",              0.0,   0.00, 1340.00, -120.0},
        {"Aria Nguyen",               340.00, 0.03,    0.0,     0.0},
        {"Daniel King",                 0.0,   0.00,  910.00,  -80.0},
        {"Grace Bennett",             720.50, 0.019,   0.0,     0.0},
        {"Matthew Gonzalez",            0.0,   0.00,  30.00,   -20.0},
        {"Victoria Rivera",          1875.00, 0.021, 600.00,  -60.0}, // both
        {"Joseph Howard",             410.00, 0.016,   0.0,     0.0},
        {"Chloe Ward",                540.00, 0.024,   0.0,     0.0},
        {"Samuel Cox",                  0.0,   0.00,  845.00, -110.0},
        {"Penelope Gray",            1120.00, 0.018,   0.0,     0.0},
        {"David Hughes",                0.0,   0.00,  400.00,  -50.0},
        {"Riley Simmons",             305.00, 0.032,   0.0,     0.0},
        {"Owen Butler",                 0.0,   0.00,  150.00,  -30.0},
        {"Zoey Foster",               980.00, 0.02,    0.0,     0.0},
        {"Wyatt Powell",                0.0,   0.00, 2200.00, -200.0},
        {"Nora Bryant",               650.00, 0.025, 350.00,  -40.0}, // both
        {"Levi Russell",              210.00, 0.017,   0.0,     0.0},
        {"Hannah Griffin",           1440.00, 0.014,   0.0,     0.0},
        {"Eli Hayes",                   0.0,   0.00,  510.00,  -60.0},
        {"Bella Coleman",             385.00, 0.029,   0.0,     0.0},
        {"Luke Perry",                  0.0,   0.00,  95.00,   -20.0},
        {"Priya Shah",               1345.00, 0.023,   0.0,     0.0},
        {"Diego Alvarez",               0.0,   0.00,  680.00,  -60.0},
        {"Yara Haddad",               2200.00, 0.017, 250.00,  -40.0}, // both
        {"Konrad Nowak",               415.75, 0.02,    0.0,     0.0},
        {"Anika Mehta",               178.20, 0.028,   0.0,     0.0},
        {"Selin Yildiz",              905.00, 0.019,   0.0,     0.0},
        {"Jonas Berg",                  0.0,   0.00,  330.00,  -30.0},
        {"Aisha Abdi",               1510.00, 0.021,  90.00,   -20.0}, // both
        {"Tobias Schmidt",            240.00, 0.034,   0.0,     0.0},
        {"Camila Souza",                0.0,   0.00,  510.00,  -50.0},
        {"Rohan Iyer",                780.00, 0.027,   0.0,     0.0},
        {"Helena Petrova",              0.0,   0.00,  970.00,  -90.0},
        {"Min-joon Park",             600.00, 0.018,   0.0,     0.0},
        {"Nina Popescu",              1280.00, 0.022, 420.00,  -60.0}, // both
        {"Omar Farouk",                 0.0,   0.00,  250.00,  -20.0},
        {"Santiago Perez",            995.00, 0.02,    0.0,     0.0},
        {"Freya Johansson",           410.00, 0.029,   0.0,     0.0},
        {"Thiago Lima",                 0.0,   0.00, 1890.00, -150.0},
        {"Maeve O'Connor",           2300.00, 0.013, 300.00,  -40.0}, // both
        {"Abel Tesfaye",              150.00, 0.031,   0.0,     0.0},
        {"Greta Müller",                0.0,   0.00,  720.00,  -70.0},
        {"Jasper de Vries",           880.00, 0.024,   0.0,     0.0},
        {"Sofia Petrova",              0.0,   0.00,  410.00,  -40.0},
        {"Arjun Singh",              1750.00, 0.02,  200.00,  -30.0}, // both
        {"Elif Demir",                340.00, 0.026,   0.0,     0.0},
        {"Hassan Ali",                  0.0,   0.00,  560.00,  -60.0},
        {"Maya Cohen",               1185.00, 0.017,   0.0,     0.0},
        {"Kenji Sato",                 90.00, 0.033,   0.0,     0.0},
        {"Anastasia Ivanova",           0.0,   0.00,  80.00,   -20.0},
        {"Charles Dubois",           2040.00, 0.014, 800.00, -100.0}, // both
        {"Lucia Bianchi",             610.00, 0.019,   0.0,     0.0},
        {"Ahmed Hassan",                0.0,   0.00,  145.00,  -20.0},
        {"Ivy Zhang",                 990.00, 0.028,   0.0,     0.0},
        {"Paulina Kowalska",            0.0,   0.00,  360.00,  -30.0},
        {"Rafael Ortega",             745.00, 0.023, 120.00,  -20.0}, // both
        {"Noor El-Sayed",             210.00, 0.02,    0.0,     0.0},
        {"Igor Petrov",                 0.0,   0.00,  950.00,  -90.0},
        {"Tamara Novak",              420.00, 0.027,   0.0,     0.0},
        {"Hugo Silva",                  0.0,   0.00,  275.00,  -20.0},
        {"Beatriz Costa",            1325.00, 0.016,  50.00,   -20.0}  // both
    };

    for (const auto& o : owners) {
        if (o.savingsBalance > 0.0) {
            if (!bank.addAccount(std::make_unique<SavingsAccount>(o.name, o.savingsBalance, o.savingsRate))) {
                std::cerr << "Failed to add savings account for " << o.name << ".\n";
                return false;
            }
        }
        if (o.checkingBalance > 0.0) {
            if (!bank.addAccount(std::make_unique<CheckingAccount>(o.name, o.checkingBalance, o.overdraftLimit))) {
                std::cerr << "Failed to add checking account for " << o.name << ".\n";
                return false;
            }
        }
    }
    
    // Additional Seed Owners with Explicit Account Numbers
    // Note: These IDs should be unique and will advance automatic numbering for subsequent accounts
    struct ExplicitSeed { const char* name; double savingsBal; double savingsRate; int savingsID; double checkingBal; double overdraft; int checkingID; };
    
    // WARNING: Any new additions to explicitSeeds[]:
    //    ~ Must have IDs < 10000
    //    ~ Must NOT have IDs within the test ranges (9001-9801)
    
    const ExplicitSeed explicitSeeds[] = {
        {"Avery Morgan",        1825.50, 0.018, 8500,   0.0,    0.0,    0},
        {"Blake Lee",              0.0,  0.000,    0,  540.00, -40.0, 8501},
        {"Casey Patel",         1200.00, 0.022, 8502,  300.00, -60.0, 8503},
        {"Drew Garcia",          350.25, 0.015, 8504,   0.0,    0.0,    0},
        {"Emery Khan",             0.0,  0.000,    0, 2200.00,-150.0, 8505},
        {"Finley Nguyen",       9800.00, 0.010, 8506,   0.0,    0.0,    0},
        {"Gale Silva",            60.75, 0.030, 8507,   0.0,    0.0,    0},
        {"Harper Kowalski",    1450.00, 0.025, 8508,  260.00, -40.0, 8509},
        {"Indigo Santos",          0.0,  0.000,    0,  110.00, -20.0, 8510},
        {"Jordan Brown",        500.00,  0.020, 8511,   0.0,    0.0,    0},
        {"Alex Morgan",         275.00,  0.035, 8512,   0.0,    0.0,    0},
        {"Blake Garcia",           0.0,  0.000,    0,  980.00, -80.0, 8513},
        {"Casey Kim",           725.00,  0.028, 8514,  150.00, -40.0, 8515},
        {"Drew Davis",           95.10,  0.020, 8516,   0.0,    0.0,    0},
        {"Emery Wilson",       2100.00,  0.018, 8517,   0.0,    0.0,    0},
        {"Finley Lee",            0.0,  0.000,    0,  420.00, -50.0, 8518},
        {"Gale Hernandez",      320.75,  0.022, 8519,   0.0,    0.0,    0},
        {"Harper Walker",         0.0,  0.000,    0, 1600.00,-120.0, 8520},
        {"Indigo Young",       1300.00,  0.028, 8521,  480.00, -70.0, 8522},
        {"Jordan Scott",        410.00,  0.030, 8523,   0.0,    0.0,    0},
        {"Alex Rivera",           0.0,  0.000,    0,  300.00, -30.0, 8524},
        {"Blake Carter",        875.25,  0.015, 8525,   0.0,    0.0,    0},
        {"Casey Perez",        1525.00,  0.020, 8526,   0.0,    0.0,    0},
        {"Drew Flores",           0.0,  0.000,    0,   50.00, -20.0, 8527},
        {"Emery Price",         605.00,  0.027, 8528,   0.0,    0.0,    0},
        {"Finley Brooks",         0.0,  0.000,    0,  780.00, -90.0, 8529},
        {"Gale Murphy",         990.00,  0.031, 8530,  200.00, -40.0, 8531},
        {"Harper Ramirez",      125.00,  0.020, 8532,   0.0,    0.0,    0},
        {"Indigo Torres",      2600.00,  0.012, 8533,   0.0,    0.0,    0},
        {"Jordan Nguyen",         0.0,  0.000,    0, 1340.00,-120.0, 8534},
        {"Alex King",           340.00,  0.030, 8535,   0.0,    0.0,    0},
        {"Blake Bennett",         0.0,  0.000,    0,  910.00, -80.0, 8536},
        {"Casey Gonzalez",      720.50,  0.019, 8537,   0.0,    0.0,    0},
        {"Drew Ward",             0.0,  0.000,    0,   30.00, -20.0, 8538},
        {"Emery Rivera",       1875.00,  0.021, 8539,  600.00, -60.0, 8540},
        {"Finley Howard",       410.00,  0.016, 8541,   0.0,    0.0,    0},
        {"Gale Gray",           540.00,  0.024, 8542,   0.0,    0.0,    0},
        {"Harper Cox",            0.0,  0.000,    0,  845.00,-110.0, 8543},
        {"Indigo Simmons",     1120.00,  0.018, 8544,   0.0,    0.0,    0},
        {"Jordan Butler",         0.0,  0.000,    0,  150.00, -30.0, 8545},
        {"Alex Foster",         980.00,  0.020, 8546,   0.0,    0.0,    0},
        {"Blake Powell",          0.0,  0.000,    0, 2200.00,-200.0, 8547},
        {"Casey Bryant",        650.00,  0.025, 8548,  350.00, -40.0, 8549},
        {"Drew Russell",        210.00,  0.017, 8550,   0.0,    0.0,    0},
        {"Emery Griffin",     1440.00,  0.014, 8551,   0.0,    0.0,    0},
        {"Finley Hayes",          0.0,  0.000,    0,  510.00, -60.0, 8552},
        {"Gale Coleman",        385.00,  0.029, 8553,   0.0,    0.0,    0},
        {"Harper Perry",          0.0,  0.000,    0,   95.00, -20.0, 8554},
        {"Indigo Shah",        1345.00,  0.023, 8555,   0.0,    0.0,    0},
        {"Jordan Alvarez",        0.0,  0.000,    0,  680.00, -60.0, 8556},
        {"Alex Haddad",        2200.00,  0.017, 8557,  250.00, -40.0, 8558},
        {"Blake Nowak",         415.75,  0.020, 8559,   0.0,    0.0,    0},
        {"Casey Mehta",         178.20,  0.028, 8560,   0.0,    0.0,    0},
        {"Drew Yildiz",         905.00,  0.019, 8561,   0.0,    0.0,    0},
        {"Emery Berg",            0.0,  0.000,    0,  330.00, -30.0, 8562},
        {"Finley Abdi",        1510.00,  0.021, 8563,   90.00, -20.0, 8564},
        {"Gale Schmidt",        240.00,  0.034, 8565,   0.0,    0.0,    0},
        {"Harper Souza",          0.0,  0.000,    0,  510.00, -50.0, 8566},
        {"Indigo Iyer",         780.00,  0.027, 8567,   0.0,    0.0,    0},
        {"Jordan Petrova",        0.0,  0.000,    0,  970.00, -90.0, 8568},
        {"Alex Park",           600.00,  0.018, 8569,   0.0,    0.0,    0},
        {"Blake Popescu",      1280.00,  0.022, 8570,  420.00, -60.0, 8571},
        {"Casey Farouk",          0.0,  0.000,    0,  250.00, -20.0, 8572},
        {"Drew Perez",          995.00,  0.020, 8573,   0.0,    0.0,    0},
        {"Emery Johansson",     410.00,  0.029, 8574,   0.0,    0.0,    0},
        {"Finley Lima",           0.0,  0.000,    0, 1890.00,-150.0, 8575},
        {"Gale O'Connor",      2300.00,  0.013, 8576,  300.00, -40.0, 8577},
        {"Harper Tesfaye",      150.00,  0.031, 8578,   0.0,    0.0,    0},
        {"Indigo Müller",         0.0,  0.000,    0,  720.00, -70.0, 8579},
        {"Jordan de Vries",     880.00,  0.024, 8580,   0.0,    0.0,    0},
        {"Alex Petrova",          0.0,  0.000,    0,  410.00, -40.0, 8581},
        {"Blake Singh",        1750.00,  0.020, 8582,  200.00, -30.0, 8583},
        {"Casey Demir",         340.00,  0.026, 8584,   0.0,    0.0,    0},
        {"Drew Ali",              0.0,  0.000,    0,  560.00, -60.0, 8585},
        {"Emery Cohen",        1185.00,  0.017, 8586,   0.0,    0.0,    0},
        {"Finley Sato",          90.00,  0.033, 8587,   0.0,    0.0,    0},
        {"Gale Ivanova",          0.0,  0.000,    0,   80.00, -20.0, 8588},
        {"Harper Dubois",      2040.00,  0.014, 8589,  800.00,-100.0, 8590},
        {"Indigo Bianchi",      610.00,  0.019, 8591,   0.0,    0.0,    0},
        {"Jordan Hassan",         0.0,  0.000,    0,  145.00, -20.0, 8592},
        {"Alex Zhang",          990.00,  0.028, 8593,   0.0,    0.0,    0},
        {"Blake Kowalska",        0.0,  0.000,    0,  360.00, -30.0, 8594},
        {"Casey Ortega",        745.00,  0.023, 8595,  120.00, -20.0, 8596},
        {"Drew El-Sayed",       210.00,  0.020, 8597,   0.0,    0.0,    0},
        {"Emery Petrov",          0.0,  0.000,    0,  950.00, -90.0, 8598},
        {"Finley Novak",        420.00,  0.027, 8599,   0.0,    0.0,    0},
        {"Gale Silva",            0.0,  0.000,    0,  275.00, -20.0, 8800},
        {"Harper Costa",       1325.00,  0.016, 8801,   50.00, -20.0, 8802},
        {"Indigo Lopez",       1820.00,  0.019, 8803,   0.0,    0.0,    0},
        {"Jordan Castillo",       0.0,  0.000,    0,  740.00, -50.0, 8804},
        {"Alex Patel",         1200.00,  0.030, 8805,  400.00,-100.0, 8806},
        {"Blake Chen",          350.25,  0.015, 8807,   0.0,    0.0,    0},
        {"Casey Johnson",         0.0,  0.000,    0, 2200.00,-200.0, 8808},
        {"Drew Rossi",         9800.00,  0.010, 8809,   0.0,    0.0,    0},
        {"Emery Rodriguez",      60.75,  0.040, 8810,   0.0,    0.0,    0},
        {"Finley Martinez",    1450.00,  0.025, 8811,  300.00, -40.0, 8812},
        {"Gale Kim",              0.0,  0.000,    0,  120.00, -20.0, 8813},
        {"Harper Thompson",     500.00,  0.020, 8814,   0.0,    0.0,    0},
        {"Indigo Clark",        250.00,  0.035, 8815,   0.0,    0.0,    0},
        {"Jordan Nguyen",         0.0,  0.000,    0,  980.00, -80.0, 8816},
        {"Alex Brown",          725.00,  0.030, 8817,  150.00, -60.0, 8818},
        {"Blake Davis",          90.10,  0.020, 8819,   0.0,    0.0,    0},
        {"Casey Wilson",      2100.00,  0.018, 8820,   0.0,    0.0,    0},
        {"Drew Garcia",           0.0,  0.000,    0,  450.00, -50.0, 8821},
        {"Emery Lee",           320.75,  0.022, 8822,   0.0,    0.0,    0},
        {"Finley Hernandez",      0.0,  0.000,    0, 1600.00,-150.0, 8823},
        {"Gale Walker",        1300.00,  0.028, 8824,  500.00, -70.0, 8825},
        {"Harper Young",        410.00,  0.030, 8826,   0.0,    0.0,    0},
        {"Quinn Harper",        725.00, 0.020, 9701, 180.00, -40.0, 9702},
        {"Rowan Patel",        1540.00, 0.025, 9703,   0.0,    0.0,    0},
        {"Sienna Morales",        0.0,  0.000,    0, 640.00, -60.0, 9704},
        {"Dmitri Volkov",      2080.00, 0.018, 9705,   0.0,    0.0,    0},
        {"Marco D'Eusebio",       0.0,  0.000,    0,15299.73,-100.0, 9999},
        {"Marco D'Eusebio",   4789372.55,0.042,10000,  0.0,    0.0,    0}
    };

    for (const auto& e : explicitSeeds) {
        if (e.savingsID > 0) {
            if (!bank.addAccount(std::make_unique<SavingsAccount>(e.name, e.savingsBal, e.savingsRate, e.savingsID))) {
                std::cerr << "Failed to add explicit savings for " << e.name << " (#" << e.savingsID << ").\n";
                return false;
            }
        }
        if (e.checkingID > 0) {
            if (!bank.addAccount(std::make_unique<CheckingAccount>(e.name, e.checkingBal, e.overdraft, e.checkingID))) {
                std::cerr << "Failed to add explicit checking for " << e.name << " (#" << e.checkingID << ").\n";
                return false;
            }
        }
    }

    return true;
}

void printMenu() {
    std::cout << "\n--- Bank Account Simulator ---\n";
    std::cout << "1. Display all accounts\n";
    std::cout << "2. Deposit\n";
    std::cout << "3. Withdraw\n";
    std::cout << "4. Transfer\n";
    std::cout << "5. Run self-checks\n";
    std::cout << "0. Exit\n\n";
    std::cout << "Choice: ";
}

void announceSelection(std::string_view selectionText) {
    std::cout << "You selected:\n";
    std::cout << ' ' << selectionText << '\n';
}
} // namespace

int main() {
    Bank bank;
    int choice;

    if (!seedInitialAccounts(bank)) {
        return 1;
    }

    do {
        printMenu();
        
        choice = getValidatedChoice(0, 5);
        std::cout << std::endl;
        
        auto t1 = high_resolution_clock::now();
        
        switch (choice) {
            case 1:
                bank.displayAllAccounts();
                break;
                
            case 2:
                announceSelection("2. Deposit");
                bank.depositPrompt();
                break;
                
            case 3:
                announceSelection("3. Withdraw");
                bank.withdrawPrompt();
                break;
                
            case 4:
                announceSelection("4. Transfer");
                bank.transferPrompt();
                break;
                
            case 5:
                announceSelection("5. Run self-checks");
                if (!runSelfChecks()) {
                    std::cout << "Please review the reported self-check failures.\n";
                }
                break;
                
            case 0:
                std::cout << "Goodbye!\n";
                break;
        }
        
        auto t2 = high_resolution_clock::now();
        
        duration<double, std::milli> ms_double = t2 - t1;
        std::cout << "\nProgram Execution Duration: " << ms_double.count() << "ms\n";
        
    } while (choice != 0);

    return 0;
}

