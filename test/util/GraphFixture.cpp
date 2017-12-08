#include "GraphFixture.h"

GraphFixture::GraphFixture()
{
    mNode1 = mGraph.addNode();
    mNode2 = mGraph.addNode();
    mNode3 = mGraph.addNode();
    mNode4 = mGraph.addNode();

    mGraph.addArc(mNode1, mNode2);
    mGraph.addArc(mNode2, mNode4);
    mGraph.addArc(mNode4, mNode1);
    mGraph.addArc(mNode4, mNode3);
}

TransitiveGraphFixture::TransitiveGraphFixture()
{
    mNode1 = mGraph.addNode();
    mNode2 = mGraph.addNode();
    mNode3 = mGraph.addNode();
    mNode4 = mGraph.addNode();

    mGraph.addArc(mNode1, mNode1);
    mGraph.addArc(mNode1, mNode2);
    mGraph.addArc(mNode1, mNode3);
    mGraph.addArc(mNode1, mNode4);

    mGraph.addArc(mNode2, mNode1);
    mGraph.addArc(mNode2, mNode2);
    mGraph.addArc(mNode2, mNode3);
    mGraph.addArc(mNode2, mNode4);

    mGraph.addArc(mNode4, mNode1);
    mGraph.addArc(mNode4, mNode2);
    mGraph.addArc(mNode4, mNode3);
    mGraph.addArc(mNode4, mNode4);
}

DAGFixture::DAGFixture()
{
    mNode1 = mGraph.addNode();
    mNode2 = mGraph.addNode();
    mNode3 = mGraph.addNode();
    mNode4 = mGraph.addNode();

    mGraph.addArc(mNode1, mNode2);
    mGraph.addArc(mNode1, mNode3);
    mGraph.addArc(mNode2, mNode4);
    mGraph.addArc(mNode3, mNode4);
}
