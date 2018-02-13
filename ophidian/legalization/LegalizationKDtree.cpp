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

const std::vector<std::pair<std::vector<std::shared_ptr<Data>>, LegalizationKDtree::Range>> LegalizationKDtree::subTrees(unsigned int k) const{
    std::vector<std::pair<std::vector<std::shared_ptr<Data>>, Range>> subtrees;
    subTrees(subtrees, mRoot, k);
    return subtrees;
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

void LegalizationKDtree::subTrees(std::vector<std::pair<std::vector<std::shared_ptr<Data>>, Range>> & result, const std::shared_ptr<Node> currentNode, unsigned int k) const{
    if(k > 0){
        --k;
        if(currentNode->left.get() != NULL)
            subTrees(result, currentNode->left, k);
        if(currentNode->right.get() != NULL)
            subTrees(result, currentNode->right, k);
    }else{
        std::vector<std::shared_ptr<Data>> subtree;
        subtree.push_back(currentNode->data);
        report_subtree(subtree, currentNode);
        result.push_back(std::make_pair(subtree, currentNode->range));
    }
}

} // namespace legalization
} // namespace ophidian
