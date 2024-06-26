// Copyright (C) 2024 Max Planck Institute for Dynamics of Complex Technical Systems, Magdeburg
//
// This file is part of phgasnets
//
// SPDX-License-Identifier:  GPL-3.0-or-later

# include <Eigen/SparseCore>
# include "operators.hpp"
# include "pipe.hpp"
# include "compressor.hpp"
# include "network.hpp"
# include "utils.hpp"

namespace PHModel {
    struct SteadySystem{
        SteadySystem(
            const int n_rho, const int n_mom,
            const Jt_operator& Jt,
            Rt_operator& Rt,
            Effort& effort,
            const G_operator& G,
            const Eigen::Vector2d& input_vec
        );

        bool operator()(double const* const* guess_state, double* residual) const;

        private:
            const int n_rho;
            const int n_mom;
            const Jt_operator& Jt;
            Rt_operator& Rt;
            Effort& effort;
            const G_operator& G;
            const Eigen::Vector2d& input_vec;
    };

    struct SteadyCompressorSystem{
        SteadyCompressorSystem(
            Network& network,
            const Eigen::Vector4d& input_vec
        );
        bool operator()(double const* const* guess_state, double* residual) const;
        private:
            Network& network;
            const Eigen::Vector4d& input_vec;
    };
}
