//
// Created by Tim Niklas Uhl on 13.02.21.
//

#ifndef GRAPH_CONVERTER_SEARCH_H
#define GRAPH_CONVERTER_SEARCH_H

#include <vector>
#include <queue>
#include <stack>
#include "types.h"
#include "graph.h"

template<class Graph>
class BFS {
public:
    BFS(const Graph& G): G(G), visited_(G.node_count()), queue_() {

    }

    template<typename NodeFunc, typename TreeEdgeFunc, typename NonTreeEdgeFunc>
    void run(NodeFunc on_root, TreeEdgeFunc on_tree_edge, NonTreeEdgeFunc on_non_tree_edge) {
        G.for_each_node([&](NodeId root) {
            if (visited_[root]) {
                return;
            }
            on_root(root);
            queue_.push(root);
            visited_[root] = true;
            while (!queue_.empty()) {
                NodeId node = queue_.front();
                queue_.pop();
                G.for_each_edge(node, [&](Edge edge) {
                    if (visited_[edge.head]) {
                        on_non_tree_edge(edge);
                    } else {
                        queue_.push(edge.head);
                        visited_[edge.head] = true;
                        on_tree_edge(edge);
                    }
                });
            }
        });
    }

private:
    const Graph& G;
    std::vector<bool> visited_;
    std::queue<NodeId> queue_;
};

template<class Graph>
class DFS {
public:
    DFS(const Graph& G): G(G), visited_(G.node_count()), stack_() {

    }

    template<typename NodeFunc, typename TreeEdgeFunc, typename NonTreeEdgeFunc>
    void run(NodeFunc on_root, TreeEdgeFunc on_tree_edge, NonTreeEdgeFunc on_non_tree_edge) {
        G.for_each_node([&](NodeId root) {
            if (visited_[root]) {
                return;
            }
            on_root(root);
            stack_.push(root);
            visited_[root] = true;
            while (!stack_.empty()) {
                NodeId node = stack_.top();
                stack_.pop();
                G.for_each_edge(node, [&](Edge edge) {
                    if (visited_[edge.head]) {
                        on_non_tree_edge(edge);
                    } else {
                        stack_.push(edge.head);
                        visited_[edge.head] = true;
                        on_tree_edge(edge);
                    }
                });
            }
        });
    }

private:
    const Graph& G;
    std::vector<bool> visited_;
    std::stack<NodeId> stack_;
};
#endif //GRAPH_CONVERTER_SEARCH_H
