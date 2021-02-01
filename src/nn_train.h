#pragma once

/*
 * NN training routines
 */

namespace neocortex {
    namespace nn {
        /**
         * Trains the network to approximate a n-millisecond classical search.
         *
         * @param ms Search ms. Lower results in faster training, but higher provides better training data.
         * @param abort_flag Abort flag.
         */
        void start_classical_training(int ms);

        /**
         * Stops any training threads and joins them.
         */
        void stop_training();
    }
}
