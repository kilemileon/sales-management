#include <iostream>
#include <mysql.h>
#include <string>
#include <iomanip> // for setting precision

using namespace std;

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

// Function to calculate total sales for the month
double calculateTotalSales(MYSQL* obj, const string& start_date, const string& end_date) {
    string query = "SELECT SUM(total_amount) FROM sales_history WHERE sale_date BETWEEN '" + start_date + "' AND '" + end_date + "'";
    
    if (!executeQuery(obj, query.c_str())) return 0;

    MYSQL_RES* result = mysql_store_result(obj);
    MYSQL_ROW row;
    double total_sales = 0;
    
    if ((row = mysql_fetch_row(result)) && row[0] != NULL) {
        total_sales = atof(row[0]); // Get the total sales for the period
    }
    mysql_free_result(result);

    return total_sales;
}

// Function to calculate total expenses for the month
double calculateTotalExpenses(MYSQL* obj, const string& start_date, const string& end_date) {
    // Sum all expenses from major, minor expenses and salary for the given month
    string query = "SELECT SUM(amount) FROM major_expense WHERE expense_date BETWEEN '" + start_date + "' AND '" + end_date + "'";
    
    if (!executeQuery(obj, query.c_str())) return 0;
    
    MYSQL_RES* result = mysql_store_result(obj);
    MYSQL_ROW row;
    double total_expenses = 0;
    
    if ((row = mysql_fetch_row(result)) && row[0] != NULL) {
        total_expenses += atof(row[0]);
    }
    mysql_free_result(result);

    // Query for minor expenses
    query = "SELECT SUM(amount) FROM minor_expense WHERE expense_date BETWEEN '" + start_date + "' AND '" + end_date + "'";
    
    if (!executeQuery(obj, query.c_str())) return 0;

    result = mysql_store_result(obj);
    
    if ((row = mysql_fetch_row(result)) && row[0] != NULL) {
        total_expenses += atof(row[0]);
    }
    mysql_free_result(result);

    // Query for salary expenses
    query = "SELECT SUM(salary) FROM salary WHERE payment_date BETWEEN '" + start_date + "' AND '" + end_date + "'";
    
    if (!executeQuery(obj, query.c_str())) return 0;

    result = mysql_store_result(obj);
    
    if ((row = mysql_fetch_row(result)) && row[0] != NULL) {
        total_expenses += atof(row[0]);
    }
    mysql_free_result(result);

    return total_expenses;
}

// Function to calculate profits and weekly/average earnings
void analyzeProfits(MYSQL* obj, const string& start_date, const string& end_date, int weeks_in_month) {
    double total_sales = calculateTotalSales(obj, start_date, end_date);
    double total_expenses = calculateTotalExpenses(obj, start_date, end_date);
    
    double profit = total_sales - total_expenses;
    double weekly_earnings = profit / weeks_in_month;
    double average_daily_earnings = profit / (weeks_in_month * 7);

    cout << fixed << setprecision(2);
    cout << "Total Sales: " << total_sales << " shillings" << endl;
    cout << "Total Expenses: " << total_expenses << " shillings" << endl;
    cout << "Profit (Sales - Expenses): " << profit << " shillings" << endl;
    cout << "Weekly Earnings: " << weekly_earnings << " shillings" << endl;
    cout << "Average Daily Earnings: " << average_daily_earnings << " shillings" << endl;
}

int main() {
    MYSQL* obj;

    // Initialize MySQL connection object
    if (!(obj = mysql_init(0))) {
        cout << "ERROR: MySQL object could not be created." << endl;
        return 1;
    }

    // Connect to the MySQL database
    if (!mysql_real_connect(obj, HOST, USER, PASS, DB, 3306, NULL, 0)) {
        cout << "ERROR: Failed to connect to database." << endl;
        cout << mysql_error(obj) << endl;
        return 1;
    }

    cout << "Connected to the database." << endl << endl;

    string start_date, end_date;
    int weeks_in_month;

    // Get user input for date range and weeks in the month
    cout << "Enter the start date for the month (YYYY-MM-DD): ";
    cin >> start_date;

    cout << "Enter the end date for the month (YYYY-MM-DD): ";
    cin >> end_date;

    cout << "Enter the number of weeks in the month: ";
    cin >> weeks_in_month;

    // Analyze profits
    analyzeProfits(obj, start_date, end_date, weeks_in_month);

    // Close MySQL connection
    mysql_close(obj);

    return 0;
}

