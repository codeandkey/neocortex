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

			/**
			 * Loads a new model from a path.
			 * Throws a runtime_error on failure.
			 *
			 * @param path Path to model directory
			 */
			Network(std::string path);

			/**
			 * Gets the network basename.
			 *
			 * @return Name of model directory without leading path.
			 */
			std::string get_name();

			/**
			 * Evaluates an input layer and returns the output tensors.
			 *
			 * @param inp_board Board input layer [num_batches * 8 * 8 * 85 elements min.]
			 * @param inp_lmm Legal move mask [num_batches * 4096 elements min.]
			 * @param num_batches Number of layers to evaluate in parallel.
			 * @return[0] policy output [num_batches * 4096 elements]
			 * @return[1] value output [num_batches * 1 elements]
			 */
			std::vector<cppflow::tensor> evaluate(std::vector<float>& inp_board, std::vector<float>& inp_lmm, int num_batches);

		private:
			cppflow::model model;
			std::string name;
		};
	}
}
