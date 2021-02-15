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
#include "search.h"

int main(int argc, char* argv[]) {
    CLI::App app;

    std::string input;
    app.add_option("input", input, "The input graph")
            ->required();

    std::string input_format;
    app.add_set("--input-format", input_format, {"binary", "metis"})
            ->required();
    std::string output_format;
    app.add_set("--output-format", output_format, {"binary", "metis"})
            ->required();

    std::string order_type;
    app.add_set("--order", order_type, {"bfs", "dfs"})
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

    std::transform(order_type.begin(), order_type.end(), std::ostream_iterator<char>(std::cout, ""), ::toupper);
    std::cout << " ..." << std::flush;

    std::vector<NodeId> order(G.node_count());
    NodeId index = 0;
    auto run_search = [&](auto& search) {
        search.run([&](NodeId root) {
            order[root] = index;
            index++;
        }, [&](Edge edge) {
            order[edge.head] = index;
            index++;
        }, [](Edge){});
    };
    if (order_type == "bfs") {
        BFS bfs(G);
        run_search(bfs);
    } else if (order_type == "dfs") {
        DFS dfs(G);
        run_search(dfs);
    } else {
        throw std::runtime_error("This should not happen");
    }
    std::cout << " finished." << std::endl;

    std::vector<NodeId> order_copy = order;
    std::sort(order_copy.begin(), order_copy.end());
    for (size_t i = 0; i < order_copy.size(); ++i) {
        if (order_copy[i] != i) {
            throw std::runtime_error("HELP!");
        }
    }

    G.permutate(order);

    if (output_format == "binary") {
        auto basename = input_path.stem();
        auto path = input_path.parent_path();
        auto first_out_path = path / (basename.string() + ".first_out");
        auto head_path = path / (basename.string() + ".head");
        std::vector<EdgeId> first_out;
        std::vector<NodeId> head;
        G.to_vectors(first_out, head);

        first_out_path = path / (basename.string() + "-" + order_type + ".first_out");
        head_path = path / (basename.string() + "-" + order_type + ".head");
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
        auto metis_path = path / (basename.string() + "-" + order_type + ".metis");
        write_metis(metis_path, edge_count, G.to_neighbors());
    } else {
        throw std::runtime_error("This should not happen");
    }
    return 0;
}
