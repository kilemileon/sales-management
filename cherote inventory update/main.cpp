#include <iostream>
#include <mysql.h>
#include <mysqld_error.h>
#include <string>
#include <algorithm> // For trimming whitespace
#include <ctime>     // For date and time
#include <iomanip>   // For formatting output

using namespace std;

// Database credentials
const char HOST[] = "localhost"; // Ensure this is in lowercase
const char USER[] = "root";
const char PASS[] = "pas1234!";
const char DB[] = "beverages_db"; // Assuming the database is named "beverages_db"

// Helper function to trim whitespace
string trim(string str) {
    str.erase(0, str.find_first_not_of(' ')); // Trim leading spaces
    str.erase(str.find_last_not_of(' ') + 1); // Trim trailing spaces
    return str;
}

// Function to get the current date in YYYY-MM-DD format
string getCurrentDate() {
    time_t now = time(0);
    tm* localtm = localtime(&now);
    char buffer[11]; // YYYY-MM-DD format
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", localtm);
    return string(buffer);
}

// Function to validate date format
bool isValidDate(const string& date) {
    if (date.length() != 10 || date[4] != '-' || date[7] != '-') {
        return false;
    }

    for (int i = 0; i < date.length(); ++i) {
        if (i == 4 || i == 7) continue; // Skip '-' characters
        if (!isdigit(date[i])) return false;
    }

    return true;
}

// Function to execute a query and handle errors
bool executeQuery(MYSQL* obj, const string& query) {
    if (mysql_query(obj, query.c_str())) {
        cout << "ERROR: " << mysql_error(obj) << endl;
        return false;
    }
    return true;
}

// Function to update beverage inventory
void updateBeverageInventory(MYSQL* obj) {
    char beverage[30];
    int quantity_added;
    char update_date[11]; // Store the update date (format: YYYY-MM-DD)

    cout << "Enter the beverage name to update: ";
    cin.getline(beverage, 30, '\n');

    cout << "Enter the quantity to add: ";
    cin >> quantity_added;
    cin.ignore(100, '\n'); // Clear input buffer

    // Ask if the user wants to enter the date manually or use the current date
    cout << "Do you want to enter the date manually? (1 for Yes, 0 for No): ";
    int manual_date;
    cin >> manual_date;
    cin.ignore(100, '\n'); // Clear input buffer

    if (manual_date == 1) {
        cout << "Enter the update date (YYYY-MM-DD): ";
        cin.getline(update_date, 11, '\n');
        if (!isValidDate(trim(update_date))) {
            cout << "ERROR: Invalid date format. Please use YYYY-MM-DD." << endl;
            return; // Exit this function if the date is invalid
        }
    } else {
        // Use the current date
        string currentDate = getCurrentDate();
        strcpy(update_date, currentDate.c_str());
        cout << "Using current date: " << update_date << endl;
    }

    // Query to update the inventory table
    char query[512];
    sprintf(query, "UPDATE inventory SET stock = stock + %d WHERE name = '%s'", quantity_added, trim(beverage).c_str());

    // Execute the UPDATE query for inventory
    if (executeQuery(obj, query)) {
        cout << "Inventory updated successfully." << endl;

        // Insert update data into the inventory_updates table
        sprintf(query, "INSERT INTO inventory_updates (beverage_name, quantity_added, update_date) "
                       "VALUES ('%s', %d, '%s')", trim(beverage).c_str(), quantity_added, update_date);

        // Execute the INSERT query
        executeQuery(obj, query);
    }
}

// Function to update beverage price
void updateBeveragePrice(MYSQL* obj) {
    char beverage[30];
    double new_price;
    char update_date[11]; // Store the update date (format: YYYY-MM-DD)

    cout << "Enter the beverage name to update the price: ";
    cin.getline(beverage, 30, '\n');

    cout << "Enter the new price for " << trim(beverage) << ": ";
    cin >> new_price;
    cin.ignore(100, '\n'); // Clear input buffer

    cout << "Enter the last updated date (YYYY-MM-DD): ";
    cin.getline(update_date, 11, '\n');
    
    // Query to update or insert the price in the beverage_prices table
    char query[512];
    sprintf(query, "INSERT INTO beverage_prices (beverage_name, price, last_updated) "
                   "VALUES ('%s', %.2f, '%s') "
                   "ON DUPLICATE KEY UPDATE price = %.2f, last_updated = '%s'", 
                   trim(beverage).c_str(), new_price, update_date, new_price, update_date);

    // Execute the price update query
    executeQuery(obj, query);
}

// Function to show beverage prices
void showBeveragePrices(MYSQL* obj) {
    string query = "SELECT beverage_name, price, last_updated FROM beverage_prices";
    if (executeQuery(obj, query)) {
        MYSQL_RES* res = mysql_store_result(obj);
        if (res) {
            MYSQL_ROW row;
            cout << "Beverage Name | Price | Last Updated\n";
            cout << "---------------------------------------\n";
            while ((row = mysql_fetch_row(res))) {
                cout << row[0] << " | " << row[1] << " | " << row[2] << endl;
            }
            mysql_free_result(res);
        }
    }
}

// Function to add a new beverage type
void addBeverage(MYSQL* obj) {
    char beverage_name[30];
    double price;
    int quantity;

    cout << "Enter Beverage Name: ";
    cin.getline(beverage_name, 30, '\n');

    cout << "Enter Price for " << trim(beverage_name) << ": ";
    cin >> price;
    cin.ignore(100, '\n'); // Clear input buffer

    cout << "Enter the quantity to add into inventory: ";
    cin >> quantity;
    cin.ignore(100, '\n'); // Clear input buffer

    // Insert the new beverage into the inventory
    char query[512];
    sprintf(query, "INSERT INTO inventory (name, stock) VALUES ('%s', %d)", 
            trim(beverage_name).c_str(), quantity);

    if (executeQuery(obj, query)) {
        cout << "Beverage added successfully to inventory!" << endl;

        // Insert the price for the new beverage into the beverage_prices table
        sprintf(query, "INSERT INTO beverage_prices (beverage_name, price, last_updated) "
                       "VALUES ('%s', %.2f, '%s')", 
                       trim(beverage_name).c_str(), price, getCurrentDate().c_str());

        executeQuery(obj, query);
    } else {
        cout << "ERROR: Unable to add beverage." << endl;
    }
}

// Function to remove a beverage from inventory
void removeBeverage(MYSQL* obj) {
    char beverage_name[30];

    cout << "Enter the Beverage Name to remove: ";
    cin.getline(beverage_name, 30, '\n');

    // Query to delete the beverage from the inventory table
    char query[512];
    sprintf(query, "DELETE FROM inventory WHERE name = '%s'", trim(beverage_name).c_str());
    
    if (executeQuery(obj, query)) {
        cout << "Beverage removed successfully from inventory!" << endl;

        // Query to delete the beverage from the beverage_prices table
        sprintf(query, "DELETE FROM beverage_prices WHERE beverage_name = '%s'", trim(beverage_name).c_str());
        executeQuery(obj, query);
    } else {
        cout << "ERROR: Unable to remove beverage." << endl;
    }
}

// Function to show inventory
void showInventory(MYSQL* obj) {
    string query = "SELECT * FROM inventory";
    if (executeQuery(obj, query)) {
        MYSQL_RES* res = mysql_store_result(obj);
        if (res) {
            MYSQL_ROW row;
            cout << "ID | Beverage Name | Stock\n";
            cout << "---------------------------------------\n";
            while ((row = mysql_fetch_row(res))) {
                cout << row[0] << " | " << row[1] << " | " << row[2] << endl;
            }
            mysql_free_result(res);
        }
    }
}

// Main function to handle menu and user input
int main() {
    MYSQL* obj;

    // Initialize MySQL connection
    obj = mysql_init(0);
    if (obj) {
        cout << "Connecting to database..." << endl;
    } else {
        cout << "ERROR: MySQL initialization failed." << endl;
        return 1;
    }

    // Connect to the database
    if (!mysql_real_connect(obj, HOST, USER, PASS, DB, 0, NULL, 0)) {
        cout << "ERROR: MySQL connection failed." << endl;
        return 1;
    }

    cout << "Connected to database successfully!" << endl;

    bool running = true;
    while (running) {
        cout << "\nMenu Options:" << endl;
        cout << "1: Show Beverage Prices" << endl;
        cout << "2: Add a New Beverage" << endl;
        cout << "3: Show Inventory" << endl;
        cout << "4: Update Beverage Inventory" << endl;
        cout << "5: Update Beverage Price" << endl;
        cout << "6: Remove a Beverage" << endl;
        cout << "0: Exit" << endl;
        cout << "Enter your choice: ";

        char choice;
        cin >> choice;
        cin.ignore(100, '\n'); // Clear input buffer

        switch (choice) {
            case '1':
                showBeveragePrices(obj);
                break;
            case '2':
                addBeverage(obj);
                break;
            case '3':
                showInventory(obj);
                break;
            case '4':
                updateBeverageInventory(obj);
                break;
            case '5':
                updateBeveragePrice(obj);
                break;
            case '6':
                      // Handle the remove beverage option
                removeBeverage(obj);
                break;
            case '0':
                running = false;
                cout << "Exiting the program." << endl;
                break;
            default:
                cout << "ERROR: Invalid option. Please try again." << endl;
        }
    }

    // Close the MySQL connection
    mysql_close(obj);
    return 0;
}

