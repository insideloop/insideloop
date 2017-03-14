//==============================================================================
//
//                                  InsideLoop
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.txt for details.
//
//==============================================================================

#ifndef IL_STATUS_H
#define IL_STATUS_H

#include <il/Info.h>

namespace il {

enum class ErrorDomain : unsigned short { filesystem = 0, parse = 1 };

enum class Error : unsigned short {
  filesystem_file_not_found = 0 * 256 + 0,
  filesystem_directory_not_found = 0 * 256 + 1,
  filesystem_no_read_access = 0 * 256 + 2,
  filesystem_no_write_access = 0 * 256 + 3,
  filesystem_cannot_close_file = 0 * 256 + 4,
  binary_file_wrong_format = 4 * 256 + 0,
  binary_file_wrong_type = 4 * 256 + 1,
  binary_file_wrong_rank = 4 * 256 + 2,
  binary_file_wrong_endianness = 4 * 256 + 3,
  parse_bool = 1 * 256 + 0,
  parse_number = 1 * 256 + 11,
  parse_int = 1 * 256 + 1,
  parse_int_overflow = 1 * 256 + 2,
  parse_integer = 1 * 256 + 3,
  parse_integer_overflow = 1 * 256 + 4,
  parse_float = 1 * 256 + 5,
  parse_double = 1 * 256 + 6,
  parse_string = 1 * 256 + 7,
  parse_unclosed_array = 1 * 256 + 8,
  parse_unidentified_trailing_character = 1 * 256 + 9,
  parse_cannot_determine_type = 1 * 256 + 10,
  parse_heterogeneous_array = 1 * 256 + 12,
  parse_array = 1 * 256 + 13,
  parse_table = 1 * 256 + 14,
  parse_duplicate_key = 1 * 256 + 14,
  parse_key = 1 * 256 + 15,
  parse_value = 1 * 256 + 16,
  overflow_int = 2 * 256 + 0,
  overflow_integer = 2 * 256 + 1,
  floating_point_nonnegative = 3 * 256 + 0,
  floating_point_positive = 3 * 256 + 1,
  floating_point_nonpositive = 3 * 256 + 2,
  floating_point_negative = 3 * 256 + 3,
  matrix_singular = 5 * 256 + 0,
  matrix_eigenvalue_no_convergence = 5 * 256 + 1,
  unimplemented = 126 * 256 + 0,
  undefined = 127 * 256 + 0
};

inline il::ErrorDomain domain(il::Error error) {
  return static_cast<il::ErrorDomain>(static_cast<short>(error) >> 8);
}

class Status {
 private:
  bool ok_;
  bool to_check_;
  Error error_;

 public:
  il::Info info;

 public:
  Status();
  Status(const Status& other) = delete;
  Status(Status&& other);
  Status& operator=(const Status& other) = delete;
  Status& operator=(Status&& other);
  ~Status();
  void set(il::Error error);
  void set_ok();
  void rearm();
  bool ok();
  void abort_on_error();
  void ignore_error();
  il::Error error() const;
  il::ErrorDomain domain() const;
};

inline Status::Status() : info{} {
  ok_ = true;
  to_check_ = false;
  error_ = il::Error::undefined;
}

inline Status::Status(Status&& other) {
  ok_ = other.ok_;
  to_check_ = true;
  error_ = other.error_;
  info = other.info;

  other.to_check_ = false;
}

inline Status& Status::operator=(Status&& other) {
  IL_EXPECT_MEDIUM(!to_check_);

  ok_ = other.ok_;
  to_check_ = true;
  error_ = other.error_;
  info = other.info;

  other.to_check_ = false;

  return *this;
}

inline Status::~Status() { IL_EXPECT_MEDIUM(!to_check_); }

inline void Status::set(il::Error error) {
  IL_EXPECT_MEDIUM(!to_check_);

  ok_ = false;
  to_check_ = true;
  error_ = error;
  info.clear();
}

inline void Status::set_ok() {
  IL_EXPECT_MEDIUM(!to_check_);

  ok_ = true;
  to_check_ = true;
  error_ = il::Error::undefined;
  info.clear();
}

inline void Status::rearm() {
  IL_EXPECT_MEDIUM(!to_check_);

  to_check_ = true;
}

inline bool Status::ok() {
  IL_EXPECT_MEDIUM(to_check_);

  to_check_ = false;
  return ok_;
}

inline void Status::abort_on_error() {
  IL_EXPECT_MEDIUM(to_check_);

  to_check_ = false;
  if (!ok_) {
    il::abort();
  }
}

inline void Status::ignore_error() {
  IL_EXPECT_MEDIUM(to_check_);

  to_check_ = false;
}

inline il::Error Status::error() const {
  IL_EXPECT_MEDIUM(!to_check_);

  return error_;
}
}

#endif  // IL_STATUS_H
