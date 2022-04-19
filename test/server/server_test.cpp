#include "gtest/gtest.h"
#include <vector>
#include <string>
#include <utility>

#include "manager/db_manager.h"
#include "server/server.h"
#include "test_util.h"

namespace dawn {

extern std::unique_ptr<DBManager> db_manager;

class SingleClient : public testing::Test {
public:
    void SetUp() {}

    void TearDown() {
        remove(meta_name);
        remove(db_name);
        remove(log_name);
    }

    const char *meta = "test";
    const char *meta_name = "test.mtd";
    const char *db_name = "test.db";
    const char *log_name = "test.log";

    const char *server_addr = "127.0.0.1";
    const int server_port = 9999;
};

bool send_sql(Client& client, std::vector<std::string>& sql) {
    size_t size = sql.size();
    bool success = false;
    for (size_t i = 0; i < size; i++) {
        if (i == size - 1) {
            // last msg, need to wait for db's response
            success = client.send(sql[i], true);
        } else {
            client.send(sql[i]);
        }
    }
    return success;
}

/**
 * Test DB's basic sql functions with single client.
 * Test list:
 *   1. CREATE TABLE
 *   2. DROP TABLE
 *   3. INSERT INTO table
 */
TEST_F(SingleClient, BasicSql) {
    db_manager = std::make_unique<DBManager>(meta, true);
    ASSERT_TRUE(db_manager->get_status());

    Server s(server_addr, server_port);

    std::thread server_thd = std::thread([&]{
        s.run();
    });

    std::vector<std::vector<std::string>> sql {
        // test 1
        std::vector<std::string> { "CREATE TABLE books (", "id INT,",
        "book_name CHAR(10),", "price DECIMAL,", "discount BOOLEAN,",
        "PRIMARY KEY(id)", ");" },
        // test 2
        std::vector<std::string> {
            "INSERT INTO books ", "VALUES (",
            "1,", "'1234',", "23.33,","true", ");"
        },
        // test 3
        std::vector<std::string> {
            "DROP TABLE books;"
        },
    };

    std::vector<std::string> expect_response {
        // test 1
        "Create Table OK.",
        // test 2
        "Insert OK.",
        // test 3
        "Drop Table OK.",
    };

    size_t test_num = sql.size();

    // Wait for the startup of the server
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    for (size_t idx = 0; idx < test_num; idx++) {
        PRINT("Run test", idx + 1, "...");
        Client client(server_addr, server_port);
        bool success = send_sql(client, sql[idx]);

        EXPECT_TRUE(success);
        if (success == false) continue;

        std::string resp = client.get_db_response();
        EXPECT_EQ(resp, expect_response[idx]);
    }

    s.shutdown();

    if (server_thd.joinable()) {
        server_thd.join();
    }
}

} // namesapce dawn
