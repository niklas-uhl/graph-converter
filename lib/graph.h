//
// Created by Tim Niklas Uhl on 13.02.21.
//

#ifndef GRAPH_CONVERTER_GRAPH_H
#define GRAPH_CONVERTER_GRAPH_H

#include <utility>
#include <vector>
#include "types.h"

class ArrayGraph {
public:
    explicit ArrayGraph(): first_out_({0}), head_() {

    }
    explicit ArrayGraph(std::vector<EdgeId> first_out, std::vector<NodeId> head): first_out_(std::move(first_out)), head_(std::move(head)) {

    }

    explicit ArrayGraph(std::vector<std::vector<NodeId>> neighbors): first_out_(), head_() {
        EdgeId edge_count = 0;
        for (NodeId node = 0; node < neighbors.size(); node++) {
            first_out_.emplace_back(edge_count);
            for (NodeId head : neighbors[node]) {
                head_.emplace_back(head);
                edge_count++;
            }
        }
        first_out_.emplace_back(edge_count);
    }

    NodeId degree(NodeId node) const {
        return first_out_[node + 1] - first_out_[node];
    }

    NodeId node_count() const {
        return first_out_.size() - 1;
    }

    NodeId edge_count() const {
        return head_.size() / 2;
    }

    template<typename NodeFunc>
    void for_each_node(NodeFunc on_node) const {
        for (NodeId node = 0; node < node_count(); node++) {
            on_node(node);
        }
    }

    template<typename EdgeFunc>
    void for_each_edge(NodeId node, EdgeFunc on_edge) const {
        EdgeId begin = first_out_[node];
        EdgeId end = first_out_[node + 1];
        for (EdgeId edge_id = begin; edge_id < end; edge_id++) {
            on_edge(node, head_[edge_id]);
        }
    }
private:
    std::vector<EdgeId> first_out_;
    std::vector<NodeId> head_;
};

class ListGraph {
public:
    explicit ListGraph(): neighbors_() {

    }
    explicit ListGraph(std::vector<std::vector<NodeId>> neighbors): neighbors_(std::move(neighbors)) {

    }
    ListGraph(std::vector<EdgeId> first_out, std::vector<NodeId> head): neighbors_(first_out.size() - 1) {
        for (NodeId node = 0; node < first_out.size() - 1; node ++) {
            EdgeId begin = first_out[node];
            EdgeId end = first_out[node + 1];
            for (EdgeId edge_id = begin; edge_id < end; edge_id++) {
                neighbors_[node].emplace_back(head[edge_id]);
            }
        }
    }

    NodeId node_count() const {
        return neighbors_.size();
    }

    template<typename NodeFunc>
    void for_each_node(NodeFunc on_node) const {
        for (NodeId node = 0; node < neighbors_.size(); node++) {
            on_node(node);
        }
    }

    template<typename EdgeFunc>
    void for_each_edge(NodeId node, EdgeFunc on_edge) const {
        for (NodeId head : neighbors_[node]) {
            on_edge(Edge(node, head));
        }
    }

    void permutate(const std::vector<NodeId>& permutation) {
        std::vector<std::vector<NodeId>> new_neighbors(neighbors_.size());
        for_each_node([&](NodeId node) {
            new_neighbors[permutation[node]] = std::move(neighbors_[node]);
        });
        neighbors_ = std::move(new_neighbors);
        for (std::vector<NodeId>& neighborhood : neighbors_) {
            for (NodeId& node : neighborhood) {
                node = permutation[node];
            }
        }
    }

    void to_vectors(std::vector<EdgeId>& first_out, std::vector<NodeId>& head) {
        first_out.clear();
        first_out.reserve(node_count() + 1);
        head.clear();
        EdgeId edge_counter = 0;
        for (NodeId node = 0; node < node_count(); node++) {
            first_out.emplace_back(edge_counter);
            edge_counter += neighbors_[node].size();
            for (NodeId node : neighbors_[node]) {
                head.emplace_back(node);
            }
            neighbors_[node].clear();
        }
        first_out.emplace_back(edge_counter);
        neighbors_.clear();
    }

    std::vector<std::vector<NodeId>> to_neighbors() {
        return std::move(neighbors_);
    }
private:
    std::vector<std::vector<NodeId>> neighbors_;
};

#endif //GRAPH_CONVERTER_GRAPH_H
