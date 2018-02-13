/*
 * Copyright 2017 Ophidian
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
 */

#ifndef OPHIDIAN_UTIL_KDTREE_H
#define OPHIDIAN_UTIL_KDTREE_H

#include <vector>
#include <ophidian/geometry/Models.h>
#include <algorithm>
#include <memory>

namespace ophidian {
namespace util {

template <typename Data>
class KDtree{
public:
    using Point = ophidian::geometry::Point;
    using Range = ophidian::geometry::Box;
    struct Node {
        std::shared_ptr<Node> left, right, parent;
        int axis;
        const Point split;
        const std::shared_ptr<Data> data;
        Range range;
        Node(const Point g, const std::shared_ptr<Data> & d) : axis(0), split(g), data(d){}
    };

    void reserve(const std::size_t size){
        mNodes.reserve(size);
    }

    void add(const Point point, const std::shared_ptr<Data> & data) {
        mNodes.push_back(std::make_shared<Node>(point, data));
    }

    void build(const Range range){
        mRoot = build(mNodes, 0, range);
    }

    void clear() {
        mRoot.reset();
        mNodes.clear();
    }

    const std::vector<std::shared_ptr<Data>> range_search(const Range & range) const{
        std::vector<std::shared_ptr<Data>> result;
        range_search(result, mRoot, range);
        return result;
    }

    KDtree() {}
protected:
    std::vector<std::shared_ptr<Node>> mNodes;
    std::shared_ptr<Node> mRoot;

    std::shared_ptr<Node> build(std::vector<std::shared_ptr<Node>> & nodes, int depth, Range range) {
        if (nodes.empty())
            return NULL;
        int axis = depth % boost::geometry::dimension<Point>();
        size_t median = nodes.size() / 2;
        if(axis == 0)
            std::nth_element(nodes.begin(), nodes.begin() + median, nodes.end(), [](std::shared_ptr<Node> n1, std::shared_ptr<Node> n2){return n1.get()->split.x() < n2.get()->split.x();});
        else
            std::nth_element(nodes.begin(), nodes.begin() + median, nodes.end(), [](std::shared_ptr<Node> n1, std::shared_ptr<Node> n2){return n1.get()->split.y() < n2.get()->split.y();});

        std::shared_ptr<Node> node = nodes.at(median);
        node->axis = axis;
        node->range = range;

        Range leftRange, rightRange;
        if(axis == 0){
            leftRange = Range(range.min_corner(), Point(node->split.x(), range.max_corner().y()));
            rightRange = Range(Point(node->split.x(), range.min_corner().y()), range.max_corner());
        }else{
            leftRange = Range(range.min_corner(), Point(range.max_corner().x(), node->split.y()));
            rightRange = Range(Point(range.min_corner().x(), node->split.y()), range.max_corner());
        }

        std::vector<std::shared_ptr<Node>> left(nodes.begin(), nodes.begin() + median);
        std::vector<std::shared_ptr<Node>> right(nodes.begin() + median + 1, nodes.end());
        node->left = build(left, depth + 1, leftRange);
        if(node->left.get() != NULL)
            node->left->parent = node;
        node->right = build(right, depth + 1, rightRange);
        if(node->right.get() != NULL)
            node->right->parent = node;
        return node;
    }

    void report_subtree(std::vector<std::shared_ptr<Data>> &result, const std::shared_ptr<Node> currentNode) const {
        if(currentNode->left.get() != NULL){
            result.push_back(currentNode->left->data);
            report_subtree(result, currentNode->left);
        }
        if(currentNode->right.get() != NULL){
            result.push_back(currentNode->right->data);
            report_subtree(result, currentNode->right);
        }
    }

    void range_search(std::vector<std::shared_ptr<Data>> &result, const std::shared_ptr<Node> currentNode, const Range &searchRange) const{
        if(boost::geometry::within(currentNode->split, searchRange))
            result.push_back(currentNode.get()->data);
        if(boost::geometry::within(currentNode->range, searchRange))
            report_subtree(result, currentNode);
        else{
            auto overlap = boost::geometry::intersects(currentNode->range, searchRange);
            if(overlap){
                if(currentNode->left.get() != NULL)
                    range_search(result, currentNode->left, searchRange);
                if(currentNode->right.get() != NULL)
                    range_search(result, currentNode->right, searchRange);
            }
        }
    }

};

} //namespace util

} //namespace ophidian

#endif // OPHIDIAN_UTIL_KDTREE_H
