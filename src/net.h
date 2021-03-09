/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include <string>
#include <vector>
#include <initializer_list>

#include "cppflow/cppflow.h"

namespace neocortex {
	namespace nn {
		class Network {
		public:
			struct Output {
				float value;
				float policy[4096];
			};

			Network(std::string path);

			std::string get_name();

			std::vector<cppflow::tensor> evaluate(std::vector<float>& inp_board, std::vector<float>& inp_lmm, int num_batches);

		private:
			cppflow::model model;
			std::string name;
		};
	}
}