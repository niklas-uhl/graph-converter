//
// Created by Tim Niklas Uhl on 21.01.21.
//

#ifndef GRAPH_CONVERTER_TYPES_H
#define GRAPH_CONVERTER_TYPES_H

#include <cstdint>

using NodeId = uint64_t;
using EdgeId = uint64_t;

struct Edge {
    NodeId tail;
    NodeId head;

    Edge(NodeId tail, NodeId head): tail(tail), head(head) {
    }
};

inline bool operator==(const Edge& e1, const Edge& e2) {
    return e1.tail == e2.tail && e1.head == e2.head;
}

#endif //GRAPH_CONVERTER_TYPES_H
