/*!
 * Copyright 2017-2019 XGBoost contributors
 */
#include <gtest/gtest.h>
#include <xgboost/objective.h>
#include <xgboost/generic_parameters.h>
#include "../helpers.h"

TEST(Objective, DeclareUnifiedTest(LinearRegressionGPair)) {
  xgboost::LearnerTrainParam tparam = xgboost::CreateEmptyGenericParam(0, NGPUS);
  std::vector<std::pair<std::string, std::string>> args;

  xgboost::ObjFunction * obj =
      xgboost::ObjFunction::Create("reg:squarederror", &tparam);

  obj->Configure(args);
  CheckObjFunction(obj,
                   {0, 0.1f, 0.9f,   1,    0,  0.1f, 0.9f,  1},
                   {0,   0,   0,   0,    1,    1,    1, 1},
                   {1,   1,   1,   1,    1,    1,    1, 1},
                   {0, 0.1f, 0.9f, 1.0f, -1.0f, -0.9f, -0.1f, 0},
                   {1,   1,   1,   1,    1,    1,    1, 1});
  CheckObjFunction(obj,
                   {0, 0.1f, 0.9f,   1,    0,  0.1f, 0.9f,  1},
                   {0,   0,   0,   0,    1,    1,    1, 1},
                   {},  // empty weight
                   {0, 0.1f, 0.9f, 1.0f, -1.0f, -0.9f, -0.1f, 0},
                   {1,   1,   1,   1,    1,    1,    1, 1});
  ASSERT_NO_THROW(obj->DefaultEvalMetric());

  delete obj;
}

TEST(Objective, DeclareUnifiedTest(LogisticRegressionGPair)) {
  xgboost::LearnerTrainParam tparam = xgboost::CreateEmptyGenericParam(0, NGPUS);
  std::vector<std::pair<std::string, std::string>> args;
  xgboost::ObjFunction * obj = xgboost::ObjFunction::Create("reg:logistic", &tparam);

  obj->Configure(args);
  CheckObjFunction(obj,
                   {   0,  0.1f,  0.9f,    1,    0,   0.1f,  0.9f,      1}, // preds
                   {   0,    0,    0,    0,    1,     1,     1,     1}, // labels
                   {   1,    1,    1,    1,    1,     1,     1,     1}, // weights
                   { 0.5f, 0.52f, 0.71f, 0.73f, -0.5f, -0.47f, -0.28f, -0.26f}, // out_grad
                   {0.25f, 0.24f, 0.20f, 0.19f, 0.25f,  0.24f,  0.20f,  0.19f}); // out_hess

  delete obj;
}

TEST(Objective, DeclareUnifiedTest(LogisticRegressionBasic)) {
  xgboost::LearnerTrainParam lparam = xgboost::CreateEmptyGenericParam(0, NGPUS);
  std::vector<std::pair<std::string, std::string>> args;
  xgboost::ObjFunction * obj = xgboost::ObjFunction::Create("reg:logistic", &lparam);

  obj->Configure(args);

  // test label validation
  EXPECT_ANY_THROW(CheckObjFunction(obj, {0}, {10}, {1}, {0}, {0}))
    << "Expected error when label not in range [0,1f] for LogisticRegression";

  // test ProbToMargin
  EXPECT_NEAR(obj->ProbToMargin(0.1f), -2.197f, 0.01f);
  EXPECT_NEAR(obj->ProbToMargin(0.5f), 0, 0.01f);
  EXPECT_NEAR(obj->ProbToMargin(0.9f), 2.197f, 0.01f);
  EXPECT_ANY_THROW(obj->ProbToMargin(10))
    << "Expected error when base_score not in range [0,1f] for LogisticRegression";

  // test PredTransform
  xgboost::HostDeviceVector<xgboost::bst_float> io_preds = {0, 0.1f, 0.5f, 0.9f, 1};
  std::vector<xgboost::bst_float> out_preds = {0.5f, 0.524f, 0.622f, 0.710f, 0.731f};
  obj->PredTransform(&io_preds);
  auto& preds = io_preds.HostVector();
  for (int i = 0; i < static_cast<int>(io_preds.Size()); ++i) {
    EXPECT_NEAR(preds[i], out_preds[i], 0.01f);
  }

  delete obj;
}

TEST(Objective, DeclareUnifiedTest(LogisticRawGPair)) {
  xgboost::LearnerTrainParam lparam = xgboost::CreateEmptyGenericParam(0, NGPUS);
  std::vector<std::pair<std::string, std::string>> args;
  xgboost::ObjFunction * obj = xgboost::ObjFunction::Create("binary:logitraw", &lparam);

  obj->Configure(args);
  CheckObjFunction(obj,
                   {   0,  0.1f,  0.9f,    1,    0,   0.1f,   0.9f,     1},
                   {   0,    0,    0,    0,    1,     1,     1,     1},
                   {   1,    1,    1,    1,    1,     1,     1,     1},
                   { 0.5f, 0.52f, 0.71f, 0.73f, -0.5f, -0.47f, -0.28f, -0.26f},
                   {0.25f, 0.24f, 0.20f, 0.19f, 0.25f,  0.24f,  0.20f,  0.19f});

  delete obj;
}

TEST(Objective, DeclareUnifiedTest(PoissonRegressionGPair)) {
  xgboost::LearnerTrainParam lparam = xgboost::CreateEmptyGenericParam(0, NGPUS);
  std::vector<std::pair<std::string, std::string>> args;
  xgboost::ObjFunction * obj = xgboost::ObjFunction::Create("count:poisson", &lparam);

  args.emplace_back(std::make_pair("max_delta_step", "0.1f"));
  obj->Configure(args);
  CheckObjFunction(obj,
                   {   0,  0.1f,  0.9f,    1,    0,  0.1f,  0.9f,    1},
                   {   0,    0,    0,    0,    1,    1,    1,    1},
                   {   1,    1,    1,    1,    1,    1,    1,    1},
                   {   1, 1.10f, 2.45f, 2.71f,    0, 0.10f, 1.45f, 1.71f},
                   {1.10f, 1.22f, 2.71f, 3.00f, 1.10f, 1.22f, 2.71f, 3.00f});
  CheckObjFunction(obj,
                   {   0,  0.1f,  0.9f,    1,    0,  0.1f,  0.9f,    1},
                   {   0,    0,    0,    0,    1,    1,    1,    1},
                   {},  // Empty weight
                   {   1, 1.10f, 2.45f, 2.71f,    0, 0.10f, 1.45f, 1.71f},
                   {1.10f, 1.22f, 2.71f, 3.00f, 1.10f, 1.22f, 2.71f, 3.00f});
  delete obj;
}

TEST(Objective, DeclareUnifiedTest(PoissonRegressionBasic)) {
  xgboost::LearnerTrainParam lparam = xgboost::CreateEmptyGenericParam(0, NGPUS);
  std::vector<std::pair<std::string, std::string>> args;
  xgboost::ObjFunction * obj = xgboost::ObjFunction::Create("count:poisson", &lparam);

  obj->Configure(args);

  // test label validation
  EXPECT_ANY_THROW(CheckObjFunction(obj, {0}, {-1}, {1}, {0}, {0}))
    << "Expected error when label < 0 for PoissonRegression";

  // test ProbToMargin
  EXPECT_NEAR(obj->ProbToMargin(0.1f), -2.30f, 0.01f);
  EXPECT_NEAR(obj->ProbToMargin(0.5f), -0.69f, 0.01f);
  EXPECT_NEAR(obj->ProbToMargin(0.9f), -0.10f, 0.01f);

  // test PredTransform
  xgboost::HostDeviceVector<xgboost::bst_float> io_preds = {0, 0.1f, 0.5f, 0.9f, 1};
  std::vector<xgboost::bst_float> out_preds = {1, 1.10f, 1.64f, 2.45f, 2.71f};
  obj->PredTransform(&io_preds);
  auto& preds = io_preds.HostVector();
  for (int i = 0; i < static_cast<int>(io_preds.Size()); ++i) {
    EXPECT_NEAR(preds[i], out_preds[i], 0.01f);
  }

  delete obj;
}

TEST(Objective, DeclareUnifiedTest(GammaRegressionGPair)) {
  xgboost::LearnerTrainParam lparam = xgboost::CreateEmptyGenericParam(0, NGPUS);
  std::vector<std::pair<std::string, std::string>> args;
  xgboost::ObjFunction * obj = xgboost::ObjFunction::Create("reg:gamma", &lparam);

  obj->Configure(args);
  CheckObjFunction(obj,
                   {0, 0.1f, 0.9f, 1, 0,  0.1f,  0.9f,    1},
                   {0,   0,   0, 0, 1,    1,    1,    1},
                   {1,   1,   1, 1, 1,    1,    1,    1},
                   {1,   1,   1, 1, 0, 0.09f, 0.59f, 0.63f},
                   {0,   0,   0, 0, 1, 0.90f, 0.40f, 0.36f});
  CheckObjFunction(obj,
                   {0, 0.1f, 0.9f, 1, 0,  0.1f,  0.9f,    1},
                   {0,   0,   0, 0, 1,    1,    1,    1},
                   {},  // Empty weight
                   {1,   1,   1, 1, 0, 0.09f, 0.59f, 0.63f},
                   {0,   0,   0, 0, 1, 0.90f, 0.40f, 0.36f});
  delete obj;
}

TEST(Objective, DeclareUnifiedTest(GammaRegressionBasic)) {
  xgboost::LearnerTrainParam lparam = xgboost::CreateEmptyGenericParam(0, NGPUS);
  std::vector<std::pair<std::string, std::string>> args;
  xgboost::ObjFunction * obj = xgboost::ObjFunction::Create("reg:gamma", &lparam);

  obj->Configure(args);

  // test label validation
  EXPECT_ANY_THROW(CheckObjFunction(obj, {0}, {-1}, {1}, {0}, {0}))
    << "Expected error when label < 0 for GammaRegression";

  // test ProbToMargin
  EXPECT_NEAR(obj->ProbToMargin(0.1f), -2.30f, 0.01f);
  EXPECT_NEAR(obj->ProbToMargin(0.5f), -0.69f, 0.01f);
  EXPECT_NEAR(obj->ProbToMargin(0.9f), -0.10f, 0.01f);

  // test PredTransform
  xgboost::HostDeviceVector<xgboost::bst_float> io_preds = {0, 0.1f, 0.5f, 0.9f, 1};
  std::vector<xgboost::bst_float> out_preds = {1, 1.10f, 1.64f, 2.45f, 2.71f};
  obj->PredTransform(&io_preds);
  auto& preds = io_preds.HostVector();
  for (int i = 0; i < static_cast<int>(io_preds.Size()); ++i) {
    EXPECT_NEAR(preds[i], out_preds[i], 0.01f);
  }

  delete obj;
}

TEST(Objective, DeclareUnifiedTest(TweedieRegressionGPair)) {
  xgboost::LearnerTrainParam lparam = xgboost::CreateEmptyGenericParam(0, NGPUS);
  std::vector<std::pair<std::string, std::string>> args;
  xgboost::ObjFunction * obj = xgboost::ObjFunction::Create("reg:tweedie", &lparam);

  args.emplace_back(std::make_pair("tweedie_variance_power", "1.1f"));
  obj->Configure(args);
  CheckObjFunction(obj,
                   {   0,  0.1f,  0.9f,    1, 0,  0.1f,  0.9f,    1},
                   {   0,    0,    0,    0, 1,    1,    1,    1},
                   {   1,    1,    1,    1, 1,    1,    1,    1},
                   {   1, 1.09f, 2.24f, 2.45f, 0, 0.10f, 1.33f, 1.55f},
                   {0.89f, 0.98f, 2.02f, 2.21f, 1, 1.08f, 2.11f, 2.30f});
  CheckObjFunction(obj,
                   {   0,  0.1f,  0.9f,    1, 0,  0.1f,  0.9f,    1},
                   {   0,    0,    0,    0, 1,    1,    1,    1},
                   {},  // Empty weight.
                   {   1, 1.09f, 2.24f, 2.45f, 0, 0.10f, 1.33f, 1.55f},
                   {0.89f, 0.98f, 2.02f, 2.21f, 1, 1.08f, 2.11f, 2.30f});

  delete obj;
}

#if defined(__CUDACC__)
TEST(Objective, CPU_vs_CUDA) {
  xgboost::LearnerTrainParam lparam = xgboost::CreateEmptyGenericParam(0, 1);

  xgboost::ObjFunction * obj =
      xgboost::ObjFunction::Create("reg:squarederror", &lparam);
  xgboost::HostDeviceVector<xgboost::GradientPair> cpu_out_preds;
  xgboost::HostDeviceVector<xgboost::GradientPair> cuda_out_preds;

  constexpr size_t kRows = 400;
  constexpr size_t kCols = 100;
  auto ppdmat = xgboost::CreateDMatrix(kRows, kCols, 0, 0);
  xgboost::HostDeviceVector<float> preds;
  preds.Resize(kRows);
  auto& h_preds = preds.HostVector();
  for (size_t i = 0; i < h_preds.size(); ++i) {
    h_preds[i] = static_cast<float>(i);
  }
  auto& info = (*ppdmat)->Info();

  info.labels_.Resize(kRows);
  auto& h_labels = info.labels_.HostVector();
  for (size_t i = 0; i < h_labels.size(); ++i) {
    h_labels[i] = 1 / (float)(i+1);
  }

  {
    // CPU
    lparam.n_gpus = 0;
    obj->GetGradient(preds, info, 0, &cpu_out_preds);
  }
  {
    // CUDA
    lparam.n_gpus = 1;
    obj->GetGradient(preds, info, 0, &cuda_out_preds);
  }

  auto& h_cpu_out = cpu_out_preds.HostVector();
  auto& h_cuda_out = cuda_out_preds.HostVector();

  float sgrad = 0;
  float shess = 0;
  for (size_t i = 0; i < kRows; ++i) {
    sgrad += std::pow(h_cpu_out[i].GetGrad() - h_cuda_out[i].GetGrad(), 2);
    shess += std::pow(h_cpu_out[i].GetHess() - h_cuda_out[i].GetHess(), 2);
  }
  ASSERT_NEAR(sgrad, 0.0f, xgboost::kRtEps);
  ASSERT_NEAR(shess, 0.0f, xgboost::kRtEps);

  delete ppdmat;
  delete obj;
}
#endif

TEST(Objective, DeclareUnifiedTest(TweedieRegressionBasic)) {
  xgboost::LearnerTrainParam lparam = xgboost::CreateEmptyGenericParam(0, NGPUS);
  std::vector<std::pair<std::string, std::string>> args;
  xgboost::ObjFunction * obj = xgboost::ObjFunction::Create("reg:tweedie", &lparam);

  obj->Configure(args);

  // test label validation
  EXPECT_ANY_THROW(CheckObjFunction(obj, {0}, {-1}, {1}, {0}, {0}))
    << "Expected error when label < 0 for TweedieRegression";

  // test ProbToMargin
  EXPECT_NEAR(obj->ProbToMargin(0.1f), -2.30f, 0.01f);
  EXPECT_NEAR(obj->ProbToMargin(0.5f), -0.69f, 0.01f);
  EXPECT_NEAR(obj->ProbToMargin(0.9f), -0.10f, 0.01f);

  // test PredTransform
  xgboost::HostDeviceVector<xgboost::bst_float> io_preds = {0, 0.1f, 0.5f, 0.9f, 1};
  std::vector<xgboost::bst_float> out_preds = {1, 1.10f, 1.64f, 2.45f, 2.71f};
  obj->PredTransform(&io_preds);
  auto& preds = io_preds.HostVector();
  for (int i = 0; i < static_cast<int>(io_preds.Size()); ++i) {
    EXPECT_NEAR(preds[i], out_preds[i], 0.01f);
  }

  delete obj;
}


// CoxRegression not implemented in GPU code, no need for testing.
#if !defined(__CUDACC__)
TEST(Objective, CoxRegressionGPair) {
  xgboost::LearnerTrainParam lparam = xgboost::CreateEmptyGenericParam(0, 0);
  std::vector<std::pair<std::string, std::string>> args;
  xgboost::ObjFunction * obj =
      xgboost::ObjFunction::Create("survival:cox", &lparam);

  obj->Configure(args);
  CheckObjFunction(obj,
                   { 0, 0.1f, 0.9f,       1,       0,    0.1f,   0.9f,       1},
                   { 0,   -2,   -2,       2,       3,       5,    -10,     100},
                   { 1,    1,    1,       1,       1,       1,      1,       1},
                   { 0,    0,    0, -0.799f, -0.788f, -0.590f, 0.910f,  1.006f},
                   { 0,    0,    0,  0.160f,  0.186f,  0.348f, 0.610f,  0.639f});

  delete obj;
}
#endif
