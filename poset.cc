#include "poset.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <iostream>
#include <string>

namespace {
    std::ostream& DebugStream() {
        static std::ios_base::Init init;
        return std::cerr;
    }

#ifdef NDEBUG
    static bool const debug = false;
#else
    static bool const debug = true;
#endif

#define PRINT_DEBUG(x) do {\
    if(debug) {\
        DebugStream() << x << std::endl;\
    }\
} while(0)

    using NodeIndex = unsigned long;
    using IndexMap = std::unordered_map<std::string, NodeIndex>;
    using Edges = std::unordered_set<NodeIndex>;
    using Node = std::pair<Edges, Edges>;
    using PosetGraph = std::unordered_map<NodeIndex, Node>;
    using Poset = std::tuple<NodeIndex, IndexMap, PosetGraph>;

    std::unordered_map<unsigned long, Poset>& PosetsMap() {
        static std::unordered_map<unsigned long, Poset> PosetsMap;

        return PosetsMap;
    }

    unsigned long PosetsIndexer() {
        static unsigned long PosetsIndexer = 0;

        PosetsIndexer++;
        return PosetsIndexer - 1;
    }

    std::optional<std::reference_wrapper<Poset>> getPoset(unsigned long id) {
        auto el = PosetsMap().find(id);
        if (el == PosetsMap().end()) {
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

        if (value == nullptr) {
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

    bool isPairInPoset(Poset& poset, char const* value1, char const* value2) {
        auto index1 = getNodeIndex(poset, value1);
        auto index2 = getNodeIndex(poset, value2);

        return index1.has_value() && index2.has_value();
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

        Edges rel;
        Edges revRel;
        Node node(rel, revRel);

        indexMap.insert({value, index});
        graph.insert({index, node});

        index++;
    }
}

unsigned long jnp1::poset_new() {
    PRINT_DEBUG("poset_new()");

    IndexMap indexMap;
    PosetGraph graph;

    Poset poset(0, indexMap, graph);

    unsigned long index = PosetsIndexer();
    PosetsMap().insert({index, poset});

    PRINT_DEBUG("poset_new: poset " + std::to_string(index) + " created");
    return index;
}

void jnp1::poset_delete(unsigned long id) {
    PRINT_DEBUG("poset_delete(" + std::to_string(id) + ")");

    auto el = PosetsMap().find(id);

    if (el != PosetsMap().end()) {
        PosetsMap().erase(el);

        PRINT_DEBUG("poset_delete: poset " + std::to_string(id) + " deleted");
    }
    else {
        PRINT_DEBUG("poset_delete: poset " + std::to_string(id) + " does not exist");
    }
}

size_t jnp1::poset_size(unsigned long id) {
    PRINT_DEBUG("poset_size(" + std::to_string(id) + ")");

    auto poset = getPoset(id);
    if (!poset.has_value()) {

        PRINT_DEBUG("poset_size: poset " + std::to_string(id) + " does not exist");
        return 0;
    }

    IndexMap& idxMap = getIndexMap(poset.value());
    size_t size = idxMap.size();

    PRINT_DEBUG("poset_size: poset " + std::to_string(id) + " contains " + std::to_string(size) + " element(s)");
    return size;
}

bool jnp1::poset_insert(unsigned long id, char const* value) {

    std::string commandParam = (value != nullptr) ? value : "NULL";
    PRINT_DEBUG("poset_insert(" + std::to_string(id) + ", \"" + commandParam + "\")");

    auto poset = getPoset(id);

    if (!poset.has_value()) {
        PRINT_DEBUG("poset_insert: poset " + std::to_string(id) + " does not exist");
        return false;
    }
    else if (value == nullptr) {
        PRINT_DEBUG("poset_insert: invalid value (NULL)");
        return false;
    }

    if (isInPoset(poset.value(), value)) {
        PRINT_DEBUG("poset_insert: poset " + std::to_string(id) + ", element \"" + commandParam + "\" already exists");
        return false;
    }

    addNode(poset.value(), value);

    PRINT_DEBUG("poset_insert: poset " + std::to_string(id) + ", element \"" + commandParam + "\" inserted");
    return true;
}

bool jnp1::poset_remove(unsigned long id, char const* value) {
    std::string commandParam = (value != nullptr) ? value : "NULL";
    PRINT_DEBUG("poset_remove(" + std::to_string(id) + ", \"" + commandParam + "\")");

    auto poset = getPoset(id);

    if (!poset.has_value()) {
        PRINT_DEBUG("poset_remove: poset " + std::to_string(id) + " does not exist");
        return false;
    }
    if (value == nullptr) {
        PRINT_DEBUG("poset_remove: invalid value (NULL)");
        return false;
    }

    bool result = removeNode(poset.value(), value);
    if (result) {
        PRINT_DEBUG("poset_remove: poset " + std::to_string(id) + ", element \"" + commandParam + "\" removed");
    }
    else {
        PRINT_DEBUG("poset_remove: poset " + std::to_string(id) + ", element \"" + commandParam + "\" does not exist");
    }

    return result;
}

bool jnp1::poset_add(unsigned long id, char const* value1, char const* value2) {
    std::string commandParam1 = (value1 != nullptr) ? value1 : "NULL";
    std::string commandParam2 = (value2 != nullptr) ? value2 : "NULL";
    PRINT_DEBUG("poset_add(" + std::to_string(id) + ", \"" + commandParam1 + "\", \"" + commandParam2 + "\")");

    auto poset = getPoset(id);

    bool invalidArgument = false;
    if (!poset.has_value()) {
        PRINT_DEBUG("poset_add: poset " + std::to_string(id) + " does not exist");
        return false;
    }
    if (value1 == nullptr) {
        PRINT_DEBUG("poset_add: invalid value1 (NULL)");
        invalidArgument = true;
    }
    if (value2 == nullptr) {
        PRINT_DEBUG("poset_add: invalid value2 (NULL)");
        invalidArgument = true;
    }
    if (invalidArgument) {
        return false;
    }

    if (!isPairInPoset(poset.value(), value1, value2)) {
        PRINT_DEBUG("poset_add: poset " + std::to_string(id) + ", element \"" + commandParam1 + "\" or \""
                    + commandParam2 + "\" does not exist");
        return false;
    }

    if (checkRelation(poset.value(), value1, value2) || checkRelation(poset.value(), value2, value1)) {
        PRINT_DEBUG(
            "poset_add: poset " + std::to_string(id) + ", relation (\"" + commandParam1 + "\", \"" + commandParam2 +
            "\") cannot be added");
        return false;
    }

    PRINT_DEBUG("poset_add: poset " + std::to_string(id) + ", relation (\"" + commandParam1 + "\", \"" + commandParam2 +
                "\") added");
    return addRelation(poset.value(), value1, value2);
}

bool jnp1::poset_test(unsigned long id, char const* value1, char const* value2) {
    std::string commandParam1 = (value1 != nullptr) ? value1 : "NULL";
    std::string commandParam2 = (value2 != nullptr) ? value2 : "NULL";
    PRINT_DEBUG("poset_test(" + std::to_string(id) + ", \"" + commandParam1 + "\", \"" + commandParam2 + "\")");

    auto poset = getPoset(id);

    bool invalidArgument = false;
    if (!poset.has_value()) {
        PRINT_DEBUG("poset_test: poset " + std::to_string(id) + " does not exist");
        return false;
    }
    if (value1 == nullptr) {
        PRINT_DEBUG("poset_test: invalid value1 (NULL)");
        invalidArgument = true;
    }
    if (value2 == nullptr) {
        PRINT_DEBUG("poset_test: invalid value2 (NULL)");
        invalidArgument = true;
    }
    if (invalidArgument) {
        return false;
    }

    if (!isPairInPoset(poset.value(), value1, value2)) {
        PRINT_DEBUG("poset_test: poset " + std::to_string(id) + ", element \"" + commandParam1 + "\" or \""
                    + commandParam2 + "\" does not exist");
        return false;
    }

    bool result = checkRelation(poset.value(), value1, value2);
    if (result) {
        PRINT_DEBUG(
            "poset_test: poset " + std::to_string(id) + ", relation (\"" + commandParam1 + "\", \"" + commandParam2 +
            "\") exists");
    }
    else {
        PRINT_DEBUG(
            "poset_test: poset " + std::to_string(id) + ", relation (\"" + commandParam1 + "\", \"" + commandParam2 +
            "\") does not exist");
    }
    return result;
}

bool jnp1::poset_del(unsigned long id, char const* value1, char const* value2) {
    std::string commandParam1 = (value1 != nullptr) ? value1 : "NULL";
    std::string commandParam2 = (value2 != nullptr) ? value2 : "NULL";
    PRINT_DEBUG("poset_del(" + std::to_string(id) + ", \"" + commandParam1 + "\", \"" + commandParam2 + "\")");

    auto poset = getPoset(id);

    bool invalidArgument = false;
    if (!poset.has_value()) {
        PRINT_DEBUG("poset_del: poset " + std::to_string(id) + " does not exist");
        return false;
    }
    if (value1 == nullptr) {
        PRINT_DEBUG("poset_del: invalid value1 (NULL)");
        invalidArgument = true;
    }
    if (value2 == nullptr) {
        PRINT_DEBUG("poset_del: invalid value2 (NULL)");
        invalidArgument = true;
    }
    if (invalidArgument) {
        return false;
    }

    if (!isPairInPoset(poset.value(), value1, value2)) {
        PRINT_DEBUG("poset_del: poset " + std::to_string(id) + ", element \"" + commandParam1 + "\" or \""
                    + commandParam2 + "\" does not exist");
        return false;
    }

    if (!checkRelation(poset.value(), value1, value2) || !canDeleteRelation(poset.value(), value1, value2)) {
        PRINT_DEBUG("poset_del: poset " + std::to_string(id) + ", relation (\"" + commandParam1 + "\", \""
                    + commandParam2 + "\") cannot be deleted");
        return false;
    }

    deleteRelation(poset.value(), value1, value2);
    PRINT_DEBUG("poset_del: poset " + std::to_string(id) + ", relation (\"" + commandParam1 + "\", \"" + commandParam2 +
                "\") deleted");
    return true;
}

void jnp1::poset_clear(unsigned long id) {
    PRINT_DEBUG("poset_clear(" + std::to_string(id) + ")");
    auto poset = getPoset(id);

    if (!poset.has_value()) {
        PRINT_DEBUG("poset_clear: poset " + std::to_string(id) + " does not exist");
        return;
    }

    clearPoset(poset.value());
    PRINT_DEBUG("poset_clear: poset " + std::to_string(id) + " cleared");
}