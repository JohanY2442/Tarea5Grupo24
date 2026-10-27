#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <stack>
#include <memory>

using namespace std;

// cada nodo guarda a quienes apunta
class Node {
public:
    void agregarVecino(int vecino) {
        vecinos.push_back(vecino);
    }

    vector<int> getVecinos() const {
        return vecinos;
    }

private:
    vector<int> vecinos;
};

// grafo dirigido, los nodos quedan privados
class Graph {
public:
    void addEdge(int origen, int destino) {
        crearNodo(origen);
        crearNodo(destino);
        nodos.at(origen).agregarVecino(destino);
    }

    bool existeNodo(int id) const {
        return nodos.find(id) != nodos.end();
    }

    // devuelve una copia para no exponer la estructura interna
    vector<int> getVecinos(int id) const {
        auto it = nodos.find(id);
        if (it == nodos.end()) {
            return vector<int>();
        }
        return it->second.getVecinos();
    }

private:
    void crearNodo(int id) {
        if (!existeNodo(id)) {
            nodos.insert({id, Node()});
        }
    }

    map<int, Node> nodos;
};

// Aqui esta la interfaz del iterador
class GraphIterator {
public:
    virtual ~GraphIterator() {}
    virtual bool hasNext() = 0;
    virtual int next() = 0;
};

// Para el recorrido en anchura, se usa una cola
class BFSIterator : public GraphIterator {
public:
    BFSIterator(const Graph& grafo, int inicio) : grafo(grafo) {
        if (grafo.existeNodo(inicio)) {
            cola.push(inicio);
            visitados.insert(inicio);
        }
    }

    bool hasNext() {
        return !cola.empty();
    }

    int next() {
        int actual = cola.front();
        cola.pop();

        vector<int> vecinos = grafo.getVecinos(actual);
        for (size_t i = 0; i < vecinos.size(); i++) {
            // se marca al encolar para no repetir nodos ni caer en ciclos
            if (visitados.find(vecinos[i]) == visitados.end()) {
                visitados.insert(vecinos[i]);
                cola.push(vecinos[i]);
            }
        }
        return actual;
    }

private:
    const Graph& grafo;
    queue<int> cola;
    set<int> visitados;
};

// Para el recorrido en profundidad, se usa una pila
class DFSIterator : public GraphIterator {
public:
    DFSIterator(const Graph& grafo, int inicio) : grafo(grafo) {
        if (grafo.existeNodo(inicio)) {
            pila.push(inicio);
        }
    }

    bool hasNext() {
        saltarVisitados();
        return !pila.empty();
    }

    int next() {
        saltarVisitados();
        int actual = pila.top();
        pila.pop();
        visitados.insert(actual);

        // Se apilan en orden inverso para procesar el primer vecino primero
        vector<int> vecinos = grafo.getVecinos(actual);
        for (int i = (int)vecinos.size() - 1; i >= 0; i--) {
            if (visitados.find(vecinos[i]) == visitados.end()) {
                pila.push(vecinos[i]);
            }
        }
        return actual;
    }

private:
    // Esto es para descartar los nodos de la pila que ya fueron visitados
    void saltarVisitados() {
        while (!pila.empty() && visitados.find(pila.top()) != visitados.end()) {
            pila.pop();
        }
    }

    const Graph& grafo;
    stack<int> pila;
    set<int> visitados;
};

int main() {
    Graph g;
    g.addEdge(1, 2);
    g.addEdge(1, 3);
    g.addEdge(2, 4);

    unique_ptr<GraphIterator> it = make_unique<BFSIterator>(g, 1);

    cout << "BFS: ";
    while (it->hasNext()) {
        cout << it->next() << " ";
    }
    cout << endl;

    // se cambia el iterador en tiempo de ejecucion
    it = make_unique<DFSIterator>(g, 1);

    cout << "DFS: ";
    while (it->hasNext()) {
        cout << it->next() << " ";
    }
    cout << endl;

    // esto es la prueba para ver que no hay bucle infinito
    g.addEdge(4, 1);

    it = make_unique<DFSIterator>(g, 1);

    cout << "DFS con ciclo: ";
    while (it->hasNext()) {
        cout << it->next() << " ";
    }
    cout << endl;

    return 0;
}