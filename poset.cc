#include "poset.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace {
    using NodeIndex = unsigned long;
    // mapa przechowujaca indexy wierzchołków
    using IndexMap = std::unordered_map<std::string, NodeIndex>;
    // set indexów do ktorych wskazuje relacja z danego wierzchołka
    using Edges = std::unordered_set<NodeIndex>;
    // wierzchołek v w grafie < krawedzie takie ze v r e, krawedzie e r v>
    using Node = std::pair<Edges, Edges>;
    // mapa przechowująca Nodes
    using PosetGraph = std::unordered_map<NodeIndex, Node>;
    // <nodesIndexer, mapa indexów, graf>
    using Poset = std::tuple<NodeIndex, IndexMap, PosetGraph>;

    std::unordered_map<unsigned long, Poset> PosetsMap;
    unsigned long PosetsIndexer = 0;

    unsigned long createNewPoset() {
        Poset poset;

        unsigned long index = PosetsIndexer;
        PosetsIndexer++;
        PosetsMap.insert({index, poset});

        return index;
    }

    Poset* getPoset(unsigned long id) {
        auto el = PosetsMap.find(id);
        if(el == PosetsMap.end()) {
            return nullptr;
        }

        return &(el->second);
    }

    IndexMap& getIndexMap(Poset& poset) {
        return std::get<IndexMap>(poset);
    }

    void setIndexMap(Poset& poset, IndexMap* iMap) {
        std::get<IndexMap>(poset)=(*iMap);
    }

    PosetGraph& getPosetGraph(Poset& poset) {
        return std::get<PosetGraph>(poset);
    }

    Edges& getRelations(Node& node) {
        return node.first;
    }

    Edges& getReverseRelations(Node& node) {
        return node.second;
    }

    std::optional<NodeIndex> getNodeIndex(Poset& poset, char const* value) {
        auto& indexMap = getIndexMap(poset);

        auto el = indexMap.find(value);
        if(el == indexMap.end()) {
            return std::nullopt;
        }
        return el->second;
    }

    NodeIndex poset_new() {
        Poset poset;

        NodeIndex index = PosetsIndexer;
        PosetsIndexer++;
        PosetsMap.insert({index, poset});

        return index;
    }

    void poset_delete(NodeIndex id){
        auto el = PosetsMap.find(id);
        if(el != PosetsMap.end()){
            PosetsMap.erase(el);
        }
    }

    size_t poset_size(NodeIndex id){
        Poset* poset =  getPoset(id);
        if(poset == nullptr) return 0;
        IndexMap idxMap =getIndexMap(*poset);
        return idxMap.size();
    }


    bool existenceInPoset(Poset poset,char const *value) {
        IndexMap indexMap = getIndexMap(poset);
        auto el = indexMap.find(value);
        if(el != indexMap.end()){
            return false;
        } else {
            return true;
        }
    }

    bool poset_insert(unsigned long id, char const *value){
        Poset* poset = getPoset(id);
        if(poset!= nullptr && existenceInPoset(*poset,value)){
            IndexMap idxMap =getIndexMap(*poset);
            idxMap.insert({value,id});
            setIndexMap(*poset,&idxMap);
            return true;
        } else {
            return false;
        }
    }


}


int main() {
    return 0;
}
