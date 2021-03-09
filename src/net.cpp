#include "net.h"

#include <filesystem>

using namespace neocortex::nn;

Network::Network(std::string path) : model(path) {
	name = std::filesystem::path(path).filename().string();

	if (name.rfind("gen", 0) == 0) {
		name = std::string("Nc0 generation ") + name.substr(3);
	}
}

std::string Network::get_name() {
	return name;
}

std::vector<cppflow::tensor> Network::evaluate(std::vector<float>& inp_board, std::vector<float>& inp_lmm, int num_batches) {
	auto inp = cppflow::tensor(TF_FLOAT, &inp_board[0], num_batches * 8 * 8 * 85 * sizeof(float), { num_batches, 8, 8, 85 });
	auto lmm = cppflow::tensor(TF_FLOAT, &inp_lmm[0], num_batches * 4096 * sizeof(float), { num_batches, 4096 });

	return model(
		{ {"serving_default_input:0", inp}, {"serving_default_legal_mask:0", lmm} },
		{ "StatefulPartitionedCall:0", "StatefulPartitionedCall:1" }
	);
}