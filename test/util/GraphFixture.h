#ifndef GRAPHFIXTURE_H
#define GRAPHFIXTURE_H

#include <lemon/list_graph.h>

class GraphFixture
{
public:
    GraphFixture();

    lemon::ListDigraph mGraph;

    lemon::ListDigraph::Node mNode1;
    lemon::ListDigraph::Node mNode2;
    lemon::ListDigraph::Node mNode3;
    lemon::ListDigraph::Node mNode4;
};

class TransitiveGraphFixture
{
public:
    TransitiveGraphFixture();

    lemon::ListDigraph mGraph;

    lemon::ListDigraph::Node mNode1;
    lemon::ListDigraph::Node mNode2;
    lemon::ListDigraph::Node mNode3;
    lemon::ListDigraph::Node mNode4;
};

class DAGFixture
{
public:
    DAGFixture();

    lemon::ListDigraph mGraph;

    lemon::ListDigraph::Node mNode1;
    lemon::ListDigraph::Node mNode2;
    lemon::ListDigraph::Node mNode3;
    lemon::ListDigraph::Node mNode4;
};

#endif // GRAPHFIXTURE_H
