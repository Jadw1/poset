#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

namespace {
    using NodeIndex = unsigned long;
    // mapa przechowujaca indexy wierzchołków
    using IndexMap = std::unordered_map<std::string, NodeIndex>;
    // set indexów do ktorych wskazuje relacja z danego wierzchołka
    using Edges = std::unordered_set<NodeIndex>;
    // mapa przechowująca sety wierzchołków do których prowadzi relacja z NodeIndex
    using PosetGraph = std::unordered_map<NodeIndex, Edges>;
    // <nodesIndexer, mapa indexów, graf>
    using Poset = std::variant<NodeIndex, IndexMap, PosetGraph>;

    std::unordered_map<unsigned long, Poset> PosetsMap;
    unsigned long PosetsIndexer = 0;

    std::optional<Poset> getPoset(unsigned long id) {
        auto el = PosetsMap.find(id);
        if(el == PosetsMap.end()) {
            return std::optional(std::nullopt);
        }

        return std::optional(el->second);
    }

    IndexMap& getIndexMap(Poset& poset) {
        return std::get<IndexMap>(poset);
    }

    PosetGraph& getPosetGraph(Poset& poset) {
        return std::get<PosetGraph>(poset);
    }
}





int main() {
    return 0;
}
