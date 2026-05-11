
#include "DB_Manager.hpp"
#include <iostream>
#include <fstream>
#include <limits>
using namespace std;

DB_Manager::DB_Manager(string config_file) : db_config_file(config_file) {}

DB_Manager::~DB_Manager() {
    disconnect();
}

std::string DB_Manager::getConnectionString() {
    ifstream file(db_config_file);
    if (!file.is_open()) return "";

    string line, conn_str = "";
    while (getline(file, line)) {
        if (!line.empty()) conn_str += line + " ";
    }
    return conn_str;
}

bool DB_Manager::connect() {
    try {
        string conn_str = getConnectionString();
        if (conn_str.empty()) {
            cerr << "[Error] 설정 파일(" << db_config_file << ")을 찾을 수 없습니다." << endl;
            return false;
        }
        conn = new pqxx::connection(conn_str);
        if (conn->is_open()) {
            cout << "[System] DB 접속 성공: " << conn->dbname() << endl;
            return true;
        }
        return false;
    } catch (const std::exception &e) {
        cerr << "[Error] DB 접속 실패: " << e.what() << endl;
        return false;
    }
}

void DB_Manager::disconnect() {
    if (conn) {
        delete conn;
        conn = nullptr;
        cout << "[System] DB 연결이 해제되었습니다." << endl;
    }
}

bool DB_Manager::processOrderTransaction(int projectId, int supplierId, int warehouseId, int distance, const vector<OrderItem>& items) {
    if (!conn || !conn->is_open()) {
        cout << "[Error] DB가 연결되지 않았습니다." << endl;
        return false;
    }

    try {
        pqxx::work W(*conn);
        W.exec("SET TRANSACTION ISOLATION LEVEL READ COMMITTED;");
        string sql_po =
                "INSERT INTO PurchaseOrder (ProjectID, SupplierID, PurchaseState, Engineer) "
                "VALUES (" + to_string(projectId) + ", " + to_string(supplierId) + ", '발주완료', '강무진') "
                                                                                   "RETURNING PurchaseID;";

        pqxx::result r_po = W.exec(sql_po);
        int new_po_id = r_po[0][0].as<int>();
        cout << ">> 1. 발주서 생성 완료 (ID: " << new_po_id << ")" << endl;

        string sql_del =
                "INSERT INTO Delivery (PurchaseID, ArrivalDate, State, TransportType, Distance) "
                "VALUES (" + to_string(new_po_id) + ", CURRENT_DATE, '정상', '트럭', " + to_string(distance) + ") "
                                                                                                           "RETURNING DeliveryID;";

        pqxx::result r_del = W.exec(sql_del);
        int new_del_id = r_del[0][0].as<int>();
        cout << ">> 2. 초기 납품 기록 생성 완료 (ID: " << new_del_id << ")" << endl;

        int line_num = 1;
        double total_weight = 0.0;
        for (const auto& item : items) {
            string sql_line =
                    "INSERT INTO PurchaseOrderLine (PurchaseID, LineNum, PartID, Quantity, UnitPrice) "
                    "VALUES (" + to_string(new_po_id) + ", " + to_string(line_num) + ", " +
                    to_string(item.part_id) + ", " + to_string(item.quantity) + ", " + to_string(item.unit_price) + ");";
            W.exec(sql_line);


            int delivered_qty = item.quantity / 2;
            if (delivered_qty == 0) delivered_qty = 1;

            string sql_d_item =
                    "INSERT INTO DeliveryDetail (DeliveryID, PurchaseID, LineNum, Quantity, InspectionResult) "
                    "VALUES (" + to_string(new_del_id) + ", " + to_string(new_po_id) + ", " +
                    to_string(line_num) + ", " + to_string(delivered_qty) + ", '정상 입고');";
            W.exec(sql_d_item);

            string sql_inv =
                    "INSERT INTO Inventory (WarehouseID, PartID, Quantity) "
                    "VALUES (" + to_string(warehouseId) + ", " + to_string(item.part_id) + ", " + to_string(delivered_qty) + ") "
                                                                                                                             "ON CONFLICT (WarehouseID, PartID) "
                                                                                                                             "DO UPDATE SET Quantity = Inventory.Quantity + " + to_string(delivered_qty) + ";";
            W.exec(sql_inv);
            total_weight += item.quantity;
            line_num++;
        }

        double transport_emission = distance * total_weight * 0.1;

        string sql_carbon_trans =
                "INSERT INTO CarbonRecord (DeliveryID, EmissionAmount, Type, Criteria) "
                "VALUES (" + to_string(new_del_id) + ", " + to_string(transport_emission) +
                ", '운송', '거리(" + to_string(distance) + "km) * 수량 * 0.1');";
        W.exec(sql_carbon_trans);

        double prod_emission = total_weight * 2.0;
        string sql_carbon_prod =
                "INSERT INTO CarbonRecord (ProjectID, EmissionAmount, Type, Criteria) "
                "VALUES (" + to_string(projectId) + ", " + to_string(prod_emission) +
                ", '생산', '부품수량 * 표준계수(2.0)');";
        W.exec(sql_carbon_prod);

        cout << ">> 3. 탄소 배출 기록 생성 완료 (운송: " << transport_emission << "kg)" << endl;
        W.commit();
        cout << "[Success] 발주 및 재고 처리 트랜잭션 완료!" << endl;
        return true;

    } catch (const std::exception &e) {
        cerr << "[Error] 트랜잭션 오류 발생 (Rollback): " << e.what() << endl;
        return false;
    }
}

void DB_Manager::printProjectList() {
    if (!conn || !conn->is_open()) return;
    try {
        pqxx::work W(*conn);
        pqxx::result R = W.exec("SELECT ProjectID, ShipName, ShipType FROM ShipProject ORDER BY ProjectID ASC");

        cout << "\n---------------- [프로젝트 목록] ----------------" << endl;
        cout << "ID\t| 선박명\t\t| 선종" << endl;
        for (const auto& row : R) {
            cout << row[0].as<int>() << "\t| " << row[1].c_str() << "\t\t| " << row[2].c_str() << endl;
        }
        cout << "-------------------------------------------------" << endl;
    } catch (const std::exception& e) { cout << "[Error] 목록 조회 실패" << endl; }
}

int DB_Manager::selectSupplierWithPaging() {
    if (!conn || !conn->is_open()) return -1;

    int page = 1;
    const int pageSize = 5;

    while (true) {
        system("cls");

        int offset = (page - 1) * pageSize;

        try {
            pqxx::work W(*conn);

            string sql =
                    "SELECT SupplierID, Name, ESGGrade, Country FROM Supplier "
                    "ORDER BY SupplierID ASC "
                    "LIMIT " + to_string(pageSize) + " OFFSET " + to_string(offset);

            pqxx::result R = W.exec(sql);

            cout << "\n---------------- [공급업체 목록 (Page " << page << ")] ----------------" << endl;
            cout << "ID\t| 업체명\t\t| ESG\t| 국가" << endl;
            cout << "--------------------------------------------------------" << endl;

            if (R.empty()) {
                cout << "       (더 이상 데이터가 없습니다)" << endl;
            } else {
                for (const auto& row : R) {
                    cout << row[0].as<int>() << "\t| "
                         << row[1].c_str() << "\t\t| "
                         << row[2].c_str() << "\t| "
                         << row[3].c_str() << endl;
                }
            }
            cout << "--------------------------------------------------------" << endl;
            cout << "[n] 다음 페이지  [p] 이전 페이지" << endl;
            cout << ">> 선택할 공급 업체 ID 입력 ([0] 취소): ";

            string input;
            cin >> input;

            if (input == "n" || input == "N") {
                if (!R.empty()) page++;
                else cout << ">> 마지막 페이지입니다." << endl;
            }
            else if (input == "p" || input == "P") {
                if (page > 1) page--;
                else cout << ">> 첫 페이지입니다." << endl;
            }
            else if (input == "0") {
                return -1;
            }
            else {
                try {
                    int selected_id = stoi(input);
                    return selected_id;
                } catch (...) {
                    cout << ">> 잘못된 입력입니다." << endl;
                    // 버퍼 비우기
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            }

        } catch (const std::exception& e) {
            cerr << "[Error] 목록 조회 실패: " << e.what() << endl;
            return -1;
        }
    }
}

void DB_Manager::printWarehouseList(int projectId) {
    if (!conn || !conn->is_open()) return;
    try {
        pqxx::work W(*conn);
        string sql = "SELECT W.WarehouseID, W.Name, W.Location FROM Warehouse W ";
        if(projectId != -1){
            sql += "JOIN ShipProject SP ON W.ShipyardID = SP.ShipyardID "
                   "WHERE SP.ProjectID = " + to_string(projectId) + " ";
        }
        sql+="ORDER BY W.WarehouseID ASC";
        pqxx::result R = W.exec(sql);

        cout << "\n---------------- [창고 목록] ----------------" << endl;
        cout << "ID\t| 창고명\t\t| 위치" << endl;
        for (const auto& row : R) {
            cout << row[0].as<int>() << "\t| " << row[1].c_str() << "\t\t| " << row[2].c_str() << endl;
        }
        cout << "---------------------------------------------" << endl;
    } catch (const std::exception& e) { cout << "[Error] 목록 조회 실패" << e.what()<<endl; }
}

void DB_Manager::printPartList() {
    if (!conn || !conn->is_open()) return;
    try {
        pqxx::work W(*conn);
        pqxx::result R = W.exec("SELECT PartID, Name, Unit, Spec FROM Part ORDER BY PartID ASC");

        cout << "\n---------------- [부품 목록] ----------------" << endl;
        cout << "ID\t| 부품명\t| 규격" << endl;
        for (const auto& row : R) {
            cout << row[0].as<int>() << "\t| " << row[1].c_str() << "\t| " << row[3].c_str() << endl;
        }
        cout << "---------------------------------------------" << endl;
    } catch (const std::exception& e) { cout << "[Error] 목록 조회 실패" << endl; }
}

void DB_Manager::showDashboard() {
    if (!conn || !conn->is_open()) return;

    printProjectList();
    int pid;
    cout << "\n>> 조회할 프로젝트 ID를 입력하세요: ";
    cin >> pid;

    try {
        pqxx::work W(*conn);

        string sql_basic =
                "SELECT ShipName, ShipType, ContractDate, DeliveryDate, State "
                "FROM ShipProject WHERE ProjectID = " + to_string(pid);

        pqxx::result r_basic = W.exec(sql_basic);

        if (r_basic.empty()) {
            cout << "[Error] 해당 ID의 프로젝트를 찾을 수 없습니다." << endl;
            return;
        }

        cout << "\n================ [프로젝트 대시보드] ================" << endl;
        cout << " [기본 정보]" << endl;
        cout << " - 선박명: " << r_basic[0][0].c_str() << endl;
        cout << " - 선종:   " << r_basic[0][1].c_str() << endl;
        cout << " - 계약일: " << r_basic[0][2].c_str() << endl;
        cout << " - 납기일: " << r_basic[0][3].c_str() << endl;
        cout << " - 상태:   " << r_basic[0][4].c_str() << endl;

        string sql_cost =
                "SELECT COALESCE(SUM(L.Quantity * L.UnitPrice), 0) "
                "FROM PurchaseOrder PO "
                "JOIN PurchaseOrderLine L ON PO.PurchaseID = L.PurchaseID "
                "WHERE PO.ProjectID = " + to_string(pid);

        pqxx::result r_cost = W.exec(sql_cost);
        long long total_cost = r_cost[0][0].as<long long>();

        cout << "\n [비용 분석]" << endl;
        cout << " - 총 발주 비용: " << total_cost << " 원" << endl;

        string sql_top3 =
                "SELECT S.Name, SUM(L.Quantity * L.UnitPrice) as Total "
                "FROM Supplier S "
                "JOIN PurchaseOrder PO ON S.SupplierID = PO.SupplierID "
                "JOIN PurchaseOrderLine L ON PO.PurchaseID = L.PurchaseID "
                "WHERE PO.ProjectID = " + to_string(pid) + " "
                                                           "GROUP BY S.Name "
                                                           "ORDER BY Total DESC LIMIT 3";

        pqxx::result r_top3 = W.exec(sql_top3);
        cout << " - 주요 공급업체 (Top 3):" << endl;
        if (r_top3.empty()) cout << "   (데이터 없음)" << endl;
        for (const auto& row : r_top3) {
            cout << "   * " << row[0].c_str() << ": " << row[1].as<long long>() << " 원" << endl;
        }

        string sql_carbon_trans =
                "SELECT COALESCE(SUM(CR.EmissionAmount), 0) "
                "FROM CarbonRecord CR "
                "JOIN Delivery D ON CR.DeliveryID = D.DeliveryID "
                "JOIN PurchaseOrder PO ON D.PurchaseID = PO.PurchaseID "
                "WHERE PO.ProjectID = " + to_string(pid) + " AND CR.Type = '운송'";

        string sql_carbon_other =
                "SELECT COALESCE(SUM(EmissionAmount), 0) "
                "FROM CarbonRecord WHERE ProjectID = " + to_string(pid);

        double carbon_trans = W.exec(sql_carbon_trans)[0][0].as<double>();
        double carbon_other = W.exec(sql_carbon_other)[0][0].as<double>();
        double total_carbon = carbon_trans + carbon_other;

        cout << "\n [탄소 발자국 (ESG)]" << endl;
        cout << " - 운송 배출량: " << carbon_trans << " kgCO2e" << endl;
        cout << " - 기타 배출량: " << carbon_other << " kgCO2e (보관/생산 등)" << endl;
        cout << " - 총 배출량:   " << total_carbon << " kgCO2e" << endl;


        double intensity = 0.0;
        if (total_cost > 0) {
            intensity = total_carbon / (total_cost / 1000000.0);
        }

        cout << "\n [핵심 성과 지표 (KPI)]" << endl;
        cout.precision(2);
        cout << fixed;
        cout << " - 탄소 집약도: " << intensity << " kgCO2e / 백만원" << endl;
        cout << "=====================================================" << endl;

    } catch (const std::exception &e) {
        cerr << "[Error] 대시보드 조회 중 오류: " << e.what() << endl;
    }
}

void DB_Manager::printSupplierOrderHistory(int supplierId) {
    if (!conn || !conn->is_open()) return;
    try {
        pqxx::work W(*conn);

        string sql =
                "SELECT PO.PurchaseID, SP.ShipName, PO.PurchaseDate, PO.PurchaseState, "
                "CASE WHEN D.State = '지연' THEN 'Y' ELSE 'N' END "
                "FROM PurchaseOrder PO "
                "JOIN ShipProject SP ON PO.ProjectID = SP.ProjectID "
                "LEFT JOIN Delivery D ON PO.PurchaseID = D.PurchaseID "
                "WHERE PO.SupplierID = " + to_string(supplierId) + " "
                                                                   "ORDER BY PO.PurchaseDate DESC, PO.PurchaseID DESC "
                                                                   "LIMIT 5";

        pqxx::result R = W.exec(sql);

        cout << "\n>> [선택한 공급업체의 최근 발주 이력 (최근 5건)]" << endl;
        cout << "발주ID\t| 프로젝트명\t\t| 발주일\t\t| 상태\t| 지연?" << endl;
        cout << "----------------------------------------------------------------" << endl;

        if (R.empty()) {
            cout << "       (발주 기록이 없습니다)" << endl;
        } else {
            for (const auto& row : R) {
                cout << row[0].as<int>() << "\t| "
                     << row[1].c_str() << "\t| "
                     << row[2].c_str() << "\t| "
                     << row[3].c_str() << "\t| "
                     << row[4].c_str() << endl;
            }
        }
        cout << "----------------------------------------------------------------" << endl;
        cout << "엔터를 누르면 목록으로 돌아갑니다...";
        cin.ignore(); cin.get(); // 잠시 대기

    } catch (const std::exception &e) {
        cerr << "[Error] 상세 조회 실패: " << e.what() << endl;
    }
}

void DB_Manager::showSupplierReport() {
    if (!conn || !conn->is_open()) return;

    int filter_option;
    cout << "\n--- [공급업체 성과 리포트] ---" << endl;
    cout << "1. 전체 공급업체 보기" << endl;
    cout << "2. ESG 등급별 필터링 (A/B/C/D)" << endl;
    cout << ">> 선택: ";
    cin >> filter_option;

    string where_clause = "";
    if (filter_option == 2) {
        char grade;
        cout << ">> 검색할 ESG 등급 입력 (A~D): ";
        cin >> grade;
        where_clause = " WHERE S.ESGGrade = '" + string(1, toupper(grade)) + "'";
    }

    int page = 1;
    const int pageSize = 5;

    while (true) {
        int offset = (page - 1) * pageSize;

        try {
            pqxx::work W(*conn);

            string sql =
                    "SELECT S.Name, S.ESGGrade, S.Country, "
                    "COUNT(PO.PurchaseID) as TotalOrders, "
                    "SUM(CASE WHEN D.State = '지연' THEN 1 ELSE 0 END) as DelayCount "
                    "FROM Supplier S "
                    "LEFT JOIN PurchaseOrder PO ON S.SupplierID = PO.SupplierID "
                    "LEFT JOIN Delivery D ON PO.PurchaseID = D.PurchaseID "
                    + where_clause +
                    " GROUP BY S.SupplierID, S.Name, S.ESGGrade, S.Country "
                    "ORDER BY TotalOrders DESC "
                    "LIMIT " + to_string(pageSize) + " OFFSET " + to_string(offset);

            pqxx::result R = W.exec(sql);
            W.commit();
            cout << "\n------------ [공급업체 리포트 (Page " << page << ")] ------------" << endl;
            cout << "업체명\t\t| ESG\t| 국가\t| 총 발주\t| 지연\t| 평가" << endl;
            cout << "--------------------------------------------------------" << endl;

            if (R.empty()) {
                if (page == 1) cout << "   (조건에 맞는 데이터가 없습니다)" << endl;
                else cout << "       (더 이상 데이터가 없습니다)" << endl;
            } else {
                for (const auto& row : R) {
                    string name = row[0].c_str();
                    string esg = row[1].c_str();
                    string country = row[2].c_str();
                    int total = row[3].as<int>();
                    int delay = row[4].as<int>();

                    string performance = "우수";
                    if (total > 0) {
                        double delay_rate = (double)delay / total;
                        if (delay_rate > 0.3) performance = "주의";
                        else if (delay_rate > 0.0) performance = "보통";
                    } else {
                        performance = "-";
                    }

                    cout << name << "\t| " << esg << "\t| " << country << "\t| "
                         << total << " 건\t| " << delay << " \t| " << performance << endl;
                }
            }
            cout << "--------------------------------------------------------" << endl;
            cout << "[n] 다음  [p] 이전  [0] 나가기 " << endl;
            cout << ">> 상세 조회 공급 업체ID 입력: ";

            string input;
            cin >> input;

            if (input == "n" || input == "N") {
                if (!R.empty()) page++;
                else cout << ">> 마지막 페이지입니다." << endl;
            }
            else if (input == "p" || input == "P") {
                if (page > 1) page--;
                else cout << ">> 첫 페이지입니다." << endl;
            }
            else if (input == "0") {
                break;
            }
            else {
                try {
                    int selected_id = stoi(input);
                    printSupplierOrderHistory(selected_id);
                } catch (...) {
                    cout << ">> 잘못된 입력입니다." << endl;
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            }
        } catch (const std::exception &e) {
            cerr << "[Error] 리포트 조회 실패: " << e.what() << endl;
            break;
        }
    }
}