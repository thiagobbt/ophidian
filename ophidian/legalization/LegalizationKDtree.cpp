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
        std::vector<std::shared_ptr<Data>> elements;
        elements.push_back(currentNode->data);
        report_subtree(elements, currentNode);
        Partition partition;
        partition.elements = elements;
        partition.range = currentNode->range;
        result.push_back(partition);
    }
}

} // namespace legalization
} // namespace ophidian
