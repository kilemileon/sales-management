#include <iostream>
#include <mysql.h>
#include <mysqld_error.h>
#include <string>
#include <iomanip> // For formatting output
#include <ctime>   // For getting current date

using namespace std;

// Database credentials
const char HOST[] = "localhost"; 
const char USER[] = "root";
const char PASS[] = "pas1234!";
const char DB[] = "beverages_db"; 

// Struct for Debt
struct Debt {
    string debtor_name;
    double amount_of_debt;
    string beverage_name;   
    string phone_number;
    string date_of_debt;    
};

// Struct for Deficit
struct Deficit {
    int id; // Unique ID for the deficit
    double amount;
    string beverage;
    string date;
    string date_of_clearance; // Added date_of_clearance field
};

// Function to get current date as a string
string get_current_date() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    
    char date[11]; // YYYY-MM-DD format
    sprintf(date, "%04d-%02d-%02d", ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday);
    
    return string(date);
}

// Function to clear deficit
void clear_deficit(MYSQL* obj) {
    int number_of_deficits;
    cout << "How many deficits do you want to clear? ";
    cin >> number_of_deficits;

    for (int i = 0; i < number_of_deficits; ++i) {
        int deficit_id;
        cout << "Enter the ID of the deficit to clear: ";
        cin >> deficit_id;

        // Move deficit to cleared_deficits table before deletion
        char move_query[512];
        sprintf(move_query, 
            "INSERT INTO cleared_deficits (id, deficit_amount, beverage_name, date_of_deficit, date_of_clearance) "
            "SELECT id, deficit_amount, beverage_name, date_of_deficit, '%s' FROM deficits WHERE id = %d", 
            get_current_date().c_str(), deficit_id);

        if (mysql_query(obj, move_query)) {
            cout << "ERROR: Failed to move deficit to cleared_deficits." << endl;
            cout << mysql_error(obj) << endl;
            return;
        }

        // Query to delete the deficit based on ID
        char consult[256];
        sprintf(consult, "DELETE FROM deficits WHERE id = %d", deficit_id);

        if (mysql_query(obj, consult)) {
            cout << "ERROR: Failed to clear deficit." << endl;
            cout << mysql_error(obj) << endl;
        } else {
            cout << "Deficit cleared successfully." << endl;
        }
    }
}

// Function to clear debt
void clear_debt(MYSQL* obj) {
    int number_of_debts;
    cout << "How many debts do you want to clear? ";
    cin >> number_of_debts;

    for (int i = 0; i < number_of_debts; ++i) {
        int debt_id;
        cout << "Enter the ID of the debt to clear: ";
        cin >> debt_id;

        // Get the current date
        string clearance_date = get_current_date();

        // Move debt to cleared_debts table before deletion
        char move_query[512];
        sprintf(move_query, 
            "INSERT INTO cleared_debts (id, debtor_name, amount_of_debt, beverage_name, phone_number, date_of_debt, date_of_clearance) "
            "SELECT id, debtor_name, amount_of_debt, beverage_name, phone_number, date_of_debt, '%s' FROM debts WHERE id = %d", 
            clearance_date.c_str(), debt_id);

        if (mysql_query(obj, move_query)) {
            cout << "ERROR: Failed to move debt to cleared_debts." << endl;
            cout << mysql_error(obj) << endl;
            return;
        }

        // Query to delete the debt based on ID
        char consult[256];
        sprintf(consult, "DELETE FROM debts WHERE id = %d", debt_id);

        if (mysql_query(obj, consult)) {
            cout << "ERROR: Failed to clear debt." << endl;
            cout << mysql_error(obj) << endl;
        } else {
            cout << "Debt cleared successfully." << endl;
        }
    }
}

// Function to manage deficits
void manage_deficits(MYSQL* obj) {
    int number_of_deficits;
    cout << "How many deficits do you want to add? ";
    cin >> number_of_deficits;
    cin.ignore(100, '\n'); // Clear input buffer

    for (int i = 0; i < number_of_deficits; ++i) {
        Deficit deficit;

        // Retrieve the last used ID
        int last_id = 0;
        MYSQL_RES* res;
        MYSQL_ROW row;

        if (mysql_query(obj, "SELECT MAX(id) FROM deficits")) {
            cout << "ERROR: Failed to fetch last ID." << endl;
            cout << mysql_error(obj) << endl;
            return;
        }

        res = mysql_store_result(obj);
        if (res) {
            row = mysql_fetch_row(res);
            last_id = row[0] ? atoi(row[0]) : 0; // Get the last used ID, default to 0 if no records exist
            mysql_free_result(res);
        }

        // Assign the new ID
        deficit.id = last_id + 1;

        cout << "Enter the amount of deficit: ";
        cin >> deficit.amount;
        cin.ignore(100, '\n'); // Clear input buffer

        cout << "Enter the beverage causing the deficit: ";
        getline(cin, deficit.beverage);

        cout << "Enter the date of deficit (YYYY-MM-DD): ";
        getline(cin, deficit.date); // Get date input

        // Insert deficit into the database with the correct column names
        char consult[512];
        sprintf(consult, "INSERT INTO deficits (id, deficit_amount, beverage_name, date_of_deficit) VALUES (%d, %.2f, '%s', '%s')",
                deficit.id, deficit.amount, deficit.beverage.c_str(), deficit.date.c_str());

        if (mysql_query(obj, consult)) {
            cout << "ERROR: Failed to add deficit." << endl;
            cout << mysql_error(obj) << endl;
        } else {
            cout << "Deficit added successfully with ID: " << deficit.id << endl; // Show the assigned ID
        }
    }

    // Ask if user wants to clear a deficit
    char clear_deficit_option;
    cout << "Do you want to clear a deficit? (y/n): ";
    cin >> clear_deficit_option;
    if (clear_deficit_option == 'y' || clear_deficit_option == 'Y') {
        clear_deficit(obj);
    }
}

// Function to manage debts
void manage_debts(MYSQL* obj) {
    int number_of_debts;
    cout << "How many debts do you want to add? ";
    cin >> number_of_debts;
    cin.ignore(100, '\n'); // Clear input buffer

    for (int i = 0; i < number_of_debts; ++i) {
        Debt debt;

        cout << "Enter the name of the debtor: ";
        getline(cin, debt.debtor_name);

        cout << "Enter the amount of debt: ";
        cin >> debt.amount_of_debt;
        cin.ignore(100, '\n'); // Clear input buffer

        cout << "Enter the beverage(s) leading to the debt (comma-separated): ";
        getline(cin, debt.beverage_name);

        cout << "Enter the phone number of the debtor: ";
        getline(cin, debt.phone_number);

        cout << "Enter the date of debt (YYYY-MM-DD): ";
        getline(cin, debt.date_of_debt);

        // Retrieve the last used ID
        int last_id = 0;
        MYSQL_RES* res;
        MYSQL_ROW row;

        if (mysql_query(obj, "SELECT MAX(id) FROM debts")) {
            cout << "ERROR: Failed to fetch last ID." << endl;
            cout << mysql_error(obj) << endl;
            return;
        }

        res = mysql_store_result(obj);
        if (res) {
            row = mysql_fetch_row(res);
            last_id = row[0] ? atoi(row[0]) : 0; // Get the last used ID, default to 0 if no records exist
            mysql_free_result(res);
        }

        // Insert debt into the database with the new ID
        char consult[512];
        sprintf(consult, 
            "INSERT INTO debts (id, debtor_name, amount_of_debt, beverage_name, phone_number, date_of_debt) VALUES (%d, '%s', %.2f, '%s', '%s', '%s')",
            last_id + 1, debt.debtor_name.c_str(), debt.amount_of_debt, debt.beverage_name.c_str(), debt.phone_number.c_str(), debt.date_of_debt.c_str());

        if (mysql_query(obj, consult)) {
            cout << "ERROR: Failed to add debt." << endl;
            cout << mysql_error(obj) << endl;
        } else {
            cout << "Debt added successfully with ID: " << last_id + 1 << endl;
        }
    }

    // Ask if user wants to clear a debt
    char clear_debt_option;
    cout << "Do you want to clear a debt? (y/n): ";
    cin >> clear_debt_option;
    if (clear_debt_option == 'y' || clear_debt_option == 'Y') {
        clear_debt(obj);
    }
}

int main() {
    MYSQL* obj; // MySQL object for connection
    char type;

    // Initialize MySQL connection object
    if (!(obj = mysql_init(0))) {
        cout << "ERROR: MySQL object could not be created." << endl;
        cout << "BYE!!" << endl;
        return 1;
    }

    // Connect to the MySQL database
    if (!mysql_real_connect(obj, HOST, USER, PASS, DB, 3306, NULL, 0)) {
        cout << "ERROR: Failed to connect to database." << endl;
        cout << mysql_error(obj) << endl;
        cout << "BYE!!" << endl;
        return 1;
    } else {
        cout << "Connected to the database." << endl << endl;
    }

    // Main menu for debt/deficit management
    cout << "Is this a debt (d) or a deficit (f)? ";
    cin >> type;
    cin.ignore(100, '\n'); // Clear input buffer

    if (type == 'f' || type == 'F') {
        manage_deficits(obj);
    } else if (type == 'd' || type == 'D') {
        manage_debts(obj);
    } else {
        cout << "Invalid option selected." << endl;
    }

    // Close MySQL connection
    mysql_close(obj);
    cout << "BYE!!" << endl; // Ensure "BYE!!" is printed on normal exit
    return 0;
}

