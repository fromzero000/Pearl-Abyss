#include <iostream>
#include <limits>
#include <cstdlib> // system()
#include "DB_Manager.hpp"

using namespace std;

void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int main() {
    //콘솔 인코딩을 UTF-8로 설정 (한글 깨짐 방지)
    system("chcp 65001 > nul");

    DB_Manager manager("db_config.txt");
    if (!manager.connect()) {
        cout << "프로그램을 종료합니다." << endl;
        return 1;
    }

    int choice = 0;
    while (true) {
        cout << "\n=========================================" << endl;
        cout << "   탄소중립 스마트 SCM 플랫폼 (HW10)" << endl;
        cout << "=========================================" << endl;
        cout << "1. 프로젝트 대시보드 조회" << endl;
        cout << "2. 신규 발주 및 입고 (트랜잭션 테스트)" << endl;
        cout << "3. 공급업체 리포트" << endl;
        cout << "0. 종료" << endl;
        cout << "=========================================" << endl;
        cout << ">> 선택: ";

        if (!(cin >> choice)) {
            clearInputBuffer();
            continue;
        }

        if (choice == 0) break;

        switch (choice) {
            case 1:
                manager.showDashboard();
                break;

            case 2: {
                int pid, sid, wid, dist;
                cout << "\n--- [신규 발주 등록] ---" << endl;
                manager.printProjectList();
                cout << "프로젝트 ID 입력: "; cin >> pid;
                sid = manager.selectSupplierWithPaging();
                if (sid == -1) {
                    cout << ">> 선택을 취소했습니다. 메뉴로 돌아갑니다." << endl;
                    break;
                }

                cout << ">> 예상 운송 거리(km) 입력: "; cin >> dist;
                manager.printWarehouseList(pid);
                cout << "입고할 창고 ID 입력: "; cin >> wid;

                vector<OrderItem> items;
                while (true) {
                    int partid, qty, price;
                    manager.printPartList();
                    cout << "\n[부품 추가] (종료하려면 PartID에 -1 입력)" << endl;
                    cout << " - 부품 ID: "; cin >> partid;
                    if (partid == -1) break;

                    cout << " - 수량: "; cin >> qty;
                    cout << " - 단가: "; cin >> price;

                    items.push_back({partid, qty, price});
                }

                if (items.empty()) {
                    cout << "[Warning] 부품이 입력되지 않아 취소합니다." << endl;
                } else {
                    cout << ">> 트랜잭션을 시작합니다..." << endl;
                    manager.processOrderTransaction(pid, sid, wid, dist, items);
                }
                break;
            }

            case 3:
                manager.showSupplierReport();
                break;

            default:
                cout << "[Warning] 올바른 번호를 입력하세요." << endl;
        }
    }

    return 0;
}