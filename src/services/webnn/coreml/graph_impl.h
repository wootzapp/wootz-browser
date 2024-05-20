// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_WEBNN_COREML_GRAPH_IMPL_H_
#define SERVICES_WEBNN_COREML_GRAPH_IMPL_H_

#import <CoreML/CoreML.h>

#include "base/containers/flat_map.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/scoped_refptr.h"
#include "base/sequence_checker.h"
#include "base/task/single_thread_task_runner.h"
#include "base/timer/elapsed_timer.h"
#include "services/webnn/coreml/graph_builder.h"
#include "services/webnn/public/mojom/webnn_context_provider.mojom.h"
#include "services/webnn/public/mojom/webnn_graph.mojom.h"
#include "services/webnn/webnn_graph_impl.h"

namespace webnn::coreml {

// GraphImpl inherits from WebNNGraphImpl to represent a CoreML graph
// implementation. It is mainly responsible for building and compiling a CoreML
// graph from mojom::GraphInfo via GraphBuilder, then initializing and executing
// the graph.
// Mac OS 13.0+ is required for model compilation
// https://developer.apple.com/documentation/coreml/mlmodel/3931182-compilemodel
// Mac OS 14.0+ is required to support WebNN logical binary operators because
// the cast operator does not support casting to uint8 prior to Mac OS 14.0.
// CoreML returns bool tensors for logical operators which need to be cast to
// uint8 tensors to match WebNN expectations.
class API_AVAILABLE(macos(14.0)) GraphImpl final : public WebNNGraphImpl {
 public:
  static void CreateAndBuild(mojom::GraphInfoPtr graph_info,
                             mojom::WebNNContext::CreateGraphCallback callback);

  GraphImpl(const GraphImpl&) = delete;
  GraphImpl& operator=(const GraphImpl&) = delete;
  ~GraphImpl() override;

 private:
  // Additional information about the model input that is required
  // for the CoreML backend.
  struct CoreMLFeatureInfo {
    CoreMLFeatureInfo(MLMultiArrayDataType data_type,
                      NSMutableArray* shape,
                      NSMutableArray* stride,
                      std::string_view coreml_name)
        : data_type(data_type),
          shape(shape),
          stride(stride),
          coreml_name(coreml_name) {}

    MLMultiArrayDataType data_type;
    NSMutableArray* __strong shape;
    NSMutableArray* __strong stride;
    std::string coreml_name;
  };
  static MLFeatureValue* CreateFeatureValue(
      GraphImpl::CoreMLFeatureInfo* feature_info,
      mojo_base::BigBuffer data);
  static std::optional<CoreMLFeatureInfo> GetCoreMLFeatureInfo(
      const GraphBuilder::InputOperandInfo& operand_info);
  using CoreMLFeatureInfoMap = base::flat_map<std::string, CoreMLFeatureInfo>;
  GraphImpl(
      ComputeResourceInfo compute_resource_info,
      std::unique_ptr<CoreMLFeatureInfoMap> input_feature_info,
      base::flat_map<std::string, std::string> coreml_name_to_operand_name,
      MLModel* ml_model);

  // Compile the CoreML model and pass the file path for the compiled
  // temporary .modelc file to OnCreateAndBuildSuccess
  static void CreateAndBuildOnBackgroundThread(
      mojom::GraphInfoPtr graph_info,
      scoped_refptr<base::SequencedTaskRunner> originating_sequence,
      mojom::WebNNContext::CreateGraphCallback callback);

  // CompilationContext shuttles objects between the background thread,
  // CoreML callback from compilation and back to the originating thread.
  // Additionally CompilationContext is responsible for cleaning up any
  // on disk artifacts created by the CoreML model compilation process.
  struct CompilationContext {
    CompilationContext(
        ComputeResourceInfo compute_resource_info,
        std::unique_ptr<CoreMLFeatureInfoMap> input_feature_info,
        base::flat_map<std::string, std::string> coreml_name_to_operand_name,
        base::ScopedTempDir model_file_dir,
        mojom::WebNNContext::CreateGraphCallback callback);
    ~CompilationContext();

    base::ElapsedTimer compilation_timer;
    ComputeResourceInfo compute_resource_info;
    std::unique_ptr<CoreMLFeatureInfoMap> input_feature_info;
    base::flat_map<std::string, std::string> coreml_name_to_operand_name;
    base::ScopedTempDir model_file_dir;
    base::ScopedTempDir compiled_model_dir;
    MLModel* __strong ml_model;
    mojom::WebNNContext::CreateGraphCallback callback;
  };
  static void OnCreateAndBuildFailure(
      mojom::WebNNContext::CreateGraphCallback callback,
      std::string error);
  static void OnCreateAndBuildSuccess(
      std::unique_ptr<CompilationContext> context);

  // Execute the compiled platform graph asynchronously. The `named_inputs` were
  // validated in base class so we can use them to compute directly, the result
  // of execution will be returned to renderer process with the `callback`.
  void ComputeImpl(
      base::flat_map<std::string, mojo_base::BigBuffer> named_inputs,
      mojom::WebNNGraph::ComputeCallback callback) override;
  void DidPredict(base::ElapsedTimer model_predict_timer,
                  mojom::WebNNGraph::ComputeCallback callback,
                  id<MLFeatureProvider> output_features,
                  NSError* error);

  void DispatchImpl(
      const base::flat_map<std::string_view, WebNNBufferImpl*>& named_inputs,
      const base::flat_map<std::string_view, WebNNBufferImpl*>& named_outputs)
      override;

 private:
  SEQUENCE_CHECKER(sequence_checker_);

  std::unique_ptr<CoreMLFeatureInfoMap> input_feature_info_;
  base::flat_map<std::string, std::string> coreml_name_to_operand_name_;
  MLModel* __strong ml_model_;

  base::WeakPtrFactory<GraphImpl> weak_factory_
      GUARDED_BY_CONTEXT(sequence_checker_){this};
};

}  // namespace webnn::coreml

#endif  // SERVICES_WEBNN_COREML_GRAPH_IMPL_H_
