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
    LegalizationKDtree();

    const std::vector<std::shared_ptr<Data>> ancientNodes(unsigned int k) const;
    const std::vector<std::pair<std::vector<std::shared_ptr<Data>>, Range>> subTrees(unsigned int k) const;

private:
    void ancientNodes(std::vector<std::shared_ptr<Data>> & result, const std::shared_ptr<Node> currentNode, unsigned int k) const;
    void subTrees(std::vector<std::pair<std::vector<std::shared_ptr<Data>>, Range>> & result, const std::shared_ptr<Node> currentNode, unsigned int k) const;

};

} // namespace legalization
} // namespace ophidian

#endif // LEGALIZATIONKDTREE_H
