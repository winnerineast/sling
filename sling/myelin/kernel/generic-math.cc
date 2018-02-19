// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "sling/myelin/kernel/generic.h"

#include <math.h>
#include <string>

#include "sling/myelin/compute.h"
#include "sling/myelin/macro-assembler.h"

#define __ masm->

namespace sling {
namespace myelin {

using namespace jit;

static float sigmoid(float x) {
  return 1.0 / (1.0 + expf(-x));
}

static float relu(float x) {
  return x > 0.0 ? x : 0;
}

// Compute element-wise float function.
class GenericFltMathFunction : public Kernel {
 public:
  typedef float (*FltFunc)(float);

  bool Supports(Step *step) override {
    // Requires CPU with SSE support.
    if (!CPU::Enabled(SSE)) return false;

    // Check inputs and outputs.
    if (step->inputs().size() != 1) return false;
    if (step->outputs().size() != 1) return false;
    Tensor *x = step->input(0);
    Tensor *y = step->output(0);

    // Check type.
    if (x->type() != DT_FLOAT) return false;
    if (y->type() != DT_FLOAT) return false;

    // Input and output must have same shape.
    if (!x->HasSameShape(y)) return false;

    return true;
  }

  void Adjust(Step *step) override {
    // Input and output must have the same alignment.
    step->input(0)->SameAlign(step->output(0));

    // Reserve four preserved registers.
    step->SetPreservedRegisterUsage(4);

    // Allow in-place operation.
    step->AllowInPlace(0, 0);
  }

  virtual FltFunc Function() = 0;
  virtual string FunctionSymbol() = 0;

  void Generate(Step *step, MacroAssembler *masm) override {
    Label l;

    // Get input and output.
    Tensor *x = step->input(0);
    Tensor *y = step->output(0);

    // Assign registers.
    Register input = masm->rr().alloc_preserved();
    Register output = masm->rr().alloc_preserved();
    Register ofs = masm->rr().alloc_preserved();
    Register func = masm->rr().alloc_preserved();
    XMMRegister value = xmm0;

    // Load tensor locations.
    __ LoadTensorAddress(input, x);
    if (y->SharedWith(x)) {
      output = input;
    } else {
      __ LoadTensorAddress(output, y);
    }

    // Get address of underlying function implementing function.
    void *funcaddr = reinterpret_cast<void *>(Function());
    __ load_extern(func, funcaddr, FunctionSymbol());
    __ xorq(ofs, ofs);

    // Loop over elements in tensor.
    __ LoopStart(&l);

    // Get next input value.
    __ movss(value, Operand(input, ofs));

    // Call function.
    __ call(func);

    // Save result in output.
    __ movss(Operand(output, ofs), value);

    // Next element.
    __ addq(ofs, Immediate(sizeof(float)));
    __ cmpq(ofs, Immediate(x->size()));
    __ j(less, &l);
  }
};

class GenericFltAbs : public GenericFltMathFunction {
 public:
  string Name() override { return "GenFltAbs"; }
  string Operation() override { return "Abs"; }
  FltFunc Function() override { return fabsf; }
  string FunctionSymbol() override { return "fabsf"; }
};

class GenericFltSqrt : public GenericFltMathFunction {
 public:
  string Name() override { return "GenFltSqrt"; }
  string Operation() override { return "Sqrt"; }
  FltFunc Function() override { return sqrtf; }
  string FunctionSymbol() override { return "sqrtf"; }
};

class GenericFltExp : public GenericFltMathFunction {
 public:
  string Name() override { return "GenFltExp"; }
  string Operation() override { return "Exp"; }
  FltFunc Function() override { return expf; }
  string FunctionSymbol() override { return "expf"; }
};

class GenericFltLog : public GenericFltMathFunction {
 public:
  string Name() override { return "GenFltLog"; }
  string Operation() override { return "Log"; }
  FltFunc Function() override { return logf; }
  string FunctionSymbol() override { return "logf"; }
};

class GenericFltCeil : public GenericFltMathFunction {
 public:
  string Name() override { return "GenFltCeil"; }
  string Operation() override { return "Ceil"; }
  FltFunc Function() override { return ceilf; }
  string FunctionSymbol() override { return "ceilf"; }
};

class GenericFltFloor : public GenericFltMathFunction {
 public:
  string Name() override { return "GenFltFloor"; }
  string Operation() override { return "Floor"; }
  FltFunc Function() override { return floorf; }
  string FunctionSymbol() override { return "floorf"; }
};

class GenericFltCos : public GenericFltMathFunction {
 public:
  string Name() override { return "GenFltCos"; }
  string Operation() override { return "Cos"; }
  FltFunc Function() override { return cosf; }
  string FunctionSymbol() override { return "cosf"; }
};

class GenericFltSin : public GenericFltMathFunction {
 public:
  string Name() override { return "GenFltSin"; }
  string Operation() override { return "Sin"; }
  FltFunc Function() override { return sinf; }
  string FunctionSymbol() override { return "sinf"; }
};

class GenericFltTan : public GenericFltMathFunction {
 public:
  string Name() override { return "GenFltTan"; }
  string Operation() override { return "Tan"; }
  FltFunc Function() override { return tanf; }
  string FunctionSymbol() override { return "tanf"; }
};

class GenericFltTanh : public GenericFltMathFunction {
 public:
  string Name() override { return "GenFltTanh"; }
  string Operation() override { return "Tanh"; }
  FltFunc Function() override { return tanhf; }
  string FunctionSymbol() override { return "tanhf"; }
};

class GenericFltSigmoid : public GenericFltMathFunction {
 public:
  string Name() override { return "GenFltSigmoid"; }
  string Operation() override { return "Sigmoid"; }
  FltFunc Function() override { return sigmoid; }
  string FunctionSymbol() override { return "sigmoid"; }
};

class GenericFltRelu : public GenericFltMathFunction {
 public:
  string Name() override { return "GenFltRelu"; }
  string Operation() override { return "Relu"; }
  FltFunc Function() override { return relu; }
  string FunctionSymbol() override { return "relu"; }
};

// Compute argmax of input.
class GenericFltArgMax : public Kernel {
 public:
  string Name() override { return "GenFltArgMax"; }
  string Operation() override { return "ArgMax"; }

  bool Supports(Step *step) override {
    // Check inputs and outputs.
    if (step->inputs().size() != 1) return false;
    if (step->outputs().size() != 1) return false;
    Tensor *x = step->input(0);
    Tensor *y = step->output(0);

    // Check type.
    if (x->type() != DT_FLOAT) return false;
    if (y->type() != DT_INT32 && y->type() != DT_INT64) return false;
    if (y->elements() != 1) return false;

    return true;
  }

  void Generate(Step *step, MacroAssembler *masm) override {
    // Get input and output.
    Tensor *x = step->input(0);
    Tensor *y = step->output(0);

    // Assign registers.
    Register input = masm->rr().alloc();
    Register output = masm->rr().alloc();
    Register idx = masm->rr().alloc();
    Register best = masm->rr().alloc();
    XMMRegister value = masm->mm().allocx();
    XMMRegister maxval = masm->mm().allocx();

    // Load tensor locations.
    __ LoadTensorAddress(input, x);
    __ LoadTensorAddress(output, y);

    // Initialize max value.
    __ movq(best, Immediate(-1));
    __ movss(maxval, Operand(masm->GetConstant<float>(-INFINITY)->address()));

    // Loop over elements in tensor.
    __ xorq(idx, idx);
    Label loop;
    __ LoopStart(&loop);

    // Get next input value.
    __ movss(value, Operand(input, idx, times_4));

    // Check if value is greater than current max value.
    Label l1;
    __ ucomiss(value, maxval);
    __ j(below_equal, &l1);
    __ movss(maxval, value);
    __ movq(best, idx);
    __ bind(&l1);

    // Next element.
    __ incq(idx);
    __ cmpq(idx, Immediate(x->elements()));
    __ j(less, &loop);

    // Save output.
    if (y->type() == DT_INT32) {
      __ movl(Operand(output), best);
    } else {
      __ movq(Operand(output), best);
    }
  }

  int64 Complexity(const Step *step) override {
    return step->input(0)->elements();
  }
};

void RegisterGenericMath(Library *library) {
  // Computes  : y = abs(x) element-wise
  // Input     : x: float32[d1,...,dn]
  // Output    : y: float32[d1,...,dn]
  library->Register(new GenericFltAbs());

  // Computes  : y = sqrt(x) element-wise
  // Input     : x: float32[d1,...,dn]
  // Output    : y: float32[d1,...,dn]
  library->Register(new GenericFltSqrt());

  // Computes  : y = exp(x) element-wise
  // Input     : x: float32[d1,...,dn]
  // Output    : y: float32[d1,...,dn]
  library->Register(new GenericFltExp());

  // Computes  : y = log(x) element-wise
  // Input     : x: float32[d1,...,dn]
  // Output    : y: float32[d1,...,dn]
  library->Register(new GenericFltLog());

  // Computes  : y = ceil(x) element-wise
  // Input     : x: float32[d1,...,dn]
  // Output    : y: float32[d1,...,dn]
  library->Register(new GenericFltCeil());

  // Computes  : y = floor(x) element-wise
  // Input     : x: float32[d1,...,dn]
  // Output    : y: float32[d1,...,dn]
  library->Register(new GenericFltFloor());

  // Computes  : y = cos(x) element-wise
  // Input     : x: float32[d1,...,dn]
  // Output    : y: float32[d1,...,dn]
  library->Register(new GenericFltCos());

  // Computes  : y = sin(x) element-wise
  // Input     : x: float32[d1,...,dn]
  // Output    : y: float32[d1,...,dn]
  library->Register(new GenericFltSin());

  // Computes  : y = tan(x) element-wise
  // Input     : x: float32[d1,...,dn]
  // Output    : y: float32[d1,...,dn]
  library->Register(new GenericFltTan());

  // Computes  : y = tanh(x) element-wise
  // Input     : x: float32[d1,...,dn]
  // Output    : y: float32[d1,...,dn]
  library->Register(new GenericFltTanh());

  // Computes  : y = sigmoid(x) = 1 / (1 + exp(-x)) element-wise
  // Input     : x: float32[d1,...,dn]
  // Output    : y: float32[d1,...,dn]
  library->Register(new GenericFltSigmoid());

  // Computes  : y = relu(x) = max(0, x) element-wise
  // Input     : x: float32[d1,...,dn]
  // Output    : y: float32[d1,...,dn]
  library->Register(new GenericFltRelu());

  // Computes  : y = argmax(x)
  // Input     : x: float32[d1,...,dn]
  // Output    : y: int32/int64
  library->Register(new GenericFltArgMax());
}

}  // namespace myelin
}  // namespace sling

