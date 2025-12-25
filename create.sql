CREATE TABLE categories (
    category_id SERIAL PRIMARY KEY,
    category_name VARCHAR(100) NOT NULL UNIQUE,
    description TEXT
);

CREATE TABLE brands (
    brand_id SERIAL PRIMARY KEY,
    brand_name VARCHAR(100) NOT NULL UNIQUE,
    country VARCHAR(100),
    website VARCHAR(255)
);

CREATE TABLE products (
    product_id SERIAL PRIMARY KEY,
    product_name VARCHAR(255) NOT NULL,
    category_id INTEGER REFERENCES categories(category_id) ON DELETE CASCADE,
    brand_id INTEGER REFERENCES brands(brand_id) ON DELETE CASCADE,
    price DECIMAL(10,2) CHECK (price >= 0),
    stock_quantity INTEGER CHECK (stock_quantity >= 0),
    expiration_date DATE,
    description TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE suppliers (
    supplier_id SERIAL PRIMARY KEY,
    supplier_name VARCHAR(200) NOT NULL,
    contact_person VARCHAR(150),
    phone VARCHAR(50),
    email VARCHAR(150),
    country VARCHAR(100)
);

CREATE TABLE shipments (
    shipment_id SERIAL PRIMARY KEY,
    product_id INTEGER REFERENCES products(product_id) ON DELETE CASCADE,
    supplier_id INTEGER REFERENCES suppliers(supplier_id) ON DELETE CASCADE,
    quantity INTEGER CHECK (quantity > 0),
    cost DECIMAL(10,2),
    shipment_date DATE DEFAULT CURRENT_DATE
);

CREATE TABLE sales (
    sale_id SERIAL PRIMARY KEY,
    product_id INTEGER REFERENCES products(product_id) ON DELETE CASCADE,
    quantity_sold INTEGER CHECK (quantity_sold > 0),
    total_price DECIMAL(10,2),
    sale_date DATE DEFAULT CURRENT_DATE
);

CREATE TABLE reviews (
    review_id SERIAL PRIMARY KEY,
    product_id INTEGER REFERENCES products(product_id) ON DELETE CASCADE,
    rating INTEGER CHECK (rating >= 1 AND rating <= 5),
    review_text TEXT,
    review_date DATE DEFAULT CURRENT_DATE
);
