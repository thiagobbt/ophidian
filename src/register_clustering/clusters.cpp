/*
 * Copyright 2016 Ophidian
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

#include "clusters.h"

namespace ophidian {
namespace register_clustering {
clusters::clusters(entity_system::entity_system &system)
    : m_system(system) {
    m_system.register_property(&m_flip_flops);
    m_system.register_property(&m_centers);
}

void clusters::insert_flip_flop(entity_system::entity cluster, cluster_element flip_flop)
{
    m_flip_flops[m_system.lookup(cluster)].push_back(flip_flop);
    m_flip_flop_to_cluster[flip_flop.first] = cluster;
}

void clusters::remove_flip_flop(entity_system::entity cluster, clusters::cluster_element flip_flop)
{
    auto & cluster_flip_flops = m_flip_flops[m_system.lookup(cluster)];
    auto flip_flop_it = std::find_if(cluster_flip_flops.begin(), cluster_flip_flops.end(), cluster_element_comparator(flip_flop));
    cluster_flip_flops.erase(flip_flop_it);
    m_flip_flop_to_cluster.erase(flip_flop.first);
}

void clusters::remove_flip_flops(entity_system::entity cluster)
{
    for (auto flip_flop : m_flip_flops[m_system.lookup(cluster)]) {
        m_flip_flop_to_cluster.erase(flip_flop.first);
    }
    m_flip_flops[m_system.lookup(cluster)].clear();
}

void clusters::center(entity_system::entity cluster, clusters::point center)
{
    m_centers[m_system.lookup(cluster)] = center;
}
}
}


