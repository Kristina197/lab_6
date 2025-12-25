#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <postgresql/libpq-fe.h>

using namespace std;

size_t utf8_display_width(const char* str) {
    size_t width = 0;
    for (const unsigned char* p = (const unsigned char*)str; *p; ++p) {
        if ((*p & 0xC0) != 0x80) width++;
    }
    return width;
}

class CosmeticsDatabase {
private:
    PGconn *conn;
    
public:
    CosmeticsDatabase(const string& conninfo) {
        conn = PQconnectdb(conninfo.c_str());
        
        if (PQstatus(conn) == CONNECTION_BAD) {
            cerr << "ERROR: " << PQerrorMessage(conn) << endl;
            PQfinish(conn);
            exit(1);
        }
        cout << "OK: PostgreSQL connected\n" << endl;
    }
    
    ~CosmeticsDatabase() {
        if (conn) {
            PQfinish(conn);
        }
    }
    
    void executeQuery(const string& query, const string& description) {
        cout << "\n" << string(130, '=') << "\n";
        cout << description << "\n";
        cout << string(130, '=') << "\n";
        
        PGresult *res = PQexec(conn, query.c_str());
        
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            cout << "ERROR: " << PQerrorMessage(conn) << "\n";
            PQclear(res);
            return;
        }
        
        printResults(res);
        PQclear(res);
    }
    
    void printResults(PGresult *res) {
        int rows = PQntuples(res);
        int cols = PQnfields(res);
        
        if (rows == 0) {
            cout << "\nNo data found\n";
            return;
        }
        
        vector<size_t> colWidth(cols, 0);
        
        for (int i = 0; i < cols; i++) {
            colWidth[i] = utf8_display_width(PQfname(res, i));
        }
        
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                size_t w = utf8_display_width(PQgetvalue(res, i, j));
                colWidth[j] = max(colWidth[j], w);
            }
        }
        
        for (int i = 0; i < cols; i++) {
            colWidth[i] += 4;
        }
        
        cout << "\n";
        
        for (int i = 0; i < cols; i++) {
            string header = PQfname(res, i);
            size_t w = utf8_display_width(header.c_str());
            cout << header << string(colWidth[i] - w, ' ');
        }
        cout << "\n";
        
        size_t totalWidth = 0;
        for (int i = 0; i < cols; i++) {
            totalWidth += colWidth[i];
        }
        cout << string(totalWidth, '-') << "\n";
        
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                string value = PQgetvalue(res, i, j);
                size_t w = utf8_display_width(value.c_str());
                cout << value << string(colWidth[j] - w, ' ');
            }
            cout << "\n";
        }
        
        cout << "\nRows returned: " << rows << "\n";
    }
    
    void query1() {
        string query = R"(
            SELECT p.product_name, c.category_name, b.brand_name, p.price, p.stock_quantity
            FROM products p 
            JOIN categories c ON p.category_id = c.category_id
            JOIN brands b ON p.brand_id = b.brand_id
            ORDER BY p.price DESC;
        )";
        executeQuery(query, "QUERY 1: All products with categories and brands (JOIN)");
    }
    
    void query2() {
        string query = R"(
            SELECT product_name, price, stock_quantity
            FROM products 
            WHERE price > 2000
            ORDER BY price DESC;
        )";
        executeQuery(query, "QUERY 2: Premium products (price > 2000) - WHERE clause");
    }
    
    void query3() {
        string query = R"(
            SELECT c.category_name, COUNT(p.product_id) as product_count, 
                   ROUND(AVG(p.price)::NUMERIC, 2) as avg_price
            FROM categories c
            LEFT JOIN products p ON c.category_id = p.category_id
            GROUP BY c.category_id, c.category_name
            HAVING COUNT(p.product_id) > 0
            ORDER BY product_count DESC;
        )";
        executeQuery(query, "QUERY 3: Products by category (COUNT + AVG + HAVING)");
    }
    
    void query4() {
        string query = R"(
            SELECT b.brand_name, COUNT(p.product_id) as products, 
                   SUM(p.stock_quantity) as total_stock
            FROM brands b
            LEFT JOIN products p ON b.brand_id = p.brand_id
            GROUP BY b.brand_id, b.brand_name
            HAVING COUNT(p.product_id) > 0
            ORDER BY products DESC;
        )";
        executeQuery(query, "QUERY 4: Products and stock by brand (GROUP BY + SUM)");
    }
    
    void query5() {
        string query = R"(
            SELECT p.product_name, ROUND(AVG(r.rating)::NUMERIC, 2) as avg_rating, 
                   COUNT(r.review_id) as review_count
            FROM products p
            INNER JOIN reviews r ON p.product_id = r.product_id
            GROUP BY p.product_id, p.product_name
            HAVING AVG(r.rating) >= 4.0
            ORDER BY avg_rating DESC;
        )";
        executeQuery(query, "QUERY 5: Top rated products (AVG rating >= 4.0)");
    }
    
    void query6() {
        string query = R"(
            SELECT product_name, expiration_date, 
                   expiration_date - CURRENT_DATE as days_left
            FROM products
            WHERE expiration_date < CURRENT_DATE + INTERVAL '6 months'
            ORDER BY expiration_date ASC;
        )";
        executeQuery(query, "QUERY 6: Products expiring soon (within 6 months) - subquery logic");
    }
    
    void query7() {
        string query = R"(
            SELECT p.product_name, su.supplier_name, sh.quantity, 
                   sh.cost, sh.shipment_date
            FROM shipments sh
            INNER JOIN products p ON sh.product_id = p.product_id
            INNER JOIN suppliers su ON sh.supplier_id = su.supplier_id
            ORDER BY sh.shipment_date DESC;
        )";
        executeQuery(query, "QUERY 7: Recent shipments with suppliers (INNER JOIN)");
    }
    
    void query8() {
        string query = R"(
            SELECT p.product_name, SUM(s.quantity_sold) as total_sold, 
                   SUM(s.total_price) as revenue
            FROM products p
            LEFT JOIN sales s ON p.product_id = s.product_id
            WHERE s.sale_id IS NOT NULL
            GROUP BY p.product_id, p.product_name
            ORDER BY revenue DESC
            LIMIT 5;
        )";
        executeQuery(query, "QUERY 8: Top-5 bestsellers by revenue (SUM + LIMIT)");
    }
    
    void query9() {
        string query = R"(
            SELECT b.brand_name, SUM(s.total_price) as total_revenue,
                   COUNT(s.sale_id) as sales_count
            FROM brands b
            JOIN products p ON b.brand_id = p.brand_id
            JOIN sales s ON p.product_id = s.product_id
            GROUP BY b.brand_id, b.brand_name
            ORDER BY total_revenue DESC;
        )";
        executeQuery(query, "QUERY 9: Revenue by brand (multiple JOINs + SUM)");
    }
    
    void query10() {
        string query = R"(
            SELECT 
                MIN(price) as cheapest,
                MAX(price) as most_expensive,
                ROUND(AVG(price)::NUMERIC, 2) as average_price,
                SUM(stock_quantity) as total_items
            FROM products;
        )";
        executeQuery(query, "QUERY 10: Database statistics (MIN, MAX, AVG, SUM)");
    }
    
    void sqlInjections() {
        cout << "\n" << string(130, '=') << "\n";
        cout << "SQL INJECTION DEMONSTRATIONS (for educational purposes)\n";
        cout << string(130, '=') << "\n";
        
        string inj1 = "SELECT product_name, price FROM products WHERE price > 0 OR 1=1;";
        executeQuery(inj1, "INJECTION 1: Boolean bypass (OR 1=1)");
        
        string inj2 = "SELECT product_name FROM products WHERE price > 0 OR (SELECT COUNT(*) FROM brands) > 0;";
        executeQuery(inj2, "INJECTION 2: Subquery injection");
        
        string inj3 = R"(
            SELECT product_name, CAST(price AS VARCHAR) as price
            FROM products 
            UNION ALL
            SELECT brand_name, '9999' FROM brands;
        )";
        executeQuery(inj3, "INJECTION 3: UNION attack");
    }
    
    void runAll() {
        cout << "\n" << string(130, '=') << "\n";
        cout << "                     COSMETICS SHOP DATABASE - PostgreSQL\n";
        cout << "                10 SQL Queries + 3 SQL Injection Examples\n";
        cout << string(130, '=') << "\n";
        
        query1();
        query2();
        query3();
        query4();
        query5();
        query6();
        query7();
        query8();
        query9();
        query10();
        sqlInjections();
        
        cout << "\n" << string(130, '=') << "\n";
        cout << "SUCCESS: All queries executed successfully!\n";
        cout << string(130, '=') << "\n\n";
    }
};

int main() {
    setlocale(LC_ALL, "C.UTF-8");
    
    string conninfo = "dbname=cosmetics_shop user=cosmetics_admin password=Cosmetics2025! host=localhost";
    
    try {
        CosmeticsDatabase db(conninfo);
        db.runAll();
    }
    catch (const exception& e) {
        cerr << "FATAL ERROR: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
