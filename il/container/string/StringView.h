//==============================================================================
//
//                                  InsideLoop
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.txt for details.
//
//==============================================================================

#ifndef IL_STRINGVIEW_H
#define IL_STRINGVIEW_H

// <cstring> is needed for memcpy
#include <cstring>

#include <il/base.h>

namespace il {

class ConstStringView {
 protected:
  unsigned char* data_;
  unsigned char* size_;

 public:
  ConstStringView();
  ConstStringView(const char* data);
  ConstStringView(const unsigned char* data);
  explicit ConstStringView(const char* data, il::int_t n);
  explicit ConstStringView(const unsigned char* data, il::int_t n);
  il::int_t size() const;
  bool is_ascii(il::int_t i) const;
  bool is_ascii(il::int_t i, char c) const;
  bool is_digit(il::int_t i) const;
  // A new line is a "\n" (Unix convention) or a "\r\n" // (Windows convention)
  bool is_newline(il::int_t i) const;

  bool last_is_ascii() const;
  bool last_is_ascii(char c) const;

  char to_ascii(il::int_t i) const;
  unsigned char to_code_unit(il::int_t i) const;
  int to_code_point(il::int_t i) const;

  il::int_t next_code_point(il::int_t i) const;

  void trim_prefix();  // remove the whitespace
  void trim_suffix();
  void remove_prefix(il::int_t i1);
  void remove_suffix(il::int_t i0);
  ConstStringView suffix(il::int_t i0) const;
  ConstStringView prefix(il::int_t i1) const;
  ConstStringView substring(il::int_t i0, il::int_t i1) const;

  bool is_empty() const;
  bool operator==(const char* string) const;
  const char* as_c_string() const;
  const unsigned char* begin() const;
  const unsigned char* end() const;
};

inline ConstStringView::ConstStringView() {
  data_ = nullptr;
  size_ = nullptr;
}

inline ConstStringView::ConstStringView(const unsigned char* data) {
  IL_EXPECT_AXIOM("data must be a null terminated string");

  il::int_t size = 0;
  while (data[size] != 0) {
    ++size;
  }
  data_ = const_cast<unsigned char*>(data);
  size_ = const_cast<unsigned char*>(data) + size;
}

inline ConstStringView::ConstStringView(const char* data)
    : ConstStringView{reinterpret_cast<const unsigned char*>(data)} {}

inline ConstStringView::ConstStringView(const unsigned char* data,
                                        il::int_t n) {
  IL_EXPECT_MEDIUM(n >= 0);

  data_ = const_cast<unsigned char*>(data);
  size_ = const_cast<unsigned char*>(data) + n;
}

inline ConstStringView::ConstStringView(const char* data, il::int_t n)
    : ConstStringView{reinterpret_cast<const unsigned char*>(data), n} {}

inline bool ConstStringView::is_ascii(il::int_t i) const {
  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i) <
                   static_cast<std::size_t>(size()));

  return data_[i] < 128;
}

inline bool ConstStringView::is_ascii(il::int_t i, char c) const {
  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i) <
                   static_cast<std::size_t>(size()));
  IL_EXPECT_MEDIUM(static_cast<unsigned char>(c) < 128);

  return data_[i] == c;
}

inline bool ConstStringView::is_digit(il::int_t i) const {
  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i) <
                   static_cast<std::size_t>(size()));

  return data_[i] >= static_cast<unsigned char>('0') &&
         data_[i] <= static_cast<unsigned char>('9');
}

inline bool ConstStringView::is_newline(il::int_t i) const {
  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i) <
                   static_cast<std::size_t>(size()));

  return data_[i] == '\n' ||
         (i + 1 < size() && data_[i] == '\r' && data_[i + 1] == '\n');
}

inline bool ConstStringView::last_is_ascii() const {
  IL_EXPECT_MEDIUM(size() > 0);

  return size_[-1] < 128;
}

inline bool ConstStringView::last_is_ascii(char c) const {
  IL_EXPECT_MEDIUM(size() > 0);
  IL_EXPECT_MEDIUM(static_cast<unsigned char>(c) < 128);

  return size_[-1] == c;
}

inline unsigned char ConstStringView::to_code_unit(il::int_t i) const {
  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i) <
                   static_cast<std::size_t>(size()));

  return data_[i];
}

inline int ConstStringView::to_code_point(il::int_t i) const {
  unsigned int ans = 0;
  const unsigned char* data = begin();
  if ((data[i] & 0x80u) == 0) {
    ans = static_cast<unsigned int>(data[i]);
  } else if ((data[i] & 0xE0u) == 0xC0u) {
    ans = (static_cast<unsigned int>(data[i] & 0x1Fu) << 6) +
          static_cast<unsigned int>(data[i + 1] & 0x3Fu);
  } else if ((data[i] & 0xF0u) == 0xE0u) {
    ans = (static_cast<unsigned int>(data[i] & 0x0Fu) << 12) +
          (static_cast<unsigned int>(data[i + 1] & 0x3Fu) << 6) +
          static_cast<unsigned int>(data[i + 2] & 0x3Fu);
  } else {
    ans = (static_cast<unsigned int>(data[i] & 0x07u) << 18) +
          (static_cast<unsigned int>(data[i + 1] & 0x3Fu) << 12) +
          (static_cast<unsigned int>(data[i + 2] & 0x3Fu) << 6) +
          static_cast<unsigned int>(data[i + 3] & 0x3Fu);
  }
  return static_cast<int>(ans);
}

inline char ConstStringView::to_ascii(il::int_t i) const {
  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i) <
                   static_cast<std::size_t>(size()));
  IL_EXPECT_MEDIUM(data_[i] < 128);

  return static_cast<char>(data_[i]);
}

// inline char ConstStringView::to_char_back() const {
//  return static_cast<char>(size_[-1]);
//}

// inline const unsigned char& ConstStringView::operator[](il::int_t i) const {
//
//  return data_[i];
//}
//
// inline const unsigned char& ConstStringView::back() const {
//  IL_EXPECT_MEDIUM(size() >= 1);
//
//  return size_[-1];
//}

inline il::int_t ConstStringView::size() const { return size_ - data_; }

inline il::int_t ConstStringView::next_code_point(il::int_t i) const {
  do {
    ++i;
  } while (i < size() && ((data_[i] & 0xC0u) == 0x80u));
  return i;
}

inline bool ConstStringView::is_empty() const { return size_ == data_; }

inline void ConstStringView::remove_prefix(il::int_t i1) {
  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i1) <=
                   static_cast<std::size_t>(size()));

  data_ += i1;
}

inline void ConstStringView::remove_suffix(il::int_t i) {
  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i) <=
                   static_cast<std::size_t>(size()));

  size_ = data_ + i;
}

inline ConstStringView ConstStringView::substring(il::int_t i0,
                                                  il::int_t i1) const {
  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i0) <=
                   static_cast<std::size_t>(size()));
  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i1) <=
                   static_cast<std::size_t>(size()));
  IL_EXPECT_MEDIUM(i0 <= i1);

  return ConstStringView{data_ + i0, i1 - i0};
}

inline ConstStringView ConstStringView::suffix(il::int_t i0) const {
  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i0) <=
                   static_cast<std::size_t>(size()));

  return ConstStringView{data_ + i0, size() - i0};
}

inline ConstStringView ConstStringView::prefix(il::int_t i1) const {
  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i1) <=
                   static_cast<std::size_t>(size()));

  return ConstStringView{data_, i1};
}

inline bool ConstStringView::operator==(const char* string) const {
  bool match = true;
  il::int_t k = 0;
  while (match && k < size() && string[k] != '\0') {
    if (data_[k] != static_cast<unsigned char>(string[k])) {
      match = false;
    }
    ++k;
  }
  return match;
}

inline const char* ConstStringView::as_c_string() const {
  return reinterpret_cast<const char*>(data_);
}

inline const unsigned char* ConstStringView::begin() const { return data_; }

inline const unsigned char* ConstStringView::end() const { return size_; }

class StringView : public ConstStringView {
 public:
  explicit StringView(unsigned char* data, il::int_t n);
  explicit StringView(char* data, il::int_t n);
  //  unsigned char& operator[](il::int_t i);
  StringView substring(il::int_t i0, il::int_t i1);
  StringView substring(il::int_t i0);
  unsigned char* begin();
  char* as_c_string();
};

inline StringView::StringView(unsigned char* data, il::int_t n) {
  IL_EXPECT_MEDIUM(n >= 0);

  data_ = data;
  size_ = data + n;
}

inline StringView::StringView(char* data, il::int_t n)
    : StringView{reinterpret_cast<unsigned char*>(data), n} {}

inline unsigned char* StringView::begin() { return data_; }

inline char* StringView::as_c_string() {
  return reinterpret_cast<char*>(data_);
}

// inline unsigned char& StringView::operator[](il::int_t i) {
//  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i) <
//      static_cast<std::size_t>(this->size()));
//
//  return this->data_[i];
//}

inline StringView StringView::substring(il::int_t i0, il::int_t i1) {
  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i0) <=
                   static_cast<std::size_t>(this->size()));
  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i1) <=
                   static_cast<std::size_t>(this->size()));
  IL_EXPECT_MEDIUM(i0 <= i1);

  return StringView{data_ + i0, i1 - i0};
}

inline StringView StringView::substring(il::int_t i0) {
  IL_EXPECT_MEDIUM(static_cast<std::size_t>(i0) <=
                   static_cast<std::size_t>(this->size()));

  return StringView{data_ + i0, this->size()};
}

}  // namespace il

#endif  // IL_STRINGVIEW_H
