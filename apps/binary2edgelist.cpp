#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "metis_io.h"
#include "types.h"
#include "vector_io.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << "[--sort] [--single-file] INPUT"
              << std::endl;
    return 1;
  }
  auto begin = argv;
  auto end = argv + argc;
  bool sort = false;
  if (std::find(begin, end, std::string("--sort")) != end) {
    sort = true;
  }
  bool single_file = false;
  if (std::find(begin, end, std::string("--single-file")) != end) {
    single_file = true;
  }

  auto input_path = std::filesystem::path(argv[argc - 1]);

  auto basename = input_path.stem();
  auto path = input_path.parent_path();
  auto first_out_path = path / (basename.string() + ".first_out");
  auto head_path = path / (basename.string() + ".head");

  std::vector<EdgeId> first_out;
  std::vector<NodeId> head;

  std::cout << "Reading input ..." << std::flush;
  try {
    first_out = load_vector<EdgeId>(first_out_path);
  } catch (std::runtime_error &e) {
    std::cerr << "[ERROR] " << e.what() << std::endl;
    return 1;
  }
  try {
    head = load_vector<NodeId>(head_path);
  } catch (std::runtime_error &e) {
    std::cerr << "[ERROR] " << e.what() << std::endl;
    return 1;
  }
  std::cout << " finished." << std::endl;

  if (sort) {
    std::cout << "Sorting ..." << std::flush;
#pragma omp parallel for
    for (size_t i = 0; i < first_out.size() - 1; i++) {
      auto neighbor_begin = head.begin() + first_out[i];
      auto neighbor_end = head.begin() + first_out[i + 1];
      std::sort(neighbor_begin, neighbor_end);
    }
    std::cout << " finished." << std::endl;
  }

  auto output_path = path / (basename.string() + ".edgelist");

  std::cout << "Writing output ..." << std::flush;
  std::ofstream out(output_path);
  for (size_t i = 0; i < first_out.size() - 1; i++) {
    auto neighbor_begin = head.begin() + first_out[i];
    auto neighbor_end = head.begin() + first_out[i + 1];
    for (auto edge_iter = neighbor_begin; edge_iter != neighbor_end; edge_iter++) {
        out << i << " " << *edge_iter << std::endl;
    }
  }
  std::cout << " finished." << std::endl;

  return 0;
}
