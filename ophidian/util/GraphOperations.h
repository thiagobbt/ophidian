#ifndef GRAPHOPERATIONS_H
#define GRAPHOPERATIONS_H

#include <queue>

#include <lemon/list_graph.h>

namespace ophidian {
namespace util {

template<class GraphType>
void transitiveClosure(GraphType & graph) {
    unsigned numberOfNodes = lemon::countNodes(graph);

    std::vector<std::vector<bool> > previousRelation;
    previousRelation.resize(numberOfNodes);
    for (auto node1 = typename GraphType::NodeIt(graph); node1 != lemon::INVALID; ++node1) {
        int node1Id = graph.id(node1);
        previousRelation[node1Id].resize(numberOfNodes);
        for (auto node2 = typename GraphType::NodeIt(graph); node2 != lemon::INVALID; ++node2) {
            int node2Id = graph.id(node2);
            previousRelation[node1Id][node2Id] = lemon::findArc(graph, node1, node2) != lemon::INVALID;
        }
    }

    for (unsigned k = 0; k < numberOfNodes; k++) {
        std::vector<std::vector<bool> > newRelation;
        newRelation.resize(numberOfNodes);
        for (unsigned i = 0; i < numberOfNodes; i++) {
            newRelation[i].resize(numberOfNodes);
            for (unsigned j = 0; j < numberOfNodes; j++) {
                newRelation[i][j] = previousRelation[i][j] || (previousRelation[i][k] && previousRelation[k][j]);
            }
        }
        previousRelation = newRelation;
    }

    for (auto node1 = typename GraphType::NodeIt(graph); node1 != lemon::INVALID; ++node1) {
        int node1Id = graph.id(node1);
        for (auto node2 = typename GraphType::NodeIt(graph); node2 != lemon::INVALID; ++node2) {
            int node2Id = graph.id(node2);
            if (previousRelation[node1Id][node2Id] && lemon::findArc(graph, node1, node2) == lemon::INVALID) {
                graph.addArc(node1, node2);
            }
        }
    }
}

template<class GraphType>
void transitiveReduction(GraphType & graph) {
    unsigned numberOfNodes = lemon::countNodes(graph);

    std::vector<std::vector<bool> > adjacencyMatrix;
    adjacencyMatrix.resize(numberOfNodes);
    for (auto node1 = typename GraphType::NodeIt(graph); node1 != lemon::INVALID; ++node1) {
        int node1Id = graph.id(node1);
        adjacencyMatrix[node1Id].resize(numberOfNodes);
        for (auto node2 = typename GraphType::NodeIt(graph); node2 != lemon::INVALID; ++node2) {
            int node2Id = graph.id(node2);
            adjacencyMatrix[node1Id][node2Id] = lemon::findArc(graph, node1, node2) != lemon::INVALID;
        }
    }

    for (auto node1 = typename GraphType::NodeIt(graph); node1 != lemon::INVALID; ++node1) {
        int node1Id = graph.id(node1);
        adjacencyMatrix[node1Id][node1Id] = false;
    }

    for (auto node1 = typename GraphType::NodeIt(graph); node1 != lemon::INVALID; ++node1) {
        int node1Id = graph.id(node1);
        for (auto node2 = typename GraphType::NodeIt(graph); node2 != lemon::INVALID; ++node2) {
            int node2Id = graph.id(node2);
            if (adjacencyMatrix[node1Id][node2Id]) {
                for (auto node3 = typename GraphType::NodeIt(graph); node3 != lemon::INVALID; ++node3) {
                    int node3Id = graph.id(node3);
                    if (adjacencyMatrix[node2Id][node3Id]) {
                        adjacencyMatrix[node1Id][node3Id] = false;
                    }
                }
            }
        }
    }

    for (auto node1 = typename GraphType::NodeIt(graph); node1 != lemon::INVALID; ++node1) {
        int node1Id = graph.id(node1);
        for (auto node2 = typename GraphType::NodeIt(graph); node2 != lemon::INVALID; ++node2) {
            int node2Id = graph.id(node2);
            if (!adjacencyMatrix[node1Id][node2Id]) {
                auto arc = lemon::findArc(graph, node1, node2);
                if (arc != lemon::INVALID) {
                    graph.erase(arc);
                }
            }
        }
    }
}

template<class GraphType>
void topologicalSort(const GraphType & graph, typename GraphType::Node source, std::vector<typename GraphType::Node> & sortedNodes) {
    typename GraphType::template NodeMap<int> nodeCounter(graph);
    typename GraphType::template NodeMap<bool> visitedNodes(graph);

    sortedNodes.reserve(lemon::countNodes(graph));

    std::queue<typename GraphType::Node> nodesToVisit;
    nodesToVisit.push(source);
    while (!nodesToVisit.empty()) {
        auto currentNode = nodesToVisit.front();
        nodesToVisit.pop();
        visitedNodes[currentNode] = true;
        sortedNodes.push_back(currentNode);

        for (auto arc = typename GraphType::OutArcIt(graph, currentNode); arc != lemon::INVALID; ++arc) {
            auto target = graph.target(arc);
            nodeCounter[target]++;
            if (nodeCounter[target] == lemon::countInArcs(graph, target)) {
                nodesToVisit.push(target);
            }
        }
    }
}

}
}


#endif // GRAPHOPERATIONS_H
