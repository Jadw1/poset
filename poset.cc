#include "poset.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace {
    using NodeIndex = unsigned long;
    using IndexMap = std::unordered_map<std::string, NodeIndex>;
    using Edges = std::unordered_set<NodeIndex>;
    using Node = std::pair<Edges, Edges>;
    using PosetGraph = std::unordered_map<NodeIndex, Node>;
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

    PosetGraph& getPosetGraph(Poset& poset) {
        return std::get<PosetGraph>(poset);
    }

    Node& getPosetNode(PosetGraph& graph, NodeIndex index) {
        auto it = graph.find(index);

        return it->second;
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

    std::optional<NodeIndex> removeNodeFromIndexMap(IndexMap& map, char const* value) {
        auto it = map.find(value);
        if(it == map.end()) {
            return std::nullopt;
        }

        map.erase(it);
        return it->second;
    }

    void removeNodeFromGraph(PosetGraph& graph, NodeIndex index) {
        auto& node = getPosetNode(graph, index);

        auto& rel = getRelations(node);
        auto& rev = getReverseRelations(node);

        for(auto el: rel) {
            auto& elRev = getReverseRelations(getPosetNode(graph, el));

            elRev.erase(index);
        }

        for(auto el: rev) {
            auto& elRel = getRelations(getPosetNode(graph, el));

            elRel.erase(index);
        }

        graph.erase(index);
    }

    bool removeNode(Poset& poset, char const* value) {
        auto index = removeNodeFromIndexMap(getIndexMap(poset), value);
        if(!index.has_value()) {
            return false;
        }

        removeNodeFromGraph(getPosetGraph(poset), index.value());
        return true;
    }

    bool checkRelation(Poset& poset, char const* value1, char const* value2) {
        auto index1 = getNodeIndex(poset, value1);
        auto index2 = getNodeIndex(poset, value2);

        if(!index1.has_value() || !index2.has_value()) {
            return false;
        }

        auto& graph = getPosetGraph(poset);
        auto& node = getPosetNode(graph, index1.value());
        auto& relations = getRelations(node);

        auto rel = relations.find(index2.value());

        return rel != relations.end();
    }

    void clearPoset(Poset& poset) {
        auto& indexMap = getIndexMap(poset);
        auto& graph = getPosetGraph(poset);

        indexMap.clear();
        graph.clear();

        std::get<NodeIndex>(poset) = 0;
    }
}


bool poset_remove(unsigned long id, char const* value) {
    auto* poset = getPoset(id);

    if(poset == nullptr) {
        return false;
    }

    return removeNode(*poset, value);
}

bool poset_test(unsigned long id, char const* value1, char const* value2) {
    auto* poset = getPoset(id);

    if(poset == nullptr) {
        return false;
    }

    return checkRelation(*poset, value1, value2);
}

void poset_clear(unsigned long id) {
    auto* poset = getPoset(id);

    if(poset == nullptr) {
        return;
    }

    clearPoset(*poset);
}


int main() {
    //for tests
    Poset po;
    auto& indexMap = std::get<IndexMap>(po);
    auto& graph = std::get<PosetGraph>(po);

    indexMap.insert({"a", 0});
    indexMap.insert({"b", 1});
    indexMap.insert({"c", 2});
    indexMap.insert({"d", 3});

    Node a({1, 2}, {});
    Node b({2}, {0, 3});
    Node c({}, {0, 1, 3});
    Node d({1, 2}, {});

    graph.insert({0, a});
    graph.insert({1, b});
    graph.insert({2, c});
    graph.insert({3, d});

    PosetsMap.insert({0, po});

    bool res = poset_remove(0, "b");

    auto* after = getPoset(0);
    return 0;
}
