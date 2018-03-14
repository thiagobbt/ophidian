#include "LegalizationKDtree.h"

namespace ophidian
{
namespace legalization
{

LegalizationKDtree::LegalizationKDtree(){

}

const std::vector<std::shared_ptr<Data>> LegalizationKDtree::ancientNodes(unsigned int k) const {
    std::vector<std::shared_ptr<Data>> ancients;
    ancientNodes(ancients, mRoot, k);
    return ancients;
}

const std::vector<LegalizationKDtree::Partition> LegalizationKDtree::partitions(unsigned int k) const{
    std::vector<Partition> result;
    partitions(result, mRoot, k);
    return result;
}

const std::vector<LegalizationKDtree::Partition> LegalizationKDtree::parentPartitions(const std::vector<std::shared_ptr<Partition>> & partitions) const{
    std::vector<Partition> result;
    std::vector<std::shared_ptr<Node>> visitedParents;
    for(auto & partition : partitions)
    {
        auto partitionParent = partition->root->parent;
        bool visited = false;
        for(auto visitedNode : visitedParents)
            if(boost::geometry::equals(visitedNode->range, partitionParent->range)){
                visited = true;
                break;
            }
        if(visited == false)
        {
            visitedParents.push_back(partitionParent);
            Partition partition = createPartition(partitionParent);
            result.push_back(partition);
        }
    }
    return result;
}

void LegalizationKDtree::ancientNodes(std::vector<std::shared_ptr<Data>> & result, const std::shared_ptr<Node> currentNode, unsigned int k) const{
    if(k > 0){
        --k;
        result.push_back(currentNode->data);
        if(currentNode->left.get() != NULL)
            ancientNodes(result, currentNode->left, k);
        if(currentNode->right.get() != NULL)
            ancientNodes(result, currentNode->right, k);
    }
}

void LegalizationKDtree::partitions(std::vector<Partition> & result, const std::shared_ptr<Node> currentNode, unsigned int k) const{
    if(k > 0){
        --k;
        if(currentNode->left.get() != NULL)
            partitions(result, currentNode->left, k);
        if(currentNode->right.get() != NULL)
            partitions(result, currentNode->right, k);
    }else{
        Partition partition = createPartition(currentNode);
        result.push_back(partition);
    }
}

const LegalizationKDtree::Partition LegalizationKDtree::createPartition(const std::shared_ptr<Node> node) const{
    std::vector<std::shared_ptr<Data>> elements;
    elements.push_back(node->data);
    report_subtree(elements, node);
    Partition partition;
    partition.elements = elements;
    partition.range = node->range;
    partition.root = node;
    return partition;
}

} // namespace legalization
} // namespace ophidian