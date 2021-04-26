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
    if (input_format == "metis") {
        read_metis(input_path, first_out, head);
    } else if (input_format == "binary") {
        auto basename = input_path.stem();
        auto path = input_path.parent_path();
        auto first_out_path = path / (basename.string() + ".first_out");
        auto head_path = path / (basename.string() + ".head");
        first_out = load_vector<EdgeId>(first_out_path);
        head = load_vector<NodeId>(head_path);
    }
    ArrayGraph G = ArrayGraph(std::move(first_out), std::move(head));
    std::cout << " finished." << std::endl;
    NodeId n = G.node_count();
    EdgeId m = G.edge_count();
    EdgeId wedges = 0;
    EdgeId degree_sum = 0;
    NodeId max_degree = 0;
    #pragma omp parallel for reduction(+:wedges, degree_sum) reduction(max:max_degree)
    for (NodeId node = 0; node < G.node_count(); node++) {
        NodeId degree = G.degree(node);
        degree_sum += degree;
        wedges += (degree * (degree - 1)) / 2;
        if (degree > max_degree) {
            max_degree = degree;
        }
    }
    double avg_degree = static_cast<double>(degree_sum) / n;
    double edge_density = static_cast<double>(2 * m) / (n * (n - 1));
    std::cout << "n=" << n
            <<", m=" << m
            << ", max_deg=" << max_degree
            << ", avg_deg=" << avg_degree
            << ", wedges="<< wedges
            << ", edge_density="<< edge_density
            << std::endl;

}
