#include "gtest/gtest.h"
#include <vector>
#include <string>

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

/**
 * Test DB's basic sql functions with single client.
 * Test list:
 *   1. Invalid sql
 *   2. CREATE TABLE
 *   3. DROP TABLE
 *   4. INSERT INTO table
 */
TEST_F(SingleClient, BasicSql) {
    db_manager = std::make_unique<DBManager>(meta, true);
    ASSERT_TRUE(db_manager->get_status());

    Server s(server_addr, server_port);

    std::thread server_thd = std::thread([&]{
        s.run();
    });

    // Wait for the startup of the server
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    do {
        // test 1
    } while (false);

    do {
        // test 2
        Client client(server_addr, server_port);
        std::vector<std::string> sql = {
            "CREATE TABLE books (", "id INT,", "book_name CHAR(10),",
            "price DECIMAL,", "discount BOOLEAN,", "PRIMARY KEY(id)", ");"
        };

        size_t size = sql.size();
        bool ret = false;
        for (size_t i = 0; i < size; i++) {
            if (i == size - 1) {
                // last msg, need to wait for db's response
                ret = client.send(sql[i], true);
            } else {
                client.send(sql[i]);
            }
        }

        EXPECT_TRUE(ret);
        if (ret == false) break;

        std::string resp = client.get_db_response();
        EXPECT_EQ(resp, std::string("Create Table OK."));
    } while (false);

    do {
        // test 3
    } while (false);

    do {
        // test 4
    } while (false);

    s.shutdown();

    if (server_thd.joinable()) {
        server_thd.join();
    }
}

} // namesapce dawn
