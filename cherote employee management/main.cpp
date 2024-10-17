#include <iostream>
#include <mysql.h>
#include <mysqld_error.h>
#include <string>
#include <ctime>   // For getting current date
#include <iomanip> // For formatting output

using namespace std;

// Database credentials
const char HOST[] = "localhost";
const char USER[] = "root";
const char PASS[] = "pas1234!";
const char DB[] = "beverages_db";

// Function to get the current date as a string
string get_current_date() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char date[11]; // YYYY-MM-DD format
    sprintf(date, "%04d-%02d-%02d", ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday);
    return string(date);
}

// Function to execute a query and handle errors
bool execute_query(MYSQL* obj, const string& query) {
    if (mysql_query(obj, query.c_str())) {
        cout << "ERROR: " << mysql_error(obj) << endl;
        return false;
    }
    return true;
}

// Function to add an employee
void add_employee(MYSQL* obj) {
    string citizen_card_number, name, phone_number, date_of_entry;
    char date_option;

    cout << "Do you want to input the date manually or use the current date? (m: Manual, a: Automatic): ";
    cin >> date_option;
    cin.ignore(); // Clear input buffer

    if (date_option == 'm' || date_option == 'M') {
        cout << "Enter Date of Entry (YYYY-MM-DD): ";
        getline(cin, date_of_entry);
    } else {
        date_of_entry = get_current_date();  // Automatically set the current date
    }

    cout << "Enter Citizen Card Number: ";
    getline(cin, citizen_card_number);

    cout << "Enter Name: ";
    getline(cin, name);

    cout << "Enter Phone Number: ";
    getline(cin, phone_number);

    // Insert employee into the employees table
    char query[512];
    sprintf(query, "INSERT INTO employees (citizen_card_number, name, phone_number, date_of_entry) VALUES ('%s', '%s', '%s', '%s')",
            citizen_card_number.c_str(), name.c_str(), phone_number.c_str(), date_of_entry.c_str());

    if (execute_query(obj, query)) {
        cout << "Employee added successfully!" << endl;
    }
}

// Function to remove an employee using citizen card number
void remove_employee(MYSQL* obj) {
    string citizen_card_number, date_of_removal;
    char date_option;

    cout << "Enter Citizen Card Number of the employee to remove: ";
    getline(cin, citizen_card_number);

    // Ask the user if they want to set the date manually or automatically
    cout << "Do you want to input the removal date manually or use the current date? (m: Manual, a: Automatic): ";
    cin >> date_option;
    cin.ignore(); // Clear input buffer

    if (date_option == 'm' || date_option == 'M') {
        cout << "Enter Date of Removal (YYYY-MM-DD): ";
        getline(cin, date_of_removal);
    } else {
        date_of_removal = get_current_date();  // Automatically set the current date
    }

    // Query to select the employee details based on citizen card number
    char select_query[512];
    sprintf(select_query, "SELECT id, citizen_card_number, name, phone_number, date_of_entry FROM employees WHERE citizen_card_number='%s'",
            citizen_card_number.c_str());

    if (execute_query(obj, select_query)) {
        MYSQL_RES* res = mysql_store_result(obj);
        if (res) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row) {
                string id = row[0], name = row[2], phone_number = row[3], date_of_entry = row[4];

                // Insert the employee into the removed_employees table
                char insert_query[512];
                sprintf(insert_query, "INSERT INTO removed_employees (id, citizen_card_number, name, phone_number, date_of_entry, date_of_removal) "
                                      "VALUES (%s, '%s', '%s', '%s', '%s', '%s')",
                        id.c_str(), citizen_card_number.c_str(), name.c_str(), phone_number.c_str(), date_of_entry.c_str(), date_of_removal.c_str());

                // Delete the employee from the employees table based on citizen card number
                char delete_query[512];
                sprintf(delete_query, "DELETE FROM employees WHERE citizen_card_number='%s'", citizen_card_number.c_str());

                if (execute_query(obj, insert_query) && execute_query(obj, delete_query)) {
                    cout << "Employee removed successfully!" << endl;
                }
            } else {
                cout << "Employee with citizen card number '" << citizen_card_number << "' not found." << endl;
            }
            mysql_free_result(res);
        }
    }
}

// Function to format and display the employees table
void show_employees(MYSQL* obj) {
    string query = "SELECT * FROM employees";
    if (execute_query(obj, query)) {
        MYSQL_RES* res = mysql_store_result(obj);
        if (res) {
            MYSQL_ROW row;
            cout << "ID | Citizen Card Number | Name | Phone Number | Date of Entry\n";
            cout << "-------------------------------------------------------------\n";
            while ((row = mysql_fetch_row(res))) {
                cout << row[0] << " | " << row[1] << " | " << row[2] << " | " << row[3] << " | " << row[4] << endl;
            }
            mysql_free_result(res);
        }
    }
}

// Function to format and display the removed employees table
void show_removed_employees(MYSQL* obj) {
    string query = "SELECT * FROM removed_employees";
    if (execute_query(obj, query)) {
        MYSQL_RES* res = mysql_store_result(obj);
        if (res) {
            MYSQL_ROW row;
            cout << "ID | Citizen Card Number | Name | Phone Number | Date of Entry | Date of Removal\n";
            cout << "----------------------------------------------------------------------\n";
            while ((row = mysql_fetch_row(res))) {
                cout << row[0] << " | " << row[1] << " | " << row[2] << " | " << row[3] << " | " << row[4] << " | " << row[5] << endl;
            }
            mysql_free_result(res);
        }
    }
}

// Main function to handle menu and user input
int main() {
    MYSQL* obj;

    // Initialize MySQL connection
    if (!(obj = mysql_init(NULL))) {
        cout << "ERROR: MySQL initialization failed!" << endl;
        return 1;
    }

    // Connect to the MySQL database
    if (!mysql_real_connect(obj, HOST, USER, PASS, DB, 3306, NULL, 0)) {
        cout << "ERROR: Failed to connect to database." << endl;
        cout << mysql_error(obj) << endl;
        return 1;
    } else {
        cout << "Connected to the database successfully!" << endl;
    }

    char option;
    bool running = true;

    // Main loop to handle user input
    while (running) {
        cout << "\nChoose an option (1: Add Employee, 2: Remove Employee, 3: Show Employees, 4: Show Removed Employees, 0: Exit): ";
        cin >> option;
        cin.ignore(100, '\n'); // Clear input buffer

        switch (option) {
            case '1':
                add_employee(obj);
                break;
            case '2':
                remove_employee(obj);
                break;
            case '3':
                show_employees(obj);
                break;
            case '4':
                show_removed_employees(obj);
                break;
            case '0':
                running = false;
                break;
            default:
                cout << "Invalid option. Please try again." << endl;
                break;
        }
    }

    // Close MySQL connection
    mysql_close(obj);
    cout << "BYE!!" << endl;

    return 0;
}

