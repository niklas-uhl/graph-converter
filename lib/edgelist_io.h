//
// Created by Tim Niklas Uhl on 21.01.21.
//

#ifndef GRAPH_CONVERTER_EDGELIST_IO_H
#define GRAPH_CONVERTER_EDGELIST_IO_H

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "types.h"

template<typename EdgeFunc>
void parse_edge_list(const std::string& input, EdgeFunc on_edge, std::string comment_prefix = "#", std::size_t lines_to_skip = 0) {
    std::ifstream stream(input);
    if (stream.fail()) {
        throw std::runtime_error("Could not open input file for reading.");
    }
    std::string line;
    std::size_t line_num = 0;
    while (std::getline(stream, line)) {
        if (line.rfind(comment_prefix, 0) == 0) {
            continue;
        }
        if (line_num < lines_to_skip) {
            line_num++;
            continue;
        }
        std::stringstream sstream(line);
        NodeId tail;
        sstream >> tail;
        NodeId head;
        sstream >> head;
        on_edge(Edge(tail, head));
        line_num++;
    }
}

#endif //GRAPH_CONVERTER_EDGELIST_IO_H
