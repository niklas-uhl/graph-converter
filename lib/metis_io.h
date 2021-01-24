//
// Created by Tim Niklas Uhl on 21.01.21.
//

#ifndef GRAPH_CONVERTER_METIS_IO_H
#define GRAPH_CONVERTER_METIS_IO_H

#include <sstream>
#include <fstream>
#include <vector>
#include "types.h"

template<typename HeaderFunc, typename NodeFunc, typename EdgeFunc>
void read_metis(const std::string& input, HeaderFunc on_header, NodeFunc on_node, EdgeFunc on_edge) {
    std::ifstream stream(input);
    if (stream.fail()) {
        throw std::runtime_error("Could not open input file for reading.");
    }
    std::string line;
    std::getline(stream, line);
    std::istringstream sstream(line);
    NodeId node_count;
    EdgeId edge_count;
    sstream >> node_count >> edge_count;
    if (sstream.bad()) {
        throw std::runtime_error("Failed to parse header.");
    }
    on_header(node_count, edge_count);


    size_t line_number = 1;
    NodeId node = 0;
    EdgeId edge_id = 0;
    while (node < node_count && std::getline(stream, line)) {
        sstream = std::istringstream(line);
        // skip comment lines
        if (line.rfind('%', 0) == 0) {
            line_number++;
            continue;
        }
        on_node(node);
        NodeId head_node;
        while (sstream >> head_node) {
            if (head_node >= node_count + 1) {
                throw std::runtime_error("Invalid node id " + std::to_string(head_node) + " in line " + std::to_string(line_number) + ".");
            }
            on_edge(Edge(node, head_node - 1));
            edge_id++;
        }
        if (sstream.bad()) {
            throw std::runtime_error("Invalid input in line " + std::to_string(line_number) + ".");
        }
        node++;
        line_number++;
    }
    if (node != node_count) {
        throw std::runtime_error("Number of nodes does not match header.");
    }
    if (edge_id != edge_count * 2) {
        std::stringstream out;
        out << "Number of edges does not mach header (header: " << edge_count << ", actual: " << edge_id << ")";
        throw std::runtime_error(out.str());
    }
}

void read_metis_header(const std::string& input, NodeId& node_count, EdgeId& edge_count) {
    std::ifstream stream(input);
    if (stream.fail()) {
        throw std::runtime_error("Could not open input file for reading.");
    }
    std::string line;
    std::getline(stream, line);
    std::istringstream sstream(line);
    sstream >> node_count >> edge_count;
    if (sstream.bad()) {
        throw std::runtime_error("Failed to parse header.");
    }
}

void read_metis(const std::string& input, std::vector<EdgeId>& first_out, std::vector<NodeId>& head) {
    NodeId node_count;
    EdgeId edge_count;
    first_out.clear();
    head.clear();
    EdgeId edge_id = 0;
    auto on_header = [&](NodeId node_count_, EdgeId edge_count_) {
        node_count = node_count_;
        edge_count = edge_count_ * 2;
        first_out.resize(node_count + 1);
        head.resize(edge_count);
    };
    auto on_node = [&](NodeId node) {
        first_out[node] = edge_id;
    };
    auto on_edge = [&](Edge edge) {
        head[edge_id] = edge.head;
        edge_id++;
    };
    read_metis(input, on_header, on_node, on_edge);
    first_out[node_count] = edge_id;
}

void write_metis(const std::string& output, const std::vector<EdgeId>& first_out, const std::vector<NodeId>& head) {
    std::ofstream out(output);
    if (out.fail()) {
        throw std::runtime_error("Could not open output file for reading.");
    }

    std::cout << "Writing: " << std::flush;
    out << first_out.size() - 1 << " " << head.size() / 2 << " " << 0 << std::endl;
    for (NodeId node = 0; node < first_out.size() - 1; node++) {
        auto begin = first_out[node];
        auto end = first_out[node + 1];
        for (EdgeId edge_id = begin; edge_id < end; edge_id++) {
            if (edge_id % 1'000'000 == 0) {
                std::cout << edge_id / 1'000'000 << "M, " << std::flush;
            }
            if (edge_id != begin) {
                out << " ";
            }
            out << head[edge_id] + 1;
        }
        out << std::endl;
    }
    std::cout << std::endl;
    std::cout << "Finished writing" << std::endl;
}


#endif //GRAPH_CONVERTER_METIS_IO_H
