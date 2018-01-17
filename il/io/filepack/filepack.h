//==============================================================================
//
// Copyright 2017 The InsideLoop Authors. All Rights Reserved.
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

#ifndef IL_FILEPACK_H
#define IL_FILEPACK_H

#include <cstdio>

#include <il/Array.h>
#include <il/Map.h>
#include <il/MapArray.h>
#include <il/String.h>
#include <il/container/dynamic/Dynamic.h>
#include <il/io/io_base.h>

#ifdef IL_WINDOWS
#include <il/UTF16String.h>
#include <il/unicode.h>
#endif

namespace il {

inline il::int_t readVarint(il::io_t, il::int_t& k, std::FILE* file) {
  std::size_t ans = 0;
  std::size_t multiplier = 1;
  const std::size_t max_byte = 1 << 7;
  const unsigned char continuation_mask = 0x7F;
  const unsigned char continuation_byte = 0x80;
  unsigned char byte;
  do {
    std::fread(&byte, sizeof(unsigned char), 1, file);
    k += 1;
    ans += multiplier * (byte & continuation_mask);
    multiplier *= max_byte;
  } while ((byte & continuation_byte) == continuation_byte);

  return static_cast<il::int_t>(ans);
}

inline void writeVarint(il::int_t n, il::io_t, il::int_t& k, std::FILE* file) {
  std::size_t un = static_cast<std::size_t>(n);

  const std::size_t max_byte = 1 << 7;
  const unsigned char continuation_byte = 0x80;
  while (true) {
    unsigned char r = static_cast<unsigned char>(un % max_byte);
    un /= max_byte;
    if (un == 0) {
      std::fwrite(&r, sizeof(unsigned char), 1, file);
      k += 1;
      break;
    } else {
      r |= continuation_byte;
      std::fwrite(&r, sizeof(unsigned char), 1, file);
      k += 1;
    }
  }
}

inline void auxLoad(il::int_t n, il::io_t,
                    il::Map<il::String, il::Dynamic>& config, std::FILE* file) {
  IL_UNUSED(n);
  il::int_t k = 0;

  while (true) {
    il::int_t size_string = readVarint(il::io, k, file);
    if (size_string == 0) {
      break;
    }
    il::Array<char> raw_string{size_string - 1};
    std::fread(raw_string.data(), sizeof(char), size_string - 1, file);
    il::String string{il::StringType::Bytes, raw_string.data(),
                      size_string - 1};
    k += string.size() + 1;

    il::Type type;
    fread(&type, sizeof(il::Type), 1, file);
    k += sizeof(il::Type);

    switch (type) {
      case il::Type::TBool: {
        bool value = false;
        std::fread(&value, sizeof(bool), 1, file);
        k += sizeof(bool);
        config.insert(std::move(string), il::Dynamic{value});
      } break;
      case il::Type::TInteger: {
        const il::int_t value = readVarint(il::io, k, file);
        config.insert(std::move(string), il::Dynamic{value});
      } break;
      case il::Type::TFloat: {
        float value = 0.0f;
        std::fread(&value, sizeof(float), 1, file);
        k += sizeof(float);
        config.insert(std::move(string), il::Dynamic{value});
      } break;
      case il::Type::TDouble: {
        double value = 0.0;
        std::fread(&value, sizeof(double), 1, file);
        k += sizeof(double);
        config.insert(std::move(string), il::Dynamic{value});
      } break;
      case il::Type::TString: {
        il::int_t size = 0;
        std::fread(&size, sizeof(il::int_t), 1, file);
        k += sizeof(il::int_t);
        il::Array<char> raw_value{size + 1};
        std::fread(raw_value.data(), sizeof(char), size + 1, file);
        k += size + 1;
        il::String value{il::StringType::Bytes, raw_value.data(),
                         raw_value.size()};
        config.insert(std::move(string), il::Dynamic{std::move(value)});
      } break;
      case il::Type::TArrayOfDouble: {
        il::int_t size = 0;
        std::fread(&size, sizeof(il::int_t), 1, file);
        k += sizeof(il::int_t);
        il::Array<double> v{size};
        std::fread(v.data(), sizeof(double), size, file);
        k += sizeof(double) * size;
        config.insert(std::move(string), il::Dynamic{std::move(v)});
      } break;
      case il::Type::TArray2dOfUint8: {
        il::int_t size0 = 0;
        il::int_t size1 = 0;
        std::fread(&size0, sizeof(il::int_t), 1, file);
        std::fread(&size1, sizeof(il::int_t), 1, file);
        k += 2 * sizeof(il::int_t);
        il::Array2D<unsigned char> A{size0, size1};
        std::fread(A.data(), sizeof(double), size0 * size1, file);
        k += sizeof(unsigned char) * size0 * size1;
        config.insert(std::move(string), il::Dynamic{std::move(A)});
      } break;
      case il::Type::TArray2dOfDouble: {
        il::int_t size0 = 0;
        il::int_t size1 = 0;
        std::fread(&size0, sizeof(il::int_t), 1, file);
        std::fread(&size1, sizeof(il::int_t), 1, file);
        k += 2 * sizeof(il::int_t);
        il::Array2D<double> v{size0, size1};
        std::fread(v.data(), sizeof(double), size0 * size1, file);
        k += sizeof(double) * size0 * size1;
        config.insert(std::move(string), il::Dynamic{std::move(v)});
      } break;
      case il::Type::TArrayOfStruct: {
        il::int_t nb_types;
        il::readVarint(il::io, nb_types, file);

        il::Array<il::String> key{nb_types};
        il::Array<il::Type> type{nb_types};
        for (il::int_t i = 0; i < nb_types; ++i) {
          il::int_t key_size;
          il::readVarint(il::io, key_size, file);

          il::Array<char> raw_string{key_size};
          std::fread(raw_string.data(), sizeof(char), key_size, file);
          key[i] =
              il::String{il::StringType::Bytes, raw_string.data(), key_size};

          fread(&(type[i]), sizeof(il::Type), 1, file);
        }
        il::int_t n;
        std::fread(&n, sizeof(il::int_t), 1, file);

        il::Array<il::Dynamic> array{nb_types};
        for (il::int_t i = 0; i < nb_types; ++i) {
          switch (type[i]) {
            case il::Type::TDouble: {
              array[i] = il::Dynamic{il::Type::TArrayOfDouble};
              il::Array<double>& ref = array[i].asArrayOfDouble();
              ref.resize(n);
            } break;
            case il::Type::TInteger: {
              array[i] = il::Dynamic{il::Type::TArrayOfInt32};
              il::Array<int>& ref = array[i].asArrayOfInt32();
              ref.resize(n);
            } break;
            default:
              IL_UNREACHABLE;
          }
        }

        for (il::int_t j = 0; j < n; ++j) {
          for (il::int_t i = 0; i < nb_types; ++i) {
            switch (type[i]) {
              case il::Type::TDouble: {
                il::Array<double>& ref = array[i].asArrayOfDouble();
                double value = 0.0;
                std::fread(&value, sizeof(double), 1, file);
                ref[j] = value;
              } break;
              case il::Type::TInteger: {
                il::Array<int>& ref = array[i].asArrayOfInt32();
                int value = 0;
                std::fread(&value, sizeof(int), 1, file);
                ref[j] = value;
              } break;
              default:
                IL_UNREACHABLE;
            }
          }
        }
      }
      case il::Type::TMapArray:
      case il::Type::TMap: {
        il::int_t n_map = 0;
        const il::int_t n = readVarint(il::io, n_map, file);
        il::int_t size = 0;
        il::int_t i = config.search(string);
        config.insert(std::move(string),
                      il::Dynamic{il::Map<il::String, il::Dynamic>{n}}, il::io,
                      i);
        if (!config.found(i)) {
          // Hello
        }
        il::Map<il::String, il::Dynamic>& config_inner =
            config.value(i).asMap();
        auxLoad(size, il::io, config_inner, file);
        k += size;
      } break;
      default:
        IL_UNREACHABLE;
    }
  }
}

inline void auxLoad(il::int_t n, il::io_t,
                    il::MapArray<il::String, il::Dynamic>& config,
                    std::FILE* file) {
  IL_UNUSED(n);
  il::int_t k = 0;

  while (true) {
    il::int_t size_string = readVarint(il::io, k, file);
    if (size_string == 0) {
      break;
    }
    il::Array<char> raw_string{size_string};
    std::fread(raw_string.data(), sizeof(char), size_string, file);
    il::String string{il::StringType::Bytes, raw_string.data(), size_string};
    k += string.size();

    il::Type type;
    fread(&type, sizeof(il::Type), 1, file);
    k += sizeof(il::Type);

    switch (type) {
      case il::Type::TBool: {
        bool value = false;
        std::fread(&value, sizeof(bool), 1, file);
        k += sizeof(bool);
        config.insert(std::move(string), il::Dynamic{value});
      } break;
      case il::Type::TInteger: {
        const il::int_t value = readVarint(il::io, k, file);
        config.insert(std::move(string), il::Dynamic{value});
      } break;
      case il::Type::TFloat: {
        float value = 0.0f;
        std::fread(&value, sizeof(float), 1, file);
        k += sizeof(float);
        config.insert(std::move(string), il::Dynamic{value});
      } break;
      case il::Type::TDouble: {
        double value = 0.0;
        std::fread(&value, sizeof(double), 1, file);
        k += sizeof(double);
        config.insert(std::move(string), il::Dynamic{value});
      } break;
      case il::Type::TString: {
        il::int_t size = 0;
        std::fread(&size, sizeof(il::int_t), 1, file);
        k += sizeof(il::int_t);
        il::Array<char> raw_value{size};
        std::fread(raw_value.data(), sizeof(char), size, file);
        k += size;
        il::String value{il::StringType::Bytes, raw_value.data(), size};
        config.insert(std::move(string), il::Dynamic{std::move(value)});
      } break;
      case il::Type::TArrayOfDouble: {
        il::int_t size = 0;
        std::fread(&size, sizeof(il::int_t), 1, file);
        k += sizeof(il::int_t);
        il::Array<double> v{size};
        std::fread(v.data(), sizeof(double), size, file);
        k += sizeof(double) * size;
        config.insert(std::move(string), il::Dynamic{std::move(v)});
      } break;
      case il::Type::TArray2dOfUint8: {
        il::int_t size0 = 0;
        il::int_t size1 = 0;
        std::fread(&size0, sizeof(il::int_t), 1, file);
        std::fread(&size1, sizeof(il::int_t), 1, file);
        k += 2 * sizeof(il::int_t);
        il::Array2D<unsigned char> A{size0, size1};
        std::fread(A.data(), sizeof(double), size0 * size1, file);
        k += sizeof(unsigned char) * size0 * size1;
        config.insert(std::move(string), il::Dynamic{std::move(A)});
      } break;
      case il::Type::TArray2dOfDouble: {
        il::int_t size0 = 0;
        il::int_t size1 = 0;
        std::fread(&size0, sizeof(il::int_t), 1, file);
        std::fread(&size1, sizeof(il::int_t), 1, file);
        k += 2 * sizeof(il::int_t);
        il::Array2D<double> v{size0, size1};
        std::fread(v.data(), sizeof(double), size0 * size1, file);
        k += sizeof(double) * size0 * size1;
        config.insert(std::move(string), il::Dynamic{std::move(v)});
      } break;
      case il::Type::TMapArray: {
        if (n == -1) {
          const il::int_t nb_types = il::readVarint(il::io, k, file);

          il::Array<il::String> key{nb_types};
          il::Array<il::Type> type{nb_types};
          for (il::int_t i = 0; i < nb_types; ++i) {
            const il::int_t key_size = il::readVarint(il::io, k, file);

            il::Array<char> raw_string{key_size};
            std::fread(raw_string.data(), sizeof(char), key_size, file);
            il::String my_string{il::StringType::Bytes, raw_string.data(),
                                 key_size};
            key[i] = my_string;

            const std::size_t error =
                fread(&(type[i]), sizeof(il::Type), 1, file);
            //            std::cout << "Here" << std::endl;
          }
          const il::int_t n = readVarint(il::io, k, file);
          //          const std::size_t error = std::fread(&n,
          //          sizeof(il::int_t), 1, file);

          il::Array<il::Dynamic> array{nb_types};
          for (il::int_t i = 0; i < nb_types; ++i) {
            switch (type[i]) {
              case il::Type::TDouble: {
                array[i] = il::Dynamic{il::Type::TArrayOfDouble};
                il::Array<double>& ref = array[i].asArrayOfDouble();
                ref.resize(n);
              } break;
              case il::Type::TInteger: {
                array[i] = il::Dynamic{il::Type::TArrayOfInt32};
                il::Array<int>& ref = array[i].asArrayOfInt32();
                ref.resize(n);
              } break;
              default:
                IL_UNREACHABLE;
            }
          }

          for (il::int_t j = 0; j < n; ++j) {
            for (il::int_t i = 0; i < nb_types; ++i) {
              switch (type[i]) {
                case il::Type::TDouble: {
                  il::Array<double>& ref = array[i].asArrayOfDouble();
                  double value = 0.0;
                  const std::size_t error =
                      std::fread(&value, sizeof(double), 1, file);
                  ref[j] = value;
                } break;
                case il::Type::TInteger: {
                  il::Array<int>& ref = array[i].asArrayOfInt32();
                  int value = 0;
                  const std::size_t error =
                      std::fread(&value, sizeof(int), 1, file);
                  ref[j] = value;
                } break;
                default:
                  IL_UNREACHABLE;
              }
            }
          }
          std::cout << "End" << std::endl;
        } else {
          il::int_t n_map = 0;
          const il::int_t n = readVarint(il::io, n_map, file);
          il::int_t size = 0;
          il::int_t i = config.search(string);
          config.insert(std::move(string),
                        il::Dynamic{il::MapArray<il::String, il::Dynamic>{n}},
                        il::io, i);
          if (!config.found(i)) {
          }
          il::MapArray<il::String, il::Dynamic>& config_inner =
              config.value(i).asMapArray();
          auxLoad(size, il::io, config_inner, file);
          k += size;
        }
      } break;
        /*
              case il::Type::TMap:
              case il::Type::TMapArray: {
                il::int_t n_map = 0;
                const il::int_t n = readVarint(il::io, n_map, file);
                il::int_t size = 0;
                il::int_t i = config.search(string);
                config.insert(std::move(string),
                              il::Dynamic{il::MapArray<il::String,
           il::Dynamic>{n}}, il::io, i); if (!config.found(i)) {
                }
                il::MapArray<il::String, il::Dynamic>& config_inner =
                    config.value(i).asMapArray();
                auxLoad(size, il::io, config_inner, file);
                k += size;
              } break;
        */
      default:
        IL_UNREACHABLE;
    }
  }
}

template <>
class LoadHelperData<il::MapArray<il::String, il::Dynamic>> {
 public:
  static il::MapArray<il::String, il::Dynamic> load(const il::String& filename,
                                                    il::io_t,
                                                    il::Status& status) {
    il::MapArray<il::String, il::Dynamic> ans{};

#ifdef IL_UNIX
    std::FILE* file = std::fopen(filename.asCString(), "rb");
    if (!file) {
      status.setError(il::Error::FilesystemFileNotFound);
      return ans;
    }
#else
    il::UTF16String filename_utf16 = il::toUtf16(filename);
    std::FILE* file;
    errno_t error_nb = _wfopen_s(&file, filename_utf16.asWString(), L"rb");
    if (error_nb != 0) {
      status.setError(il::Error::FilesystemFileNotFound);
      return ans;
    }
#endif

    auxLoad(-1, il::io, ans, file);

    const int error = std::fclose(file);
    if (error != 0) {
      status.setError(il::Error::FilesystemCanNotCloseFile);
      return ans;
    }

    status.setOk();
    return ans;
  }
};

template <>
class LoadHelperData<il::Map<il::String, il::Dynamic>> {
 public:
  static il::Map<il::String, il::Dynamic> load(const il::String& filename,
                                               il::io_t, il::Status& status) {
    il::Map<il::String, il::Dynamic> ans{};

#ifdef IL_UNIX
    std::FILE* file = std::fopen(filename.asCString(), "rb");
    if (!file) {
      status.setError(il::Error::FilesystemFileNotFound);
      return ans;
    }
#else
    il::UTF16String filename_utf16 = il::toUtf16(filename);
    std::FILE* file;
    errno_t error_nb = _wfopen_s(&file, filename_utf16.asWString(), L"rb");
    if (error_nb != 0) {
      status.setError(il::Error::FilesystemFileNotFound);
      return ans;
    }
#endif

    auxLoad(-1, il::io, ans, file);

    const int error = std::fclose(file);
    if (error != 0) {
      status.setError(il::Error::FilesystemCanNotCloseFile);
      return ans;
    }

    status.setOk();
    return ans;
  }
};

inline void auxSave(const il::MapArray<il::String, il::Dynamic>& data,
                    const il::ArrayView<il::String>& option, il::io_t,
                    il::int_t& n, std::FILE* file) {
  n = 0;

  for (il::int_t i = 0; i < data.size(); ++i) {
    il::String to_check{il::StringType::Bytes, data.key(i).asCString(),
                        data.key(i).size()};
    il::int_t my_size = data.key(i).size();
    IL_UNUSED(to_check);
    IL_UNUSED(my_size);

    const il::int_t string_size = data.key(i).size();
    writeVarint(string_size, il::io, n, file);
    std::fwrite(data.key(i).asCString(), sizeof(char), string_size, file);
    n += data.key(i).size();
    const il::Type type = data.value(i).type();
    std::fwrite(&type, sizeof(il::Type), 1, file);
    n += sizeof(il::Type);

    switch (data.value(i).type()) {
      case il::Type::TBool: {
        const bool value = data.value(i).toBool();
        std::fwrite(&value, sizeof(bool), 1, file);
        n += sizeof(bool);
      } break;
      case il::Type::TInteger: {
        writeVarint(data.value(i).toInteger(), il::io, n, file);
      } break;
      case il::Type::TFloat: {
        const float value = data.value(i).toFloat();
        std::fwrite(&value, sizeof(float), 1, file);
        n += sizeof(float);
      } break;
      case il::Type::TDouble: {
        const double value = data.value(i).toDouble();
        std::fwrite(&value, sizeof(double), 1, file);
        n += sizeof(double);
      } break;
      case il::Type::TString: {
        const il::int_t size = data.value(i).asString().size();
        std::fwrite(&size, sizeof(il::int_t), 1, file);
        std::fwrite(data.value(i).asString().asCString(), sizeof(char),
                    size + 1, file);
        n += sizeof(il::int_t) + size + 1;
      } break;
      case il::Type::TArrayOfDouble: {
        const il::int_t size = data.value(i).asArrayOfDouble().size();
        std::fwrite(&size, sizeof(il::int_t), 1, file);
        std::fwrite(data.value(i).asArrayOfDouble().data(), sizeof(double),
                    size, file);
        n += sizeof(il::int_t) + sizeof(double) * size;
      } break;
      case il::Type::TArray2dOfUint8: {
        const il::Array2D<unsigned char>& A = data.value(i).asArray2dOfUint8();
        const il::int_t size0 = A.size(0);
        const il::int_t size1 = A.size(1);
        std::fwrite(&size0, sizeof(il::int_t), 1, file);
        std::fwrite(&size1, sizeof(il::int_t), 1, file);
        for (int j = 0; j < size1; ++j) {
          std::fwrite(A.data() + j * A.capacity(0), sizeof(double), size0,
                      file);
        }
        n += 2 * sizeof(il::int_t) + sizeof(unsigned char) * size0 * size1;
      } break;
      case il::Type::TArray2dOfDouble: {
        const il::Array2D<double>& A = data.value(i).asArray2dOfDouble();
        const il::int_t size0 = A.size(0);
        const il::int_t size1 = A.size(1);
        std::fwrite(&size0, sizeof(il::int_t), 1, file);
        std::fwrite(&size1, sizeof(il::int_t), 1, file);
        for (int j = 0; j < size1; ++j) {
          std::fwrite(A.data() + j * A.capacity(0), sizeof(double), size0,
                      file);
        }
        n += 2 * sizeof(il::int_t) + sizeof(double) * size0 * size1;
      } break;
      case il::Type::TMapArray: {
        const bool save_as_array_of_struct =
            (option.size() == 1 && option[0] == data.key(i));
        if (save_as_array_of_struct) {
          const il::MapArray<il::String, il::Dynamic>& my_data =
              data.value(i).asMapArray();
          il::int_t k = 0;

          const il::int_t population_struct = my_data.nbElements();
          writeVarint(population_struct, il::io, k, file);

          il::int_t n;
          bool size_of_array_is_known = false;
          for (il::int_t i = my_data.first(); i < my_data.sentinel();
               i = my_data.next(i)) {
            const il::String& key = my_data.key(i);
            writeVarint(key.size(), il::io, k, file);
            std::fwrite(key.asCString(), sizeof(char), key.size(), file);
            const il::Type type = my_data.value(i).type();
            switch (type) {
              case il::Type::TArrayOfDouble: {
                if (!size_of_array_is_known) {
                  const il::Array<double>& ref =
                      my_data.value(i).asArrayOfDouble();
                  n = ref.size();
                  size_of_array_is_known = true;
                }
                const il::Type my_type = il::Type::TDouble;
                std::fwrite(&my_type, sizeof(il::Type), 1, file);
              } break;
              case il::Type::TArrayOfInt32: {
                if (!size_of_array_is_known) {
                  const il::Array<int>& ref = my_data.value(i).asArrayOfInt32();
                  n = ref.size();
                  size_of_array_is_known = true;
                }
                const il::Type my_type = il::Type::TInteger;
                std::fwrite(&my_type, sizeof(il::Type), 1, file);
              } break;
              default:
                IL_UNREACHABLE;
            }
          }

          writeVarint(n, il::io, k, file);
          for (il::int_t j = 0; j < n; ++j) {
            for (il::int_t i = my_data.first(); i != my_data.sentinel();
                 i = my_data.next(i)) {
              const il::Type type = my_data.value(i).type();
              switch (type) {
                case il::Type::TArrayOfDouble: {
                  const il::Array<double>& ref =
                      my_data.value(i).asArrayOfDouble();
                  const double value = ref[j];
                  std::fwrite(&value, sizeof(double), 1, file);
                } break;
                case il::Type::TArrayOfInt32: {
                  const il::Array<int>& ref = my_data.value(i).asArrayOfInt32();
                  std::fwrite(&ref[j], sizeof(int), 1, file);
                } break;
                default:
                  IL_UNREACHABLE;
              }
            }
          }
        } else {
          il::int_t n_map = 0;
          //        std::fwrite(&n_map, sizeof(il::int_t), 1, file);
          writeVarint(data.value(i).asMapArray().size(), il::io, n_map, file);
          il::ArrayView<il::String> new_option{};
          if (option.size() > 0 && option[0] == data.key(i)) {
            new_option = option.view(il::Range{1, option.size()});
          }
          auxSave(data.value(i).asMapArray(), new_option, il::io, n_map, file);
          //        std::fseek(file, -(n_map + sizeof(il::int_t)),
//          SEEK_CUR);
          //        std::fwrite(&n_map, sizeof(il::int_t), 1, file);
          //        std::fseek(file, n_map, SEEK_CUR);
        }
        std::cout << "End write" << std::endl;
      } break;
      default:
        IL_UNREACHABLE;
    }
  }
  writeVarint(0, il::io, n, file);
}

template <>
class SaveHelperData<il::MapArray<il::String, il::Dynamic>> {
 public:
  static void save(const il::MapArray<il::String, il::Dynamic>& data,
                   const il::String& filename, il::io_t, il::Status& status) {
#ifdef IL_UNIX
    std::FILE* file = std::fopen(filename.asCString(), "wb");
    if (!file) {
      status.setError(il::Error::FilesystemFileNotFound);
      return;
    }
#else
    il::UTF16String filename_utf16 = il::toUtf16(filename);
    std::FILE* file;
    errno_t error_nb = _wfopen_s(&file, filename_utf16.asWString(), L"wb");
    if (error_nb != 0) {
      status.setError(il::Error::FilesystemFileNotFound);
      return;
    }
#endif

    il::Array<il::String> option{};
    il::int_t n = 0;
    auxSave(data, option.view(), il::io, n, file);

    const int error = std::fclose(file);
    if (error != 0) {
      status.setError(il::Error::FilesystemCanNotCloseFile);
      return;
    }

    status.setOk();
    return;
  }
};

template <>
class SaveHelperDataWithOptions<il::MapArray<il::String, il::Dynamic>,
                                il::Array<il::String>> {
 public:
  static void save(const il::MapArray<il::String, il::Dynamic>& data,
                   const il::Array<il::String>& option,
                   const il::String& filename, il::io_t, il::Status& status) {
#ifdef IL_UNIX
    std::FILE* file = std::fopen(filename.asCString(), "wb");
    if (!file) {
      status.setError(il::Error::FilesystemFileNotFound);
      return;
    }
#else
    il::UTF16String filename_utf16 = il::toUtf16(filename);
    std::FILE* file;
    errno_t error_nb = _wfopen_s(&file, filename_utf16.asWString(), L"wb");
    if (error_nb != 0) {
      status.setError(il::Error::FilesystemFileNotFound);
      return;
    }
#endif

    il::int_t n = 0;
    auxSave(data, option.view(), il::io, n, file);

    const int error = std::fclose(file);
    if (error != 0) {
      status.setError(il::Error::FilesystemCanNotCloseFile);
      return;
    }

    status.setOk();
    return;
  }
};

}  // namespace il

#endif  // IL_FILEPACK_H
