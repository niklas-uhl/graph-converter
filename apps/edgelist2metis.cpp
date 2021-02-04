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
    std::cout << "Parsed edges: " << std::flush;
    parse_edge_list(input_path, [&](Edge edge) {
        if (edge_count % 1'000'000 == 0) {
            std::cout << edge_count / 1'000'000 << "M, " << std::flush;
        }
        max_id = std::max({max_id, edge.tail, edge.head});
        if (edge.tail == edge.head) {
            std::cout << "self loop, " << std::flush;
            edge_count++;
            return;
        }
        if (edge.head < edge.tail) {
            std::swap(edge.tail, edge.head);
        }
        edges.emplace_back(edge.tail, edge.head);
        edge_count++;
    });
    std::cout << std::endl;
    std::cout << std::endl << "Sorting edges ..." << std::flush;
    ips4o::parallel::sort(edges.begin(), edges.end(), [&](const Edge& e1, const Edge& e2) {
        return std::tie(e1.tail, e1.head) < std::tie(e2.tail, e2.head);
    });
    std::cout << " finished." << std::endl;

    std::cout << "Removing duplicates ..." << std::flush;
    auto last = std::unique(edges.begin(), edges.end());
    edges.erase(last, edges.end());
    std::cout << " finished." << std::endl;

    NodeId node_count = max_id + 1;
    edge_count = edges.size();
    std::cout << "n:" << std::setw(20) << node_count<< std::endl;
    std::cout << "m:" << std::setw(20) << edge_count<< std::endl;

    std::cout << "Edge list to adjacency list ..." << std::flush;
    std::vector<std::vector<NodeId>> neighbors(node_count);
    for (Edge e : edges) {
        neighbors[e.head].emplace_back(e.tail);
        neighbors[e.tail].emplace_back(e.head);
    }
    edges.clear();
    std::cout << " finished." << std::endl;

    std::cout << "Remove dangling nodes ..." << std::flush;
    std::vector<NodeId> node_mapping(neighbors.size());
    NodeId compressed_node_id = 0;
    for (NodeId node_id = 0; node_id < neighbors.size(); node_id++) {
        NodeId degree = neighbors[node_id].size();
        if (degree > 0) {
            node_mapping[node_id] = compressed_node_id;
            neighbors[compressed_node_id] = std::move(neighbors[node_id]);
            compressed_node_id++;
        }
    }

    neighbors.erase(neighbors.begin() + compressed_node_id, neighbors.end());
    std::cout << " finished." << std::endl;

    std::cout << "n:" << std::setw(20) <<  neighbors.size() << std::endl;

    tbb::parallel_for(tbb::blocked_range<size_t>(0, neighbors.size()), [&](tbb::blocked_range<size_t> r) {
        for (NodeId node = r.begin(); node != r.end(); node++) {
            tbb::parallel_for(tbb::blocked_range<size_t>(0, neighbors[node].size()), [&](tbb::blocked_range<size_t> r) {
                for (size_t i = r.begin(); i != r.end(); ++i) {
                    neighbors[node][i] = node_mapping[neighbors[node][i]];
                }
            });
        }
    });

    auto basename = input_path.stem();
    auto path = input_path.parent_path();
    auto metis_path = path / (basename.string() + ".metis");
    write_metis(metis_path, edge_count, neighbors);
    return 0;
}