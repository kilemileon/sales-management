#include <iostream>
#include <mysql.h>
#include <mysqld_error.h>
#include <string>
#include <iomanip> // For formatting
#include <ctime>   // For getting current date

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

// Function to get the next unique ID from a table
int get_next_id(MYSQL* obj, const string& table) {
    int last_id = 0;
    string query = "SELECT MAX(id) FROM " + table;
    if (execute_query(obj, query)) {
        MYSQL_RES* res = mysql_store_result(obj);
        if (res) {
            MYSQL_ROW row = mysql_fetch_row(res);
            last_id = row[0] ? atoi(row[0]) : 0;
            mysql_free_result(res);
        }
    }
    return last_id + 1;
}

// Function to add salaries to the database
void add_salaries(MYSQL* obj) {
    int number_of_salaries;
    cout << "How many salaries do you want to add? ";
    cin >> number_of_salaries;
    cin.ignore(100, '\n'); // Clear input buffer

    for (int i = 0; i < number_of_salaries; ++i) {
        string worker_name, phone_number, date_of_payment = get_current_date();
        double salary_amount;

        cout << "Enter the worker's name: ";
        getline(cin, worker_name);

        cout << "Enter the worker's phone number: ";
        getline(cin, phone_number);

        cout << "Enter the salary amount: ";
        cin >> salary_amount;
        cin.ignore(100, '\n'); // Clear input buffer

        int salary_id = get_next_id(obj, "salary_management");

        // Insert salary data into the table
        char query[512];
        sprintf(query, "INSERT INTO salary_management (id, worker_name, amount, phone_number, date_of_payment) VALUES (%d, '%s', %.2f, '%s', '%s')",
                salary_id, worker_name.c_str(), salary_amount, phone_number.c_str(), date_of_payment.c_str());

        if (execute_query(obj, query)) {
            cout << "Salary for worker '" << worker_name << "' added successfully with ID: " << salary_id << endl;
        }
    }
}

// Function to add major expenses to the database
void add_major_expenses(MYSQL* obj) {
    int number_of_expenses;
    cout << "How many major expenses do you want to add? ";
    cin >> number_of_expenses;
    cin.ignore(100, '\n'); // Clear input buffer

    for (int i = 0; i < number_of_expenses; ++i) {
        string expense_name, date_of_expense = get_current_date();
        double expense_amount;

        cout << "Enter the name of the expense: ";
        getline(cin, expense_name);

        cout << "Enter the amount of the expense: ";
        cin >> expense_amount;
        cin.ignore(100, '\n'); // Clear input buffer

        int expense_id = get_next_id(obj, "major_expense");

        // Insert major expense data into the table
        char query[512];
        sprintf(query, "INSERT INTO major_expense (id, name, amount_of_expense, date_of_expense) VALUES (%d, '%s', %.2f, '%s')",
                expense_id, expense_name.c_str(), expense_amount, date_of_expense.c_str());

        if (execute_query(obj, query)) {
            cout << "Major expense '" << expense_name << "' added successfully with ID: " << expense_id << endl;
        }
    }
}

// Function to add minor expenses to the database
void add_minor_expenses(MYSQL* obj) {
    int number_of_expenses;
    cout << "How many minor expenses do you want to add? ";
    cin >> number_of_expenses;
    cin.ignore(100, '\n'); // Clear input buffer

    for (int i = 0; i < number_of_expenses; ++i) {
        string expense_name, date_of_expense = get_current_date();
        double expense_amount;

        cout << "Enter the name of the minor expense: ";
        getline(cin, expense_name);

        cout << "Enter the amount of the minor expense: ";
        cin >> expense_amount;
        cin.ignore(100, '\n'); // Clear input buffer

        int expense_id = get_next_id(obj, "minor_expense");

        // Insert minor expense data into the table
        char query[512];
        sprintf(query, "INSERT INTO minor_expense (id, name, amount_of_expense, date_of_expense) VALUES (%d, '%s', %.2f, '%s')",
                expense_id, expense_name.c_str(), expense_amount, date_of_expense.c_str());

        if (execute_query(obj, query)) {
            cout << "Minor expense '" << expense_name << "' added successfully with ID: " << expense_id << endl;
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
        cout << "\nChoose an option (1: Salary, 2: Major Expense, 3: Minor Expense, 4: Exit): ";
        cin >> option;
        cin.ignore(100, '\n'); // Clear input buffer

        switch (option) {
            case '1':
                add_salaries(obj);
                break;
            case '2':
                add_major_expenses(obj);
                break;
            case '3':
                add_minor_expenses(obj);
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

