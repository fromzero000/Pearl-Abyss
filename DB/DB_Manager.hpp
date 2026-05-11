#ifndef DB_MANAGER_HPP
#define DB_MANAGER_HPP


#include <string>
#include <vector>
#include <pqxx/pqxx>

struct OrderItem {
    int part_id;
    int quantity;
    int unit_price;
};

class DB_Manager {
private:
    pqxx::connection* conn = nullptr;
    std::string db_config_file;

    std::string getConnectionString();

public:
    DB_Manager(std::string config_file);
    ~DB_Manager();

    bool connect();
    void disconnect();

    bool processOrderTransaction(int projectId, int supplierId, int warehouseId, int distance, const std::vector<OrderItem>& items);

    void printProjectList();
    int selectSupplierWithPaging();
    void printWarehouseList(int projectId = -1);
    void printPartList();
    void showDashboard();
    void showSupplierReport();
    void printSupplierOrderHistory(int supplierId);
};

#endif //DB_MANAGER_HPP
