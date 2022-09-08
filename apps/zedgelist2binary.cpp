#include <fstream>
#include <iostream>
#include <filesystem>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <stxxl.h>
#include <stxxl/cmdline>

struct Edge {
    uint64_t tail, head;
    Edge reverse() const
    {
        return Edge{head, tail};
    }
    bool operator==(Edge const& other) const {
        return this->tail == other.tail && this->head == other.head;
    }
};

struct EdgeSorter {
    bool operator()(Edge const& e1, Edge const& e2) const {
        return std::tuple(e1.tail, e1.head) < std::tuple(e2.tail, e2.head);
    }
    static Edge min_value() {
        return Edge {0, 0};
    }
    static Edge max_value() {
        return Edge {std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max()};
    }
};

int main(int argc, char** argv) {
    stxxl::cmdline_parser cp;
    stxxl::uint64 buffer_size = 400 * 1024 * 1024;
    cp.add_bytes('s', "size", "Number of bytes to use internally for sorting", buffer_size);
    std::string outfile = "";
    cp.add_param_string("outfile", "Base path of the output", outfile);
    std::vector<std::string> inputs;
    cp.add_param_stringlist("inputs", "Compressed input files", inputs);
    bool compressed = false;
    cp.add_flag('c', "compressed", "input is compressed", compressed);

    if(!cp.process(argc, argv)) {
        return -1;
    }

    // Read from the first command line argument, assume it's gzipped
    using edge_vector = stxxl::VECTOR_GENERATOR<Edge>::result;
    edge_vector edges;
    edge_vector::bufwriter_type writer(edges);
    uint64_t max_node_id = 0;
    for (auto i = 0; i < inputs.size(); i++) 
    {
        std::cout << "Start reading file #" << i << std::endl;
        std::ifstream file(inputs[i], std::ios_base::in | std::ios_base::binary);
        boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
        if (compressed) {
            inbuf.push(boost::iostreams::gzip_decompressor());
        }
        inbuf.push(file);
        // Convert streambuf to istream
        std::istream instream(&inbuf);
        // Iterate lines
        std::string line;
        size_t linenum = 0;
        while (std::getline(instream, line))
        {
            if (linenum % 1000000 == 0)
            {
                std::cout << "Read " << linenum / 1000000 << "M edges" << std::endl;
            }
            Edge edge;
            std::istringstream linestream(line);
            linestream >> edge.tail >> edge.head;
            writer << edge;
            writer << edge.reverse();
            max_node_id = std::max(edge.tail, max_node_id);
            max_node_id = std::max(edge.head, max_node_id);
            linenum++;
        }
        // Cleanup
        file.close();
    }
    writer.finish();
    std::cout << "Finished reading " << edges.size() << " edges" << std::endl;
    std::cout << "max_node_id=" << max_node_id << std::endl;
    stxxl::sort(edges.begin(), edges.end(), EdgeSorter {}, buffer_size);
    std::cout << "Finished sorting." << std::endl;
    using id_vector = stxxl::VECTOR_GENERATOR<uint64_t>::result;
    auto out_path = std::filesystem::path(outfile);
    auto basename = out_path.stem();
    auto path = out_path.parent_path();
    auto first_out_path = path / (basename.string() + ".first_out");
    auto head_path = path / (basename.string() + ".head");
    stxxl::syscall_file first_out_file(first_out_path, stxxl::file::RDWR | stxxl::file::CREAT | stxxl::file::DIRECT);
    id_vector first_out(&first_out_file, max_node_id + 2);
    id_vector::bufwriter_type first_out_writer(first_out);
    stxxl::syscall_file head_file(head_path, stxxl::file::RDWR | stxxl::file::CREAT | stxxl::file::DIRECT);
    id_vector head(&head_file, edges.size());
    id_vector::bufwriter_type head_writer(head);
    uint64_t running_sum = 0;
    uint64_t first_out_index = 0;
    // first_out_writer << running_sum;
    // first_out_index++;
    Edge previous{std::numeric_limits<uint64_t>::max(), 0};
    for (edge_vector::bufreader_type reader(edges); !reader.empty(); ++reader) {
        //skip duplicate edges
        if (*reader == previous) {
            continue;
        }
        if (reader->tail != previous.tail) {
            std::cout << "New node " << reader->tail << std::endl;
            while (first_out_index == 0 || first_out_index <= reader->tail ) {
                std::cout << "writing first_out[" << first_out_index << "]" << std::endl;
                first_out_writer << running_sum;
                first_out_index++;
            }
        }
        head_writer << reader->head;
        previous = *reader;
        running_sum++;
    }
    auto node_count = max_node_id + 1;
    while(first_out_index < node_count + 1) {
        std::cout << "writing first_out[" << first_out_index << "]" << std::endl;
        first_out_writer << running_sum;
        first_out_index++;
    }
    first_out_writer << running_sum;
    first_out_writer.finish();
    head_writer.finish();
    first_out.resize(node_count + 1);
    head.resize(running_sum);
    std::cout << "n=" << first_out.size() - 1 << ", m=" << head.size() / 2 << std::endl;
}