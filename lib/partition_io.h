//
// Created by Tim Niklas Uhl on 06.03.21.
//

#ifndef GRAPH_CONVERTER_PARTITION_IO_H
#define GRAPH_CONVERTER_PARTITION_IO_H

#include <sstream>
#include <fstream>
#include <vector>
#include "types.h"

std::vector<NodeId> read_partition(const std::string& input) {
    std::ifstream stream(input);
    if (stream.fail()) {
        throw std::runtime_error("Could not open input file for reading.");
    }
    std::string line;
    std::vector<NodeId> partition;
    while (std::getline(stream, line)) {
        std::istringstream sstream = std::istringstream(line);
        NodeId part_id;
        sstream >> part_id;
        partition.emplace_back(part_id);
    }
    return partition;
}

#endif //GRAPH_CONVERTER_PARTITION_IO_H
