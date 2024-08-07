// Copyright (C) 2024 Max Planck Institute for Dynamics of Complex Technical Systems, Magdeburg
//
// This file is part of phgasnets
//
// SPDX-License-Identifier:  GPL-3.0-or-later

# pragma once

# include "operators.hpp"
# include "pipe.hpp"
# include "compressor.hpp"
# include "utils.hpp"

# include <nlohmann/json.hpp>
# include <ceres/jet.h>
# include <vector>

namespace phgasnets {

  struct Network{
    Network(
      std::vector<Pipe>& pipes,
      std::vector<Compressor>& compressors
    ): pipes(pipes), compressors(compressors)
    {}

    public:
      std::vector<Pipe>& pipes;
      std::vector<Compressor>& compressors;
  };

  template<typename T>
  struct DiscreteNetwork {

    DiscreteNetwork(
      std::vector<DiscretePipe<T>>& pipes,
      std::vector<Compressor>& compressors
    ):
      pipes(pipes), compressors(compressors), n_state(0), n_res(0)
    {
      // diagonally block static operators
      std::vector<std::reference_wrapper<BaseOperator<double>>> operators_e, operators_j;
      std::vector<std::reference_wrapper<BaseOperator<T>>> operators_r, operators_g;
      for (auto& pipe: pipes) {
        operators_e.push_back(std::ref(pipe.Et));
        operators_j.push_back(std::ref(pipe.Jt));
        operators_r.push_back(std::ref(pipe.Rt));
        operators_g.push_back(std::ref(pipe.G));
        n_state += pipe.n_state;
        n_res += pipe.n_res;
      }
      E = diagonalBlock<double>(operators_e);
      J = diagonalBlock<double>(operators_j);
      R = diagonalBlock<T>(operators_r);
      G = diagonalBlock<T>(operators_g);
      effort.resize(n_res);
    };

    void set_state(const Eigen::Ref<const Eigen::Vector<T, Eigen::Dynamic>>& state) {
      // distribute state across network constituents
      int pipe_state_startIdx = 0;
      for (auto& pipe : pipes) {
        auto pipe_state = state(Eigen::seqN(pipe_state_startIdx, pipe.n_state));
        pipe.set_state(pipe_state);
        pipe_state_startIdx += pipe.n_state;
      }

      // update pipe-specific R and effort
      std::vector<std::reference_wrapper<BaseOperator<T>>> operators_r, operators_g;

      int pipe_res_startIdx = 0;
      for(auto& pipe : pipes){
        operators_r.push_back(std::ref(pipe.Rt));
        effort(Eigen::seqN(pipe_res_startIdx, pipe.n_res)) = pipe.effort.vec_t;
        pipe_res_startIdx += pipe.n_res;
      }

      // build network R operator
      R = diagonalBlock<T>(operators_r);

      // update network G operator
      auto precompressor_pressure = phgasnets::GAS_CONSTANT * pipes[0].temperature * pipes[0].rho(Eigen::last);
      auto postcompressor_momentum = pipes[1].mom(0);

      G.coeffRef(pipes[0].n_res-1, 1) = -postcompressor_momentum;
      if (compressors[0].type == "FC") {
        G.coeffRef(pipes[0].n_res+pipes[1].n_res-2, 2) = T(precompressor_pressure);
      }
      else {
        if (compressors[0].type == "FP" && compressors[0].model == "AV") {
          G.coeffRef(pipes[0].n_res-1, 1) *= ceres::pow(precompressor_pressure, 1.0/compressors[0].isentropic_exponent);
        }
      }
    }

    public:
      std::vector<DiscretePipe<T>> pipes;
      std::vector<Compressor> compressors;
      Eigen::SparseMatrix<double> E, J;
      Eigen::SparseMatrix<T> R, G;
      Eigen::Vector<T, Eigen::Dynamic> effort;
      int n_state, n_res;
  };

  template<typename T>
  DiscreteNetwork<T> discretize(const Network& network, const nlohmann::json& spatial_disc_params){

    std::vector<DiscretePipe<T>> discrete_pipes;
    int Nx = spatial_disc_params["resolution"];

    for (auto& pipe: network.pipes)
      discrete_pipes.push_back(DiscretePipe<T>(pipe, Nx));

    return DiscreteNetwork<T>(discrete_pipes, network.compressors);
  }
} // namespace phgasnets
