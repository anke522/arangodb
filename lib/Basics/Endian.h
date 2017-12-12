////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2017 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Jan Christoph Uhde
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_BASICS_ENDIAN_H
#define ARANGODB_BASICS_ENDIAN_H 1

#include <cstdint>
#include <cstring>

#ifdef __APPLE__
  #include <machine/endian.h>
  #include <libkern/OSByteOrder.h>
#elif _WIN32
#elif __linux__
  #include <endian.h>
#else
  #pragma messsage("unsupported os or compiler")
#endif

namespace arangodb {
namespace basics {

inline bool isLittleEndian(){
  int num = 42;
  static bool rv = ( *(char *)&num == 42 );
  return rv;
}

template<typename T> inline void ByteSwap (T& t) {
  uint8_t* ptr = (uint8_t*)(&t);
  size_t bytes = sizeof(T);
  for(size_t i = 0; i < bytes/2; i ++ ) {
    uint8_t swap = ptr[i];
    ptr[i] = ptr[bytes-i-1];
    ptr[bytes-i-1] = swap;
  }
}

template<typename T, size_t size> struct EndianTraits;

template<typename T> struct EndianTraits<T,2> {
  typedef typename std::make_unsigned<T>::type type;
  inline static type htole (type in) {
#ifdef __APPLE__
    return OSSwapHostToLittleInt16(in);
#elif __linux__
    return htole16(in);
#elif _WIN32
    if(!isLittleEndian()) { ByteSwap(in); }
#endif
  }
  inline static type letoh (type in) {
#ifdef __APPLE__
    return OSSwapLittleToHostInt16(in);
#elif __linux__
    return le16toh(in);
#elif _WIN32
    if(!isLittleEndian()) { ByteSwap(in); }
#endif
    return in;
  }
};

template<typename T> struct EndianTraits<T,4> {
  typedef typename std::make_unsigned<T>::type type;
  inline static type htole (type in) {
#ifdef __APPLE__
    return OSSwapHostToLittleInt32(in);
#elif __linux__
    return htole32(in);
#elif _WIN32
    if(!isLittleEndian()) { ByteSwap(in); }
#endif
    return in;
  }
  inline static type letoh (type in) {
#ifdef __APPLE__
    return OSSwapLittleToHostInt32(in);
#elif __linux__
    return le32toh(in);
#elif _WIN32
    if(!isLittleEndian()) { ByteSwap(in); }
#endif
    return in;
  }
};

template<typename T> struct EndianTraits<T,8> {
  typedef typename std::make_unsigned<T>::type type;
  inline static type htole (type in) {
#ifdef __APPLE__
    return OSSwapHostToLittleInt64(in);
#elif __linux__
    return htole64(in);
#elif _WIN32
    if(!isLittleEndian()) { ByteSwap(in); }
#endif
    return in;
  }
  inline static type letoh (type in) {
#ifdef __APPLE__
    return OSSwapLittleToHostInt64(in);
#elif __linux__
    return le64toh(in);
#elif _WIN32
    if(!isLittleEndian()) { ByteSwap(in); }
#endif
    return in;
  }
};

template<bool> struct cp {
  template<typename T> inline static T mu(T t) { return t; }
  template<typename T> inline static T ms(T t) { return t; }  
};
  
template<> struct cp<true> {
  // make unsigned
  template<typename T> inline static T mu(T t) {
    typename std::make_unsigned<T>::type tmp;
    std::memcpy(&tmp,&t,sizeof(T));
    return tmp;
  }
  // revert back to signed
  template<typename T> inline static T ms(T t) {
    typename std::make_signed<T>::type tmp;
    std::memcpy(&tmp,&t,sizeof(T));
    return tmp;
  }
};

// hostToLittle
template<typename T> inline T hostToLittle(T in) {
  return cp<std::is_signed<T>::value>::ms(
    EndianTraits<T,sizeof(T)>::htole(cp<std::is_signed<T>::value>::mu(in)));
}

// littleToHost
template<typename T> inline T littleToHost(T in) {
  return cp<std::is_signed<T>::value>::ms(
    EndianTraits<T,sizeof(T)>::letoh(cp<std::is_signed<T>::value>::mu(in)));
}

}}

#endif
