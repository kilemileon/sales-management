#include <iostream>
#include <mysql.h>
#include <mysqld_error.h>
#include <string>
#include <limits> // Include for numeric_limits

using namespace std;

// Database credentials
const char HOST[] = "localhost";
const char USER[] = "root";
const char PASS[] = "pas1234!";
const char DB[] = "beverages_db"; // Assuming the database is named "beverages_db"

// Function to execute a query and handle errors
bool execute_query(MYSQL* obj, const string& query) {
    if (mysql_query(obj, query.c_str())) {
        cout << "ERROR: " << mysql_error(obj) << endl;
        return false;
    }
    return true;
}

// Function to display all cleared debts
void display_cleared_debts(MYSQL* obj) {
    const string query = "SELECT id, debtor_name, amount_of_debt, beverage_name, phone_number, date_of_debt, date_of_clearance FROM cleared_debts";

    if (!execute_query(obj, query)) return;

    MYSQL_RES* res = mysql_store_result(obj);
    if (!res) {
        cout << "ERROR: Failed to store result: " << mysql_error(obj) << endl;
        return;
    }

    MYSQL_ROW row;
    cout << "Cleared Debts:" << endl;
    cout << "ID | Debtor Name | Amount | Beverage | Phone Number | Date of Debt | Date of Clearance" << endl;
    cout << "-------------------------------------------------------------------------------" << endl;

    while ((row = mysql_fetch_row(res))) {
        cout << row[0] << " | "   // ID
             << row[1] << " | "   // Debtor Name
             << row[2] << " | "   // Amount
             << row[3] << " | "   // Beverage
             << row[4] << " | "   // Phone Number
             << row[5] << " | "   // Date of Debt
             << row[6] << endl;    // Date of Clearance
    }

    mysql_free_result(res);
}

// Function to display all cleared deficits
void display_cleared_deficits(MYSQL* obj) {
    const string query = "SELECT id, deficit_amount, beverage_name, date_of_deficit, date_of_clearance FROM cleared_deficits";

    if (!execute_query(obj, query)) return;

    MYSQL_RES* res = mysql_store_result(obj);
    if (!res) {
        cout << "ERROR: Failed to store result: " << mysql_error(obj) << endl;
        return;
    }

    MYSQL_ROW row;
    cout << "Cleared Deficits:" << endl;
    cout << "ID | Amount | Beverage | Date of Deficit | Date of Clearance" << endl;
    cout << "--------------------------------------------------------------" << endl;

    while ((row = mysql_fetch_row(res))) {
        cout << row[0] << " | "   // ID
             << row[1] << " | "   // Amount
             << row[2] << " | "   // Beverage
             << row[3] << " | "   // Date of Deficit
             << row[4] << endl;    // Date of Clearance
    }

    mysql_free_result(res);
}

// Main function to manage logging
int main() {
    MYSQL* obj; // MySQL object for connection
    int choice;

    // Initialize MySQL connection object
    if (!(obj = mysql_init(NULL))) {
        cout << "ERROR: MySQL object could not be created." << endl;
        return 1;
    }

    // Connect to the database
    obj = mysql_real_connect(obj, HOST, USER, PASS, DB, 0, NULL, 0);
    if (!obj) {
        cout << "ERROR: Database connection failed!" << endl;
        cout << mysql_error(obj) << endl;
        return 1;
    }

    cout << "Connected to the database successfully!" << endl;

    while (true) {
        cout << "\nChoose an option:" << endl;
        cout << "1. Display Cleared Debts" << endl;
        cout << "2. Display Cleared Deficits" << endl;
        cout << "3. Exit" << endl;
        cout << "Enter your choice: ";

        // Validate input
        if (!(cin >> choice)) {
            cout << "Invalid input. Please enter a number." << endl;
            cin.clear(); // Clear error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard invalid input
            continue;
        }

        switch (choice) {
            case 1:
                display_cleared_debts(obj);
                break;
            case 2:
                display_cleared_deficits(obj);
                break;
            case 3:
                cout << "Bye!!" << endl; // Exit message
                mysql_close(obj);
                cout << "BYE!!" << endl; // Final goodbye message
                return 0;
            default:
                cout << "Invalid choice. Please try again." << endl;
                break;
        }
    }

    // Close the MySQL connection
    mysql_close(obj);
    cout << "BYE!!" << endl; // Final goodbye message
    return 0;
}

