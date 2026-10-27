#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;

class Account {
public:
    Account(string id, double saldo) {
        this->id = id;
        this->saldo = saldo;
    }

    void agregar(double monto) {
        saldo += monto;
    }

    void quitar(double monto) {
        saldo -= monto;
    }

    double getBalance() {
        return saldo;
    }

    string getId() {
        return id;
    }

private:
    string id;
    double saldo;
};

// guarda la info de una operacion ya hecha
class Transaction {
public:
    Transaction(string tipo, string cuenta, double monto) {
        this->tipo = tipo;
        this->cuenta = cuenta;
        this->monto = monto;
    }

    void print() {
        cout << tipo << " - cuenta " << cuenta
             << " - monto " << monto << endl;
    }

private:
    string tipo;
    string cuenta;
    double monto;
};

// singleton
class TransactionManager {
public:
    static TransactionManager& getInstance() {
        static TransactionManager instancia;
        return instancia;
    }

    // no se puede copiar ni asignar
    TransactionManager(const TransactionManager&) = delete;
    TransactionManager& operator=(const TransactionManager&) = delete;

    void deposit(Account* cuenta, double monto) {
        lock_guard<mutex> lock(m);
        cuenta->agregar(monto);
        historial.emplace_back("DEPOSITO", cuenta->getId(), monto);
    }

    void withdraw(Account* cuenta, double monto) {
        lock_guard<mutex> lock(m);
        if (cuenta->getBalance() < monto) {
            cout << "saldo insuficiente" << endl;
            return;
        }
        cuenta->quitar(monto);
        historial.emplace_back("RETIRO", cuenta->getId(), monto);
    }

    size_t totalOperaciones() {
        lock_guard<mutex> lock(m);
        return historial.size();
    }

    void mostrarHistorial() {
        lock_guard<mutex> lock(m);
        for (size_t i = 0; i < historial.size(); i++) {
            historial[i].print();
        }
    }

private:
    TransactionManager() {}

    mutex m;
    vector<Transaction> historial;
};

int main() {
    auto& manager = TransactionManager::getInstance();
    Account account("A1", 1000);

    vector<thread> hilos;

    for (int i = 0; i < 10; ++i) {
        hilos.emplace_back([&]() {
            manager.deposit(&account, 100);
            manager.withdraw(&account, 50);
        });
    }

    for (auto& h : hilos) {
        h.join();
    }

    cout << "Balance final: " << account.getBalance() << endl;
    cout << "Direccion manager: " << &TransactionManager::getInstance() << endl;
    cout << "Operaciones registradas: " << manager.totalOperaciones() << endl;

    return 0;
}