/* Copyright (C) 2016-2018 Alibaba Group Holding Limited

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "xdl/core/lib/status.h"
#include "xdl/core/framework/op_kernel.h"
#include "xdl/core/framework/op_define.h"
#include "xdl/core/framework/op_registry.h"
#include "xdl/core/ops/ps_ops/define_op.h"
#include "xdl/core/ops/ps_ops/convert_utils.h"
#include "xdl/core/ops/ps_ops/client.h"
#include "xdl/core/ops/ps_ops/var_type.h"

namespace xdl {

class PsIsInitializedOp : public xdl::OpKernelAsync {
 public:
  Status Init(OpKernelConstruction* ctx) override {
    XDL_CHECK_STATUS(ctx->GetAttr("var_name", &var_name_));
    return Status::Ok();
  }

  void Compute(OpKernelContext* ctx, Callback done) override {
    ps::client::BaseClient* client;
    XDL_CHECK_STATUS_ASYNC(GetClient(&client), done);
    bool* result = new bool;
    auto cb = [ctx, done, result](const ps::Status& st) {
      std::unique_ptr<bool> result_deleter(result);
      XDL_CHECK_STATUS_ASYNC(PS2XDL::ConvertStatus(st), done);
      Tensor output;
      XDL_CHECK_STATUS_ASYNC(
          ctx->AllocateOutput(0, TensorShape({}), &output), 
          done);
      int8_t* output_ptr = output.Raw<int8_t>();
      *output_ptr = static_cast<int8_t>(*result);
      done(Status::Ok());
    };

    client->IsInitialized(var_name_, result, cb);
  }

 private:
  std::string var_name_;
};

XDL_DEFINE_OP(PsIsInitializedOp)
  .Output("ret", DataType::kInt8)
  .Attr("var_name", AttrValue::kString);

XDL_REGISTER_KERNEL(PsIsInitializedOp, PsIsInitializedOp).Device("CPU");

} // namespace xdl


