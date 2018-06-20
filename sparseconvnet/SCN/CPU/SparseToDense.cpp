// Copyright 2016-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the license found in the
// LICENSE file in the root directory of this source tree.

#include "SparseToDense.h"

template <typename T, Int Dimension>
void cpu_SparseToDense_updateOutput(
    /*long*/ at::Tensor inputSize, Metadata<Dimension> &m,
    /*float*/ at::Tensor input_features,
    /*float*/ at::Tensor output_features, long nPlanes) {

  {
    std::array<long, Dimension + 2> sz;
    sz[0] = m.grids.begin()->second.size(); // batch size
    sz[1] = nPlanes;
    long *in_sz = inputSize.data<long>();
    for (Int i = 0; i < Dimension; ++i)
      sz[i + 2] = in_sz[i];
    output_features.resize_(sz);
    output_features.zero_();
  }
  if (input_features.ndimension() == 2) {
    auto _rules = m.getSparseToDenseRuleBook(inputSize, true);
    Int _nPlanes = input_features.size(1);
    auto iF = input_features.data<T>();
    auto oF = output_features.data<T>();
    long spatialVolume = inputSize.prod().data<long>()[0];
    for (auto &r : _rules) {
      Int nHot = r.size() / 2;
      SparseToDense_ForwardPass<T>(iF, oF, _nPlanes, spatialVolume, &r[0],
                                   nHot);
      oF += _nPlanes * spatialVolume;
    }
  }
}
template <typename T, Int Dimension>
void cpu_SparseToDense_updateGradInput(
    /*long*/ at::Tensor inputSize, Metadata<Dimension> &m,
    /*float*/ at::Tensor input_features,
    /*float*/ at::Tensor d_input_features,
    /*float*/ at::Tensor d_output_features) {

  d_input_features.resize_as_(input_features);
  d_input_features.zero_();
  if (input_features.ndimension() == 2) {
    auto _rules = m.getSparseToDenseRuleBook(inputSize, true);
    long spatialVolume = inputSize.prod().data<long>()[0];
    Int _nPlanes = d_input_features.size(1);
    auto diF = d_input_features.data<T>();
    auto doF = d_output_features.data<T>();
    for (auto &r : _rules) {
      Int nHot = r.size() / 2;
      SparseToDense_BackwardPass<T>(diF, doF, _nPlanes, spatialVolume, &r[0],
                                    nHot);
      doF += _nPlanes * spatialVolume;
    }
  }
}
