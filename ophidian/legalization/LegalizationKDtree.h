#ifndef LEGALIZATIONKDTREE_H
#define LEGALIZATIONKDTREE_H

#include <ophidian/util/KDtree.h>
#include <ophidian/circuit/Netlist.h>

namespace ophidian
{
namespace legalization
{
using Data = ophidian::circuit::Cell;
class LegalizationKDtree : public util::KDtree<Data>{
public:
    struct Partition{
        bool legalized = false;
        std::shared_ptr<Node> root;
        std::vector<std::shared_ptr<Data>> elements;
        Range range;
    };

    LegalizationKDtree();

    const std::vector<std::shared_ptr<Data>> ancientNodes(unsigned int k) const;
    const std::vector<Partition> partitions(unsigned int k) const;
    const std::vector<Partition> parentPartitions(std::vector<std::shared_ptr<Node>> nodes) const;
private:
    void ancientNodes(std::vector<std::shared_ptr<Data>> & result, const std::shared_ptr<Node> currentNode, unsigned int k) const;
    void partitions(std::vector<Partition> & result, const std::shared_ptr<Node> currentNode, unsigned int k) const;
    const Partition createPartition(const std::shared_ptr<Node> node) const;

};

} // namespace legalization
} // namespace ophidian

#endif // LEGALIZATIONKDTREE_H
