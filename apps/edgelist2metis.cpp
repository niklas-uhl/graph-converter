#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <thread>

#include <ips4o.hpp>
#include <tbb/parallel_for.h>

#include "types.h"
#include "metis_io.h"
#include "vector_io.h"
#include "edgelist_io.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << "[--sort] INPUT" << std::endl;
        return 1;
    }

    auto input_path = std::filesystem::path(argv[argc - 1]);

    std::vector<Edge> edges;
    NodeId max_id = 0;
    size_t edge_count = 0;
    std::cout << "Parsed edges: ";
    parse_edge_list(input_path, [&](Edge edge) {
        if (edge_count % 1'000'000 == 0) {
            std::cout << edge_count / 1'000'000 << "M, ";
        }
        max_id = std::max({max_id, edge.tail, edge.head});
        edges.emplace_back(edge.tail, edge.head);
        edges.emplace_back(edge.head, edge.tail);
        edge_count++;
    });
    std::cout << std::endl;
    std::cout << std::endl << "Sorting edges ...";
    ips4o::parallel::sort(edges.begin(), edges.end(), [&](const Edge& e1, const Edge& e2) {
        return std::tie(e1.tail, e1.head) < std::tie(e2.tail, e2.head);
    });
    std::cout << " finished." << std::endl;

    std::cout << "Removing duplicates ...";
    auto last = std::unique(edges.begin(), edges.end());
    edges.erase(last, edges.end());
    std::cout << " finished." << std::endl;
    assert(edges.size() % 2 == 0);

    std::cout << "n:" << std::setw(20) << max_id + 1 << std::endl;
    std::cout << "m:" << std::setw(20) << edges.size() / 2 << std::endl;

    std::vector<EdgeId> first_out;
    first_out.reserve(max_id + 1);
    std::vector<NodeId> head;
    head.reserve(edges.size());

    NodeId current_tail = 0;
    first_out.push_back(0);
    for (auto& edge : edges) {
        while (edge.tail != current_tail) {
            first_out.push_back(head.size());
            current_tail++;
        }
        head.emplace_back(edge.head);
    }
    while (current_tail != max_id + 1) {
        first_out.push_back(head.size());
        ++current_tail;
    }
    assert(first_out.size() == max_id + 2);
    edges.clear();

    std::cout << "Remove dangling nodes ...";
    std::vector<NodeId> node_mapping(first_out.size() - 1);
    NodeId compressed_node_id = 0;
    for (NodeId node_id = 0; node_id < first_out.size() - 1; node_id++) {
        NodeId degree = first_out[node_id + 1] - first_out[node_id];
        if (degree > 0) {
            node_mapping[node_id] = compressed_node_id;
            first_out[compressed_node_id] = first_out[node_id];
            compressed_node_id++;
        }
    }
    first_out[compressed_node_id] = head.size();
    first_out.erase(first_out.begin() + compressed_node_id + 1, first_out.end());
    std::cout << " finished." << std::endl;

    std::cout << "n:" << std::setw(20) <<  first_out.size() - 1 << std::endl;

    tbb::parallel_for(tbb::blocked_range<size_t>(0, head.size()), [&](tbb::blocked_range<size_t> r) {
        for(size_t i = r.begin(); i != r.end(); ++i) {
            head[i] = node_mapping[head[i]];
        }
    });

    auto basename = input_path.stem();
    auto path = input_path.parent_path();
    auto metis_path = path / (basename.string() + ".metis");
    write_metis(metis_path, first_out, head);
    return 0;
}
