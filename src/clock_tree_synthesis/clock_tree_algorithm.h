#ifndef CLOCK_TREE_ALGORITHM_H
#define CLOCK_TREE_ALGORITHM_H

#include "clock_tree_definition.h"
#include "topology.h"
#include "embedding.h"

namespace ophidian {
namespace clock_tree_synthesis {

class clock_tree_algorithm{
public:
    clock_tree_algorithm(){

    }

    virtual ~clock_tree_algorithm(){

    }

    virtual void build(const topology & topology, embedding & embedding) = 0;
};
}
}
#endif // CLOCK_TREE_ALGORITHM_H
