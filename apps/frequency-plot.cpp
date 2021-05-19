#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <tuple>

#include <CLI/CLI.hpp>
#include "types.h"
#include "metis_io.h"
#include "vector_io.h"
#include "graph.h"
#include <cmath>

int main(int argc, char* argv[]) {
    CLI::App app;

    std::string input;
    app.add_option("input", input, "The input graph")
            ->required();

    std::string input_format;
    app.add_set("--input-format", input_format, {"binary", "metis"})
            ->required();

    CLI11_PARSE(app, argc, argv);

    auto input_path = std::filesystem::path(input);

    std::cout << "Reading input ..." << std::flush;

    std::vector<EdgeId> first_out;
    std::vector<NodeId> head;
    auto path = input_path.parent_path();
    auto basename = input_path.stem();
    if (input_format == "metis") {
        read_metis(input_path, first_out, head);
    } else if (input_format == "binary") {
        auto first_out_path = path / (basename.string() + ".first_out");
        auto head_path = path / (basename.string() + ".head");
        first_out = load_vector<EdgeId>(first_out_path);
        head = load_vector<NodeId>(head_path);
    }
    ArrayGraph G = ArrayGraph(std::move(first_out), std::move(head));
    std::cout << " finished." << std::endl;
    NodeId max_degree = 0;
    #pragma omp parallel for reduction(max:max_degree)
    for (NodeId node = 0; node < G.node_count(); node++) {
        NodeId degree = G.degree(node);
        if (degree > max_degree) {
            max_degree = degree;
        }
    }
    std::cout << "max_degree=" << max_degree << std::endl;
    std::vector<NodeId> degree_frequency(max_degree + 1);
    std::vector<NodeId> out_degree_frequency(max_degree + 1);
    std::vector<NodeId> in_degree_frequency(max_degree + 1);
    auto is_outgoing = [&](NodeId tail, NodeId head) {
        return std::pair(G.degree(tail), tail) < std::pair(G.degree(head), head);
    };
    #pragma omp parallel for
    for (NodeId node = 0; node < G.node_count(); node++) {
        NodeId degree = G.degree(node);
        NodeId out_degree = 0;
        NodeId in_degree = 0;
        G.for_each_edge(node, [&](NodeId tail, NodeId head) {
            if (is_outgoing(tail, head)) {
                out_degree++;
            } else {
                in_degree++;
            }
        });
        //std::stringstream out;
        //out << "node=" << node <<" deg=" << degree << " out_deg=" << out_degree << std::endl;
        //std::cout << out.str();
        #pragma omp atomic
        degree_frequency[degree]++;
        #pragma omp atomic
        out_degree_frequency[out_degree]++;
        #pragma omp atomic
        in_degree_frequency[in_degree]++;
    }

    auto degree_output_filename = basename.string() + ".degree_freq";
    auto out_degree_output_filename = basename.string() + ".outdegree_freq";
    auto in_degree_output_filename = basename.string() + ".indegree_freq";
    std::ofstream degree_output(path / degree_output_filename);
    std::ofstream out_degree_output(path / out_degree_output_filename);
    std::ofstream in_degree_output(path / in_degree_output_filename);
    for (size_t i = 0; i < degree_frequency.size(); ++i) {
        if (degree_frequency[i] > 0)  {
            degree_output << i << " " << degree_frequency[i] << std::endl;
        }
        if (out_degree_frequency[i] > 0)  {
            out_degree_output << i << " " << out_degree_frequency[i] << std::endl;
        }
        if (in_degree_frequency[i] > 0)  {
            in_degree_output << i << " " << in_degree_frequency[i] << std::endl;
        }
    }
}
