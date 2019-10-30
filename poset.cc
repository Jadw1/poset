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

    std::unordered_map<unsigned long, Poset> &PosetsMap() {
        static std::unordered_map<unsigned long, Poset> PosetsMap;

        return PosetsMap;
    }

    unsigned long &PosetsIndexer() {
        static unsigned long PosetsIndexer = 0;

        return PosetsIndexer;
    }

    std::optional<std::reference_wrapper<Poset>> getPoset(unsigned long id) {
        auto el = PosetsMap().find(id);
        if (el == PosetsMap().end()) {
            return std::nullopt;
        }

        return std::ref(el->second);
    }

    IndexMap &getIndexMap(Poset &poset) {
        return std::get<IndexMap>(poset);
    }

    PosetGraph &getPosetGraph(Poset &poset) {
        return std::get<PosetGraph>(poset);
    }

    Node &getPosetNode(PosetGraph &graph, NodeIndex index) {
        auto it = graph.find(index);

        return it->second;
    }

    Edges &getRelations(Node &node) {
        return node.first;
    }

    Edges &getReverseRelations(Node &node) {
        return node.second;
    }

    std::optional<NodeIndex> getNodeIndex(Poset &poset, char const *value) {
        auto &indexMap = getIndexMap(poset);

        if (value == nullptr) {
            return std::nullopt;
        }

        auto el = indexMap.find(value);
        if (el == indexMap.end()) {
            return std::nullopt;
        }
        return el->second;
    }

    bool isInPoset(Poset &poset, char const *value) {
        auto index = getNodeIndex(poset, value);

        return index.has_value();
    }

    std::optional<NodeIndex> removeNodeFromIndexMap(IndexMap &map, char const *value) {
        auto it = map.find(value);
        if (it == map.end()) {
            return std::nullopt;
        }

        map.erase(it);
        return it->second;
    }

    void removeNodeFromGraph(PosetGraph &graph, NodeIndex index) {
        auto &node = getPosetNode(graph, index);

        auto &rel = getRelations(node);
        auto &rev = getReverseRelations(node);

        for (auto el: rel) {
            auto &elRev = getReverseRelations(getPosetNode(graph, el));

            elRev.erase(index);
        }

        for (auto el: rev) {
            auto &elRel = getRelations(getPosetNode(graph, el));

            elRel.erase(index);
        }

        graph.erase(index);
    }

    bool removeNode(Poset &poset, char const *value) {
        auto index = removeNodeFromIndexMap(getIndexMap(poset), value);
        if (!index.has_value()) {
            return false;
        }

        removeNodeFromGraph(getPosetGraph(poset), index.value());
        return true;
    }

    bool checkRelation(Poset &poset, char const *value1, char const *value2) {
        auto index1 = getNodeIndex(poset, value1);
        auto index2 = getNodeIndex(poset, value2);

        if (!index1.has_value() || !index2.has_value()) {
            return false;
        }
        if (index1.value() == index2.value()) {
            return true;
        }

        auto &graph = getPosetGraph(poset);
        auto &node = getPosetNode(graph, index1.value());
        auto &relations = getRelations(node);

        auto rel = relations.find(index2.value());

        return rel != relations.end();
    }

    void clearPoset(Poset &poset) {
        auto &indexMap = getIndexMap(poset);
        auto &graph = getPosetGraph(poset);

        indexMap.clear();
        graph.clear();

        std::get<NodeIndex>(poset) = 0;
    }

    void addSingleRelation(PosetGraph &graph, NodeIndex index1, NodeIndex index2) {
        auto &set1 = getRelations(getPosetNode(graph, index1));
        auto &set2 = getReverseRelations(getPosetNode(graph, index2));

        set1.insert(index2);
        set2.insert(index1);
    }

    bool addRelation(Poset &poset, char const *value1, char const *value2) {
        auto index1 = getNodeIndex(poset, value1);
        auto index2 = getNodeIndex(poset, value2);

        if (!index1.has_value() || !index2.has_value()) {
            return false;
        }

        auto &graph = getPosetGraph(poset);

        auto &extendForwardSet = getRelations(getPosetNode(graph, index2.value()));
        auto &extendBackwardSet = getReverseRelations(getPosetNode(graph, index1.value()));

        for (auto index : extendForwardSet) {
            addSingleRelation(graph, index1.value(), index);
        }
        for (auto index : extendBackwardSet) {
            addSingleRelation(graph, index, index2.value());
        }

        addSingleRelation(graph, index1.value(), index2.value());

        return true;
    }

    bool canDeleteRelation(Poset &poset, char const *value1, char const *value2) {
        auto index1 = getNodeIndex(poset, value1);
        auto index2 = getNodeIndex(poset, value2);

        if (!index1.has_value() || !index2.has_value()) {
            return false;
        }

        if (index1.value() == index2.value()) {
            return false;
        }

        auto &graph = getPosetGraph(poset);
        auto &relations = getRelations(getPosetNode(graph, index1.value()));

        for (auto i : relations) {
            if (i != index2.value()) {
                auto &set = getRelations(getPosetNode(graph, i));

                auto it = set.find(index2.value());

                if (it != set.end()) {
                    return false;
                }
            }
        }

        return true;
    }

    void deleteRelation(Poset &poset, char const *value1, char const *value2) {
        auto index1 = getNodeIndex(poset, value1);
        auto index2 = getNodeIndex(poset, value2);

        if (!index1.has_value() || !index2.has_value()) {
            return;
        }

        auto &graph = getPosetGraph(poset);

        auto &rel = getRelations(getPosetNode(graph, index1.value()));
        auto &revRel = getReverseRelations(getPosetNode(graph, index2.value()));

        rel.erase(index2.value());
        revRel.erase(index1.value());
    }

    void addNode(Poset &poset, const char *value) {
        IndexMap &indexMap = getIndexMap(poset);
        PosetGraph &graph = getPosetGraph(poset);

        auto &index = std::get<NodeIndex>(poset);

        Edges rel;
        Edges revRel;
        Node node(rel, revRel);

        indexMap.insert({value, index});
        graph.insert({index, node});

        index++;
    }

    char const *nullConverter(char const *value) {
        if (value == NULL) return "NULL";
        else return value;
    }

    bool nullValue(char const *value) {
        if (value == NULL) return true;
        else return false;
    }

    void nullValueErr(char const *operation, char const *value1, char const *value2) {
        if (debug == true) {
            if (nullValue(value1)) std::cerr << operation << ": invalid value1 (NULL)" << std::endl;
            if (nullValue(value2)) std::cerr << operation << ": invalid value2 (NULL)" << std::endl;
        }
    }

    void elementErr(char const *operation, NodeIndex posetId, char const *value1, char const *value2) {
        if (debug == true) {
            std::cerr << operation << ": poset " << posetId << ", element \"";
            std::cerr << value1 << "\" or \"" << value2 << "\" does not exist" << std::endl;
        }
    }

    void relationStatus(char const *operation, NodeIndex posetId, char const *value1, char const *value2,
                        char const *status) {
        if (debug == true) {
            std::cerr << operation << ": poset " << posetId << ", relation (\"";
            std::cerr << value1 << "\", \"" << value2 << "\") " << status << std::endl;
        }
    }

    void posetErr(char const *operation, NodeIndex posetId) {
        if (debug == true) {
            std::cerr << operation << ": poset " << posetId << " does not exist" << std::endl;
        }
    }

    void debugOperation3arg(char const *operation, NodeIndex posetId, char const *value1, char const *value2) {
        if (debug == true) {
            std::cerr << operation << "(poset " << posetId << ", \"";
            std::cerr << nullConverter(value1) << "\", \"" << nullConverter(value2) << "\")" << std::endl;
        }
    }

    void debugOperation2arg(char const *operation, NodeIndex posetId, char const *value) {
        if (debug == true) {
            std::cerr << operation << "(poset " << posetId << ", \"" << nullConverter(value) << "\")" << std::endl;
        }
    }

    void debugOperation1arg(char const *operation, NodeIndex posetId) {
        if (debug == true) {
            std::cerr << operation << "(" << posetId << ")" << std::endl;
        }
    }

    void posetChange(char const *operation, NodeIndex posetId, char const *status) {
        if (debug == true) {
            std::cerr << operation << ": poset " << posetId << " " << status << std::endl;
        }
    }

    void posetStatus(char const *operation, NodeIndex posetId, char const *value, char const *status) {
        if (debug == true) {
            std::cerr << operation << ": poset " << posetId << ", element \"" << value << "\" " << status << std::endl;
        }
    }

    void posetNull(char const *operation) {
        if (debug == true) {
            std::cerr << operation << ": invalid value (NULL)" << std::endl;
        }
    }


}

unsigned long jnp1::poset_new() {
    if (debug == true) std::cerr << "poset_new()" << std::endl;
    IndexMap indexMap;
    PosetGraph graph;

    Poset poset(0, indexMap, graph);

    unsigned long index = PosetsIndexer();
    posetChange(__func__, index, "created");
    PosetsIndexer() += 1;
    PosetsMap().insert({index, poset});

    return index;
}

void jnp1::poset_delete(unsigned long id) {
    debugOperation1arg(__func__, id);
    auto el = PosetsMap().find(id);

    if (el != PosetsMap().end()) {
        posetChange(__func__, id, "deleted");
        PosetsMap().erase(el);
    }
    posetChange(__func__, id, "does not exist");
}

size_t jnp1::poset_size(unsigned long id) {
    debugOperation1arg(__func__, id);
    auto poset = getPoset(id);
    if (!poset.has_value()) {
        posetErr(__func__, id);
        return 0;
    }

    IndexMap &idxMap = getIndexMap(poset.value());
    if (debug == true)
        std::cerr << "poset_size: poset " << id << " contains " << idxMap.size() << " element(s)" << std::endl;
    return idxMap.size();
}

bool jnp1::poset_insert(unsigned long id, char const *value) {
    debugOperation2arg(__func__, id, value);
    auto poset = getPoset(id);

    if (!poset.has_value() || value == nullptr) {
        if (value == nullptr) posetNull(__func__);
        else posetErr(__func__, id);
        return false;
    }


    if (isInPoset(poset.value(), value)) {
        posetStatus(__func__, id, value, "already exists");
        return false;
    }

    posetStatus(__func__, id, value, "inserted");
    addNode(poset.value(), value);

    return true;
}

bool jnp1::poset_remove(unsigned long id, char const *value) {
    debugOperation2arg(__func__, id, value);
    auto poset = getPoset(id);

    if (!poset.has_value()) {
        posetErr(__func__, id);
        return false;
    }


    if (removeNode(poset.value(), value)) {
        posetStatus(__func__, id, value, "removed");
        return true;
    }
    posetStatus(__func__, id, value, "does not exist");
    return false;
}

bool jnp1::poset_add(unsigned long id, char const *value1, char const *value2) {
    debugOperation3arg(__func__, id, value1, value2);
    auto poset = getPoset(id);

    if (!poset.has_value()) {
        posetErr(__func__, id);
        return false;
    }


    if (nullValue(value1) || nullValue(value2)) {
        nullValueErr(__func__, value1, value2);
        return false;
    }


    if (checkRelation(poset.value(), value1, value2) || checkRelation(poset.value(), value2, value1)) {
        relationStatus(__func__, id, value1, value2, "cannot be added");
        return false;
    }


    if (addRelation(poset.value(), value1, value2)) {
        relationStatus(__func__, id, value1, value2, "added");
        return true;
    } else {
        elementErr(__func__, id, value1, value2);
        return false;
    }

}

bool jnp1::poset_test(unsigned long id, char const *value1, char const *value2) {
    debugOperation3arg(__func__, id, value1, value2);
    auto poset = getPoset(id);

    if (!poset.has_value()) {
        posetErr(__func__, id);
        return false;
    }

    if (nullValue(value1) || nullValue(value2)) {
        nullValueErr(__func__, value1, value2);
        return false;
    }

    if (checkRelation(poset.value(), value1, value2)) {
        relationStatus(__func__, id, value1, value2, "exists");
        return true;
    } else {
        relationStatus(__func__, id, value1, value2, "does not exists");
        return false;
    }
}

bool jnp1::poset_del(unsigned long id, char const *value1, char const *value2) {
    debugOperation3arg(__func__, id, value1, value2);
    auto poset = getPoset(id);

    if (!poset.has_value()) {
        posetErr(__func__, id);
        return false;
    }

    if (nullValue(value1) || nullValue(value2)) {
        nullValueErr(__func__, value1, value2);
        return false;
    }

    if (!checkRelation(poset.value(), value1, value2) || !canDeleteRelation(poset.value(), value1, value2)) {
        relationStatus(__func__, id, value1, value2, "cannot be deleted");
        return false;
    }

    deleteRelation(poset.value(), value1, value2);
    relationStatus(__func__, id, value1, value2, "deleted");
    return true;
}

void jnp1::poset_clear(unsigned long id) {
    debugOperation1arg(__func__, id);
    auto poset = getPoset(id);

    if (!poset.has_value()) {
        posetChange(__func__, id, "does not exist");
        return;
    }
    posetChange(__func__, id, "cleared");
    clearPoset(poset.value());
}