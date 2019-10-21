#include <iostream>
#include <string>
#include <set>
#include <map>

using namespace std;
// id posetu, element, nasz identyfikator
map < pair< int, string>, int > M;

//id posetu, id, id      //to jest nasz graf. Trzymamy id posetu i zbiór krawędzi.
map < int, set< pair< int,int > > > G;

//id posetu, zbiór wierzcholków naszych id,
map < int, set< int > > V;







int main() {
    cout << "Hello, World!" << endl;
    return 0;
}
