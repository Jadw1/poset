#include "poset.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace {
    using NodeIndex = unsigned long;
    using IndexMap = std::unordered_map<std::string, NodeIndex>;
    using Edges = std::unordered_set<NodeIndex>;
    using Node = std::pair<Edges, Edges>;
    using PosetGraph = std::unordered_map<NodeIndex, Node>;
    using Poset = std::tuple<NodeIndex, IndexMap, PosetGraph>;

    std::unordered_map<unsigned long, Poset> PosetsMap;
    unsigned long PosetsIndexer = 0;

    std::optional<std::reference_wrapper<Poset>> getPoset(unsigned long id) {
        auto el = PosetsMap.find(id);
        if (el == PosetsMap.end()) {
            return std::nullopt;
        }

        return std::ref(el->second);
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

        if(value == nullptr) {
            return std::nullopt;
        }

        auto el = indexMap.find(value);
        if (el == indexMap.end()) {
            return std::nullopt;
        }
        return el->second;
    }

    bool isInPoset(Poset& poset, char const* value) {
        auto index = getNodeIndex(poset, value);

        return index.has_value();
    }

    std::optional<NodeIndex> removeNodeFromIndexMap(IndexMap& map, char const* value) {
        auto it = map.find(value);
        if (it == map.end()) {
            return std::nullopt;
        }

        map.erase(it);
        return it->second;
    }

    void removeNodeFromGraph(PosetGraph& graph, NodeIndex index) {
        auto& node = getPosetNode(graph, index);

        auto& rel = getRelations(node);
        auto& rev = getReverseRelations(node);

        for (auto el: rel) {
            auto& elRev = getReverseRelations(getPosetNode(graph, el));

            elRev.erase(index);
        }

        for (auto el: rev) {
            auto& elRel = getRelations(getPosetNode(graph, el));

            elRel.erase(index);
        }

        graph.erase(index);
    }

    bool removeNode(Poset& poset, char const* value) {
        auto index = removeNodeFromIndexMap(getIndexMap(poset), value);
        if (!index.has_value()) {
            return false;
        }

        removeNodeFromGraph(getPosetGraph(poset), index.value());
        return true;
    }

    bool checkRelation(Poset& poset, char const* value1, char const* value2) {
        auto index1 = getNodeIndex(poset, value1);
        auto index2 = getNodeIndex(poset, value2);

        if (!index1.has_value() || !index2.has_value()) {
            return false;
        }
        if (index1.value() == index2.value()) {
            return true;
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

    void addSingleRelation(PosetGraph& graph, NodeIndex index1, NodeIndex index2) {
        auto& set1 = getRelations(getPosetNode(graph, index1));
        auto& set2 = getReverseRelations(getPosetNode(graph, index2));

        set1.insert(index2);
        set2.insert(index1);
    }

    bool addRelation(Poset& poset, char const* value1, char const* value2) {
        auto index1 = getNodeIndex(poset, value1);
        auto index2 = getNodeIndex(poset, value2);

        if (!index1.has_value() || !index2.has_value()) {
            return false;
        }

        auto& graph = getPosetGraph(poset);

        auto& extendForwardSet = getRelations(getPosetNode(graph, index2.value()));
        auto& extendBackwardSet = getReverseRelations(getPosetNode(graph, index1.value()));

        for (auto index : extendForwardSet) {
            addSingleRelation(graph, index1.value(), index);
        }
        for (auto index : extendBackwardSet) {
            addSingleRelation(graph, index, index2.value());
        }

        addSingleRelation(graph, index1.value(), index2.value());

        return true;
    }

    bool canDeleteRelation(Poset& poset, char const* value1, char const* value2) {
        auto index1 = getNodeIndex(poset, value1);
        auto index2 = getNodeIndex(poset, value2);

        if (!index1.has_value() || !index2.has_value()) {
            return false;
        }

        if (index1.value() == index2.value()) {
            return false;
        }

        auto& graph = getPosetGraph(poset);
        auto& relations = getRelations(getPosetNode(graph, index1.value()));

        for (auto i : relations) {
            if (i != index2.value()) {
                auto& set = getRelations(getPosetNode(graph, i));

                auto it = set.find(index2.value());

                if (it != set.end()) {
                    return false;
                }
            }
        }

        return true;
    }

    void deleteRelation(Poset& poset, char const* value1, char const* value2) {
        auto index1 = getNodeIndex(poset, value1);
        auto index2 = getNodeIndex(poset, value2);

        if (!index1.has_value() || !index2.has_value()) {
            return;
        }

        auto& graph = getPosetGraph(poset);

        auto& rel = getRelations(getPosetNode(graph, index1.value()));
        auto& revRel = getReverseRelations(getPosetNode(graph, index2.value()));

        rel.erase(index2.value());
        revRel.erase(index1.value());
    }

    void addNode(Poset& poset, const char* value) {
        IndexMap& indexMap = getIndexMap(poset);
        PosetGraph& graph = getPosetGraph(poset);

        auto& index = std::get<NodeIndex>(poset);

        Node node;

        indexMap.insert({value, index});
        graph.insert({index, node});

        index++;
    }
}

unsigned long jnp1::poset_new() {
    Poset poset;

    unsigned long index = PosetsIndexer;
    PosetsIndexer++;
    PosetsMap.insert({index, poset});

    return index;
}

void jnp1::poset_delete(unsigned long id) {
    auto el = PosetsMap.find(id);

    if (el != PosetsMap.end()) {
        PosetsMap.erase(el);
    }
}

size_t jnp1::poset_size(unsigned long id) {
    auto poset = getPoset(id);
    if (!poset.has_value()) {
        return 0;
    }

    IndexMap& idxMap = getIndexMap(poset.value());
    return idxMap.size();
}

bool jnp1::poset_insert(unsigned long id, char const* value) {
    auto poset = getPoset(id);

    if (!poset.has_value() || value == nullptr) {
        return false;
    }

    if (isInPoset(poset.value(), value)) {
        return false;
    }

    addNode(poset.value(), value);

    return true;
}

bool jnp1::poset_remove(unsigned long id, char const* value) {
    auto poset = getPoset(id);

    if (!poset.has_value()) {
        return false;
    }

    return removeNode(poset.value(), value);
}

bool jnp1::poset_add(unsigned long id, char const* value1, char const* value2) {
    auto poset = getPoset(id);

    if (!poset.has_value()) {
        return false;
    }

    if (checkRelation(poset.value(), value1, value2) || checkRelation(poset.value(), value2, value1)) {
        return false;
    }

    return addRelation(poset.value(), value1, value2);
}

bool jnp1::poset_test(unsigned long id, char const* value1, char const* value2) {
    auto poset = getPoset(id);

    if (!poset.has_value()) {
        return false;
    }

    return checkRelation(poset.value(), value1, value2);
}

bool jnp1::poset_del(unsigned long id, char const* value1, char const* value2) {
    auto poset = getPoset(id);

    if (!poset.has_value()) {
        return false;
    }

    if (!checkRelation(poset.value(), value1, value2) || !canDeleteRelation(poset.value(), value1, value2)) {
        return false;
    }

    deleteRelation(poset.value(), value1, value2);
    return true;
}

void jnp1::poset_clear(unsigned long id) {
    auto poset = getPoset(id);

    if (!poset.has_value()) {
        return;
    }

    clearPoset(poset.value());
}

/*int main () {
    auto id = jnp1::poset_new();
    jnp1::poset_insert(id, "a");
    jnp1::poset_insert(id, "b");
    jnp1::poset_insert(id, "c");
    jnp1::poset_add(id, "a", "b");
    jnp1::poset_add(id, "a", "b");


    auto r1 = jnp1::poset_test(id, "a", "b");
    auto d1 = jnp1::poset_del(id, "a", "b");
    auto r2 = jnp1::poset_test(id, "a", "b");
    auto d2 = jnp1::poset_del(id, "a", "b");
}*/