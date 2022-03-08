#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <algorithm>

#include <CLI/CLI.hpp>
#include "types.h"
#include "metis_io.h"
#include "vector_io.h"
#include "graph.h"
#include "partition_io.h"

int main(int argc, char* argv[]) {
    CLI::App app;

    std::string input;
    app.add_option("input", input, "The input graph")
            ->required();

    std::string partition_file;
    app.add_option("partition", partition_file, "The input partition")
            ->required();

    std::string input_format;
    app.add_set("--input-format", input_format, {"binary", "metis"})
            ->required();
    std::string output_format;
    app.add_set("--output-format", output_format, {"binary", "metis"})
            ->required();

    CLI11_PARSE(app, argc, argv);

    auto input_path = std::filesystem::path(input);

    std::cout << "Reading input ..." << std::flush;
    ListGraph G;

    EdgeId edge_count = 0;
    if (input_format == "metis") {
        std::vector<std::vector<NodeId>> neighbors;
        auto on_header = [&](NodeId node_count_, EdgeId edge_count_) {
            edge_count = edge_count_;
            neighbors.resize(node_count_);
        };
        auto on_node = [&](NodeId) {

        };
        auto on_edge = [&](Edge edge) {
            neighbors[edge.tail].emplace_back(edge.head);
        };
        read_metis(input_path, on_header, on_node, on_edge);
        G = ListGraph(std::move(neighbors));
    } else if (input_format == "binary") {
        auto basename = input_path.stem();
        auto path = input_path.parent_path();
        auto first_out_path = path / (basename.string() + ".first_out");
        auto head_path = path / (basename.string() + ".head");
        std::vector<EdgeId> first_out = load_vector<EdgeId>(first_out_path);
        std::vector<NodeId> head = load_vector<NodeId>(head_path);
        edge_count = head.size() / 2;

        G = ListGraph(std::move(first_out), std::move(head));
    }
    std::cout << " finished." << std::endl;

    std::cout << "Reading partition ..." << std::flush;
    std::vector<NodeId> partition = read_partition(partition_file);
    std::cout << " finished." << std::endl;

    std::cout << "Building order ..." << std::flush;
    NodeId max_partition = *std::max_element(partition.begin(), partition.end());
    std::vector<NodeId> partition_size(max_partition + 2);
    for (NodeId part : partition) {
        partition_size[part]++;
    }
    std::exclusive_scan(partition_size.begin(), partition_size.end() - 1, partition_size.begin(), 0);
    partition_size[max_partition + 1] = partition.size();
    std::vector<NodeId> order(partition.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](NodeId x, NodeId y) {
        return partition[x] < partition[y];
    });
    std::cout << " finished." << std::endl;

    std::cout << "Permutation ..." << std::flush;
    G.permutate(order);
    std::cout << " finished." << std::endl;

    auto basename = std::filesystem::path(partition_file).stem();
    basename = basename.stem();
    auto partitioner = basename.extension().string();
    partitioner.erase(0, 1);
    auto suffix = partitioner + "-" + std::to_string(max_partition + 1);
    if (output_format == "binary") {
        auto basename = input_path.stem();
        auto path = input_path.parent_path();
        auto first_out_path = path / (basename.string() + ".first_out");
        auto head_path = path / (basename.string() + ".head");
        std::vector<EdgeId> first_out;
        std::vector<NodeId> head;
        G.to_vectors(first_out, head);

        first_out_path = path / (basename.string() + "-" + suffix + ".first_out");
        head_path = path / (basename.string() + "-" + suffix + ".head");
        std::cout << "Writing first_out ..." << std::flush;
        try {
            save_vector(first_out_path, first_out);
        } catch (std::runtime_error &e) {
            std::cerr << "[ERROR] " << e.what() << std::endl;
            return 1;
        }
        std::cout << " finished." << std::endl;

        std::cout << "Writing head ..." << std::flush;
        try {
            save_vector(head_path, head);
        } catch (std::runtime_error &e) {
            std::cerr << "[ERROR] " << e.what() << std::endl;
            return 1;
        }
        std::cout << " finished." << std::endl;
    } else if (output_format == "metis") {
        auto basename = input_path.stem();
        auto path = input_path.parent_path();
        auto metis_path = path / (basename.string() + "-" + suffix + ".metis");
        write_metis(metis_path, G.to_neighbors());
    } else {
        throw std::runtime_error("This should not happen");
    }

    basename = input_path.stem();
    auto path = input_path.parent_path();
    auto ranges_path = path / (basename.string() + "-" + suffix + ".ranges");
    std::ofstream out(ranges_path);
    if (out.fail()) {
        throw std::runtime_error("Could not open output file for reading.");
    }
    for (size_t part = 0; part < partition_size.size() - 1; part++) {
        out << part << " " << partition_size[part] << " " << partition_size[part + 1] << std::endl;
    }

    return 0;
}
