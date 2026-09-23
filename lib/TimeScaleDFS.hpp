#ifndef timescaledfs_hpp__
#define timescaledfs_hpp__

#include "constants.hpp"
#include <array>
#include <cstdint>

static constexpr std::array<uint8_t, 7> convgr =
    // TCB, TDB, TCG, UT1, UTC, TAI, TT
    {
        0b0111010, // TT
        0b0001101, // TAI
        0b0001010, // UTC
        0b0000111, // UT1
        0b0000001, // TCG
        0b1000001, // TDB
        0b0100000, // TCB
}; // formater breaks table

template <Scale To>
uint64_t consteval conv_resolve_p(uint8_t From, uint8_t visited = 0) {
  constexpr uint8_t shift = 31;
  if (To == (Scale)From)
    return 0; // ULL << From;
  if (std::popcount(visited) > 4)
    return 1ULL << shift;
  uint64_t path = 1ULL << shift;
  visited = visited | (uint8_t)(1U << From);
  for (uint8_t i = 0; i < 7; i++) {
    uint64_t mask = 1ULL << i;
    if ((mask & visited) || !(mask & convgr[From]))
      continue;
    mask = conv_resolve_p<To>(i, visited);
    if (mask < path)
      path = mask;
  }
  if (path == (1ULL << shift))
    return 1ULL << shift;
  return (path << 8) + (1ULL << From);
}

/*template <Scale To, Scale From> struct b_conv_op {};

template <class A, class B> struct cat_tp;

template <class... A, class... B>
struct cat_tp<std::tuple<A...>, std::tuple<B...>> {
  using type = std::tuple<A..., B...>;
};

template <class Tp, class Tnext>
using cat_op = typename cat_tp<Tp, Tnext>::type;*/

template <Scale... Sc> struct op_seq {};

template <class T, Scale B> struct op_seq_add_s;

template <Scale B, Scale... A> struct op_seq_add_s<op_seq<A...>, B> {
  using type = op_seq<A..., B>;
};

template <class A, Scale B> using seq_add = typename op_seq_add_s<A, B>::type;

template <Scale To, Scale From, uint64_t path = 0>
auto consteval conv_resolve_op() {
  if constexpr (path == 0)
    return conv_resolve_op<To, From, conv_resolve_p<To>((uint8_t)From)>();
  else {
    constexpr Scale // Sto = (Scale)std::countr_zero(path >> 8),
        Sfrom = (Scale)std::countr_zero(path);
    if constexpr (path < (1ULL << 8))
      // return std::tuple<b_conv_op<To, Sfrom>>();
      return op_seq<To, Sfrom>{};
    else
      // return cat_op<std::tuple<b_conv_op<Sto, Sfrom>>,
      //              decltype(conv_resolve_op<To, Sfrom, (path >> 8)>())>();
      return seq_add<decltype(conv_resolve_op<To, Sfrom, (path >> 8)>()),
                     Sfrom>();
  }
}

#endif
