#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <algorithm>

#include "types.h"
#include "metis_io.h"
#include "vector_io.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << "[--sort] INPUT" << std::endl;
        return 1;
    }
    auto begin = argv;
    auto end = argv + argc;
    bool sort = false;
    if (std::find(begin, end, std::string("--sort")) != end) {
        sort = true;
    }
    auto input_path = std::filesystem::path(argv[argc - 1]);

    std::vector<EdgeId> first_out;
    std::vector<NodeId> head;

    std::cout << "Reading input ..." << std::flush;
    try {
        read_metis(input_path, first_out, head);
    } catch (std::runtime_error& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }
    std::cout << " finished." << std::endl;

    if (sort) {
        #pragma omp parallel for
        for (size_t i = 0; i < first_out.size() - 1; i++) {
            auto neighbor_begin = head.begin() + first_out[i];
            auto neighbor_end = head.begin() + first_out[i + 1];
            std::sort(neighbor_begin, neighbor_end);
        }
    }


    auto basename = input_path.stem();
    auto path = input_path.parent_path();
    auto first_out_path = path / (basename.string() + ".first_out");
    auto head_path = path / (basename.string() + ".head");
    std::cout << "Writing first_out ..." << std::flush;
    try {
        save_vector(first_out_path, first_out);
    } catch (std::runtime_error& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }
    std::cout << " finished." << std::endl;

    std::cout << "Writing head ..." << std::flush;
    try {
        save_vector(head_path, head);
    } catch (std::runtime_error& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }
    std::cout << " finished." << std::endl;
    return 0;
}
