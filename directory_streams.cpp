#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <windows.h>

using namespace std;

// Структура одного запису в довіднику
struct CompanyRecord {
    string name;      // 1. Назва фірми
    string owner;     // 2. Власник
    string phone;     // 3. Телефон
    string address;   // 4. Адреса
    string activity;  // 5. Рід діяльності

    void print(size_t index) const {
        cout << "--------------------------------------------------\n";
        cout << "Запис #" << index + 1 << '\n';
        cout << "  Назва фірми:    " << name << '\n';
        cout << "  Власник:        " << owner << '\n';
        cout << "  Телефон:        " << phone << '\n';
        cout << "  Адреса:         " << address << '\n';
        cout << "  Рід діяльності: " << activity << '\n';
    }

    // Потоковий запис об'єкта у файл (роздільник полів — '|')
    friend ostream& operator<<(ostream& os, const CompanyRecord& rec) {
        os << rec.name << '|'
           << rec.owner << '|'
           << rec.phone << '|'
           << rec.address << '|'
           << rec.activity << '\n';
        return os;
    }

    // Потокове зчитування об'єкта з файлу
    friend istream& operator>>(istream& is, CompanyRecord& rec) {
        string line;
        if (!getline(is, line) || line.empty()) {
            is.setstate(ios::failbit);
            return is;
        }

        stringstream ss(line);
        if (!getline(ss, rec.name, '|') ||
            !getline(ss, rec.owner, '|') ||
            !getline(ss, rec.phone, '|') ||
            !getline(ss, rec.address, '|') ||
            !getline(ss, rec.activity)) {
            is.setstate(ios::failbit);
        }
        return is;
    }
};

// =========================================================
// Клас «Довідник» із збереженням даних у файловий потік
// =========================================================
class Directory {
private:
    string fileName;
    vector<CompanyRecord> records;

    // Завантаження всіх записів із файлу при запуску
    void loadFromFile() {
        records.clear();
        ifstream inFile(fileName);
        if (!inFile.is_open()) {
            return;
        }

        CompanyRecord temp;
        while (inFile >> temp) {
            records.push_back(temp);
        }
        inFile.close();
    }

    // Дозапис нового запису в кінець файлу
    void appendToFile(const CompanyRecord& rec) const {
        ofstream outFile(fileName, ios::app);
        if (!outFile.is_open()) {
            cerr << "Помилка відкриття файлу '" << fileName << "' для запису!\n";
            return;
        }
        outFile << rec;
        outFile.close();
    }

    // Допоміжний метод пошуку за вибраним полем (підтримує частковий збіг)
    void searchByField(int fieldType, const string& query, const string& label) const {
        cout << "\n=== Результати пошуку (" << label << ": \"" << query << "\") ===\n";
        bool found = false;

        for (size_t i = 0; i < records.size(); ++i) {
            const string* fieldPtr = nullptr;
            switch (fieldType) {
                case 1: fieldPtr = &records[i].name; break;
                case 2: fieldPtr = &records[i].owner; break;
                case 3: fieldPtr = &records[i].phone; break;
                case 4: fieldPtr = &records[i].activity; break;
            }

            if (fieldPtr && fieldPtr->find(query) != string::npos) {
                records[i].print(i);
                found = true;
            }
        }

        if (!found) {
            cout << "Записів за вказаним запитом не знайдено.\n";
        }
        cout << "--------------------------------------------------\n";
    }

public:
    Directory(const string& fileName) : fileName(fileName) {
        loadFromFile();

        // Якщо файл ще не існує або порожній — створюємо стартову базу для зручної перевірки
        if (records.empty()) {
            addRecord({"БудТехСервіс", "Коваленко Олександр Іванович", "+380671234567", "м. Одеса, вул. Дерибасівська, 15", "Будівництво та проектування"});
            addRecord({"SoftCore UA", "Бондаренко Дмитро Олегович", "+380509876543", "м. Київ, пр-т Перемоги, 42", "Розробка програмного забезпечення"});
            addRecord({"АгроСвіт", "Мельник Ірина Сергіївна", "+380935554433", "м. Львів, вул. Городоцька, 110", "Аграрна логістика"});
        }
    }

    // Додавання нового запису до пам'яті та файлу
    void addRecord(const CompanyRecord& rec) {
        records.push_back(rec);
        appendToFile(rec);
    }

    // Виведення всіх записів на екран
    void displayAll() const {
        cout << "\n=== Усі записи довідника (файл: " << fileName << ", всього: " << records.size() << ") ===\n";
        if (records.empty()) {
            cout << "Довідник порожній.\n";
            return;
        }
        for (size_t i = 0; i < records.size(); ++i) {
            records[i].print(i);
        }
        cout << "--------------------------------------------------\n";
    }

    // 1. Пошук за назвою фірми
    void searchByName(const string& name) const {
        searchByField(1, name, "Назва фірми");
    }

    // 2. Пошук за власником
    void searchByOwner(const string& owner) const {
        searchByField(2, owner, "Власник");
    }

    // 3. Пошук за номером телефону
    void searchByPhone(const string& phone) const {
        searchByField(3, phone, "Телефон");
    }

    // 4. Пошук за родом діяльності
    void searchByActivity(const string& activity) const {
        searchByField(4, activity, "Рід діяльності");
    }
};

// Безпечне зчитування рядка з консолі (із заміною випадкового символу '|')
string readLineSafe(const string& prompt) {
    cout << prompt;
    string input;
    getline(cin, input);
    for (char& ch : input) {
        if (ch == '|') ch = '/';
    }
    return input;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    Directory directory("directory.txt");
    int choice = -1;

    while (true) {
        cout << "\n================ ДОВІДНИК ПІДПРИЄМСТВ ================\n"
             << "1. Вивести всі записи\n"
             << "2. Додати новий запис\n"
             << "3. Пошук за назвою фірми\n"
             << "4. Пошук за власником\n"
             << "5. Пошук за номером телефону\n"
             << "6. Пошук за родом діяльності\n"
             << "0. Вихід\n"
             << "Оберіть дію: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Некоректне введення. Введіть цифру від 0 до 6.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 0) {
            cout << "Роботу завершено. Усі дані збережено у файлі directory.txt.\n";
            break;
        }

        switch (choice) {
            case 1:
                directory.displayAll();
                break;

            case 2: {
                cout << "\n--- Додавання нового запису ---\n";
                CompanyRecord newRec;
                newRec.name = readLineSafe("1. Назва фірми: ");
                newRec.owner = readLineSafe("2. Власник: ");
                newRec.phone = readLineSafe("3. Телефон: ");
                newRec.address = readLineSafe("4. Адреса: ");
                newRec.activity = readLineSafe("5. Рід діяльності: ");

                directory.addRecord(newRec);
                cout << "Запис успішно додано та збережено у файл!\n";
                break;
            }

            case 3: {
                string query = readLineSafe("Введіть назву фірми (або її частину): ");
                directory.searchByName(query);
                break;
            }

            case 4: {
                string query = readLineSafe("Введіть ПІБ власника (або частину): ");
                directory.searchByOwner(query);
                break;
            }

            case 5: {
                string query = readLineSafe("Введіть номер телефону (або частину): ");
                directory.searchByPhone(query);
                break;
            }

            case 6: {
                string query = readLineSafe("Введіть рід діяльності (або частину): ");
                directory.searchByActivity(query);
                break;
            }

            default:
                cout << "Невідомий пункт меню. Спробуйте ще раз.\n";
                break;
        }
    }

    return 0;
}