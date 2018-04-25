//==============================================================================
//
// Copyright 2018 The InsideLoop Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//==============================================================================

#ifndef IL_BLAS_DEFINE_H
#define IL_BLAS_DEFINE_H

#ifdef IL_MKL

#include <mkl_cblas.h>
#define IL_CBLAS_INT MKL_INT
#define IL_CBLAS_LAYOUT CBLAS_LAYOUT

#elif IL_OPENBLAS

#include <OpenBLAS/cblas.h>
#define IL_CBLAS_INT int
#define IL_CBLAS_LAYOUT CBLAS_ORDER

#endif

#endif  // IL_BLAS_DEFINE_H

