package(default_visibility = ["//visibility:public"])

cc_library(
  name = "generic",
  srcs = [
    "array.cc",
    "generic.cc",
    "generic-math.cc",
    "generic-matmul.cc",
    "generic-operators.cc",
  ],
  hdrs = ["generic.h"],
  deps = [
    "//sling/base",
    "//sling/myelin:compute",
  ],
)

cc_library(
  name = "arithmetic",
  srcs = [
    "arithmetic.cc",
  ],
  hdrs = ["arithmetic.h"],
  deps = [
    "//sling/base",
    "//sling/myelin:compute",
    "//sling/myelin:express",
    "//sling/myelin/generator:elementwise",
    "//sling/myelin/generator:expression",
  ],
)

cc_library(
  name = "sse",
  srcs = [
    "sse.cc",
    "sse-matmul.cc",
  ],
  hdrs = ["sse.h"],
  deps = [
    "//sling/base",
    "//sling/myelin:compute",
  ],
)

cc_library(
  name = "avx",
  srcs = [
    "avx.cc",
    "avx-math.cc",
    "avx-matmul.cc",
    "avx-operators.cc",
  ],
  hdrs = ["avx.h"],
  deps = [
    "//sling/base",
    "//sling/myelin:compute",
  ],
)

cc_library(
  name = "precompute",
  srcs = ["precompute.cc"],
  hdrs = ["precompute.h"],
  deps = [
    ":arithmetic",
    ":generic",
    "//sling/myelin:compute",
  ],
)

cc_library(
  name = "tensorflow",
  srcs = ["tensorflow.cc"],
  hdrs = ["tensorflow.h"],
  deps = [
    ":arithmetic",
    ":avx",
    ":generic",
    ":precompute",
    ":sse",
    "//sling/myelin:compute",
  ],
)

cc_library(
  name = "cuda",
  srcs = [
    "cuda.cc",
    "cuda-array.cc",
    "cuda-matmul.cc",
    "cuda-arithmetic.cc",
  ],
  hdrs = ["cuda.h"],
  deps = [
    ":arithmetic",
    "//sling/myelin:compute",
    "//sling/myelin:express",
    "//sling/myelin/cuda:cuda-kernel",
  ],
)

cc_library(
  name = "dragnn",
  srcs = ["dragnn.cc"],
  hdrs = ["dragnn.h"],
  deps = [
    "//sling/base",
    "//sling/myelin:compute",
  ],
)

