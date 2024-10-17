#include <iostream>
#include <mysql.h>
#include <mysqld_error.h>
#include <iomanip>  // For formatting the output table
#include <sstream>  // For converting numbers to strings
#include <string>

using namespace std;

// Function to convert numbers to strings
template <typename T>
string toString(T value) {
    stringstream ss;
    ss << value;
    return ss.str();
}

// Database credentials
char HOST[] = "LOCALHOST";
char USER[] = "root";
char PASS[] = "pas1234!";
char DB[] = "beverages_db";

// Function to execute a query and display errors if any
bool executeQuery(MYSQL* obj, const char* query) {
    if (mysql_query(obj, query)) {
        cout << "ERROR: " << mysql_error(obj) << endl;
        return false;
    }
    return true;
}

// Function to display data in a table format
void displayTableRow(const string& label, const string& amount) {
    cout << setw(20) << left << label 
         << setw(15) << right << amount << endl;
    cout << "---------------------------------------" << endl;
}

// Function to display M-Pesa transactions and total
void displayMpesaTransactions(MYSQL* obj) {
    char sale_date[11];
    char consult[1000];

    cout << "Enter the date to display M-Pesa transactions (YYYY-MM-DD): ";
    cin.getline(sale_date, 11, '\n');

    sprintf(consult, "SELECT amount FROM mpesa_transactions WHERE transaction_date = '%s'", sale_date);
    if (!executeQuery(obj, consult)) return;

    MYSQL_RES* result = mysql_store_result(obj);
    MYSQL_ROW row;
    double total_mpesa = 0;

    // Display table header
    cout << "\nM-Pesa Transactions for " << sale_date << ":" << endl;
    cout << "---------------------------------------" << endl;
    displayTableRow("Transaction", "Amount (shillings)");

    // Fetch and display transactions
    while ((row = mysql_fetch_row(result))) {
        double amount = atof(row[0]);
        total_mpesa += amount;
        displayTableRow("M-Pesa", toString(amount));
    }

    // Display total amount for M-Pesa transactions
    displayTableRow("Total M-Pesa", toString(total_mpesa));
    cout << endl;

    mysql_free_result(result);
}

// Function to display Cash transactions and total
void displayCashTransactions(MYSQL* obj) {
    char sale_date[11];
    char consult[1000];

    cout << "Enter the date to display Cash transactions (YYYY-MM-DD): ";
    cin.getline(sale_date, 11, '\n');

    sprintf(consult, "SELECT amount FROM cash_transactions WHERE transaction_date = '%s'", sale_date);
    if (!executeQuery(obj, consult)) return;

    MYSQL_RES* result = mysql_store_result(obj);
    MYSQL_ROW row;
    double total_cash = 0;

    // Display table header
    cout << "\nCash Transactions for " << sale_date << ":" << endl;
    cout << "---------------------------------------" << endl;
    displayTableRow("Transaction", "Amount (shillings)");

    // Fetch and display transactions
    while ((row = mysql_fetch_row(result))) {
        double amount = atof(row[0]);
        total_cash += amount;
        displayTableRow("Cash", toString(amount));
    }

    // Display total amount for Cash transactions
    displayTableRow("Total Cash", toString(total_cash));
    cout << endl;

    mysql_free_result(result);
}

// Function to record a sale, update stock and sales history
void recordSale(MYSQL* obj) {
    char beverage[30], sale_date[11];
    int sold, remaining;
    char consult[1000];

    // Input beverage name and quantity sold
    cout << "Enter the beverage sold: ";
    cin.getline(beverage, 30, '\n');

    cout << "Enter the quantity sold: ";
    cin >> sold;
    cin.ignore(100, '\n'); // Clear input buffer

    cout << "Enter the sale date (YYYY-MM-DD): ";
    cin.getline(sale_date, 11, '\n'); // Get the sale date from the user

    // Query the database for remaining stock of the beverage
    sprintf(consult, "SELECT stock FROM inventory WHERE name = '%s'", beverage);

    // Execute the SELECT query
    if (mysql_query(obj, consult)) {
        cout << "ERROR: Failed to retrieve stock." << endl;
        cout << mysql_error(obj) << endl;
        return;
    }

    MYSQL_RES* result = mysql_store_result(obj);
    MYSQL_ROW row;
    if ((row = mysql_fetch_row(result))) {
        remaining = atoi(row[0]); // Fetch remaining stock as integer
        if (sold > remaining) {
            cout << "ERROR: Not enough stock available. Only " << remaining << " left." << endl;
        } else {
            remaining -= sold; // Update remaining stock
            cout << "Remaining stock after sale: " << remaining << endl;

            // Update the stock in the database
            sprintf(consult, "UPDATE inventory SET stock = %d WHERE name = '%s'", remaining, beverage);
            if (!executeQuery(obj, consult)) return;

            // Calculate the total sales
            int price_per_soda = 200; // Assume price per soda is 200
            int total_sales = sold * price_per_soda;
            cout << "Total sales for " << sold << " " << beverage << "(s): " << total_sales << " shillings." << endl;

            // Insert sale data into the sales_history table
            sprintf(consult, "INSERT INTO sales_history (beverage_name, quantity_sold, total_amount, sale_date) "
                             "VALUES ('%s', %d, %d, '%s')", beverage, sold, total_sales, sale_date);
            if (!executeQuery(obj, consult)) return;

            cout << "Sale recorded successfully in sales_history." << endl;
        }
    } else {
        cout << "ERROR: Beverage not found in the inventory." << endl;
    }
    mysql_free_result(result);
}

// Function to record M-Pesa transactions
void recordMpesaTransaction(MYSQL* obj) {
    char transaction_date[11];
    double amount;
    char consult[1000];

    // Input M-Pesa transaction details
    cout << "Enter the date of the transaction (YYYY-MM-DD): ";
    cin.getline(transaction_date, 11, '\n');

    cout << "Enter the amount of the M-Pesa transaction: ";
    cin >> amount;
    cin.ignore(100, '\n'); // Clear input buffer

    // Insert M-Pesa transaction into mpesa_transactions table
    sprintf(consult, "INSERT INTO mpesa_transactions (transaction_date, amount) "
                     "VALUES ('%s', %.2f)", transaction_date, amount);
    if (executeQuery(obj, consult)) {
        cout << "M-Pesa transaction recorded successfully." << endl;
    }
}

// Function to record Cash transactions
void recordCashTransaction(MYSQL* obj) {
    char transaction_date[11];
    double amount;
    char consult[1000];

    // Input Cash transaction details
    cout << "Enter the date of the transaction (YYYY-MM-DD): ";
    cin.getline(transaction_date, 11, '\n');

    cout << "Enter the amount of the Cash transaction: ";
    cin >> amount;
    cin.ignore(100, '\n'); // Clear input buffer

    // Insert Cash transaction into cash_transactions table
    sprintf(consult, "INSERT INTO cash_transactions (transaction_date, amount) "
                     "VALUES ('%s', %.2f)", transaction_date, amount);
    if (executeQuery(obj, consult)) {
        cout << "Cash transaction recorded successfully." << endl;
    }
}

// Function to display all sales and the total for a specific day
void checkTotalSales(MYSQL* obj) {
    char sale_date[11];
    char consult[1000];

    cout << "Enter the date to check total sales (YYYY-MM-DD): ";
    cin.getline(sale_date, 11, '\n');

    // Query to fetch all sales for the given date
    sprintf(consult, "SELECT beverage_name, quantity_sold, total_amount FROM sales_history WHERE sale_date = '%s'", sale_date);
    if (!executeQuery(obj, consult)) return;

    MYSQL_RES* result = mysql_store_result(obj);
    MYSQL_ROW row;
    double total_sales = 0;

    // Display table header for all sales
    cout << "\nSales for " << sale_date << ":" << endl;
    cout << "--------------------------------------------------------" << endl;
    cout << setw(20) << left << "Beverage"
         << setw(15) << right << "Quantity Sold"
         << setw(15) << right << "Total (shillings)" << endl;
    cout << "--------------------------------------------------------" << endl;

    // Fetch and display each sale
    while ((row = mysql_fetch_row(result))) {
        string beverage_name = row[0];
        int quantity_sold = atoi(row[1]);
        double total_amount = atof(row[2]);

        // Accumulate the total sales for the day
        total_sales += total_amount;

        // Display each sale
        cout << setw(20) << left << beverage_name
             << setw(15) << right << toString(quantity_sold)
             << setw(15) << right << toString(total_amount) << endl;
    }

    // Display the total sales for the day
    cout << "--------------------------------------------------------" << endl;
    cout << setw(20) << left << "Total Sales"
         << setw(15) << right << "" // Empty space for alignment
         << setw(15) << right << toString(total_sales) << " shillings" << endl;

    cout << endl;

    mysql_free_result(result);
}

int main() {
    MYSQL* obj; // MySQL object for connection
    bool ProgramIsOpened = true;
    int answer;

    // Initialize MySQL connection
    obj = mysql_init(0);
    obj = mysql_real_connect(obj, HOST, USER, PASS, DB, 3306, NULL, 0);
    if (!obj) {
        cout << "Failed to connect to MySQL database." << endl;
        return -1;
    }
    cout << "Connected to MySQL database successfully." << endl;

    // Main menu loop
    while (ProgramIsOpened) {
        cout << "\nChoose an option:" << endl;
        cout << "1. Record sale" << endl;
        cout << "2. Check total sales (with details)" << endl;  // Updated option 2
        cout << "3. Record M-Pesa and Cash transactions" << endl;
        cout << "4. Display M-Pesa transactions" << endl;
        cout << "5. Display Cash transactions" << endl;
        cout << "0. Exit" << endl;
        cout << "Your choice: ";
        cin >> answer;
        cin.ignore(100, '\n'); // Clear input buffer

        switch (answer) {
            case 1:
                recordSale(obj);
                break;
            case 2:
                checkTotalSales(obj);  // Combined function for detailed sales and total
                break;
            case 3:
                // Record both M-Pesa and Cash transactions
                recordMpesaTransaction(obj);
                recordCashTransaction(obj);
                break;
            case 4:
                displayMpesaTransactions(obj);
                break;
            case 5:
                displayCashTransactions(obj);
                break;
            case 0:
                ProgramIsOpened = false;
                cout << "Exiting program..." << endl;
                break;
            default:
                cout << "Invalid option. Try again." << endl;
                break;
        }
    }

    mysql_close(obj); // Close the MySQL connection
    return 0;
}

