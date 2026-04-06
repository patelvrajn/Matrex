#pragma once

#include <compare>
#include <cstdint>

#include "fixed_point.hpp"

// Sign is Positive && Mating Bit Set = Friendly player has opponent in mating
// net (value = encoded plys to mate, if 0 plys = checkmate) Sign is Negative &&
// Mating Bit Set = Friendly player is in an opponent's mating net (value =
// encoded plys to mate, if 0 plys = checkmate) Sign is Positive && Mating Bit
// Unset = Friendly player has advantage (value > 0) or is drawing (value = 0).
// Sign is Negative && Mating Bit Unset = Friendly player has disadvantage
// (value > 0) or is drawing (value = 0).

constexpr Fixed_Point_Int_Storage_Type MAXIMUM_PLYS_TO_MATE = 128;

enum ESCORE : Fixed_Point_Int_Storage_Type {
  NEGATIVE_INFINITY = -8191,  // -(2^13 - 1); Evaluation should never be this.
  LOSING_MATE_MIN = NEGATIVE_INFINITY + 1,
  LOSING_MATE_MAX = LOSING_MATE_MIN + MAXIMUM_PLYS_TO_MATE,
  EVALUATION_MIN = LOSING_MATE_MAX + 1,  // Non-mating minimum evaluation
  DRAW = 0,
  POSITIVE_INFINITY = -NEGATIVE_INFINITY,  // Evaluation should never be this.
  WINNING_MATE_MAX = POSITIVE_INFINITY - 1,
  WINNING_MATE_MIN = WINNING_MATE_MAX - MAXIMUM_PLYS_TO_MATE,
  EVALUATION_MAX = WINNING_MATE_MIN - 1,  // Non-mating maximum evaluation
};

constexpr Fixed_Point_Int_Storage_Type FP_NEGATIVE_INFINITY =
    Matrex_FP_Int::from_integer(ESCORE::NEGATIVE_INFINITY).get_value();
constexpr Fixed_Point_Int_Storage_Type FP_POSITIVE_INFINITY =
    Matrex_FP_Int::from_integer(ESCORE::POSITIVE_INFINITY).get_value();

constexpr Fixed_Point_Int_Storage_Type FP_EVALUATION_MIN =
    Matrex_FP_Int::from_integer(ESCORE::EVALUATION_MIN).get_value();
constexpr Fixed_Point_Int_Storage_Type FP_EVALUATION_MAX =
    Matrex_FP_Int::from_integer(ESCORE::EVALUATION_MAX).get_value();

constexpr Fixed_Point_Int_Storage_Type FP_WINNING_MATE_MAX =
    Matrex_FP_Int::from_integer(ESCORE::WINNING_MATE_MAX).get_value();
constexpr Fixed_Point_Int_Storage_Type FP_WINNING_MATE_MIN =
    Matrex_FP_Int::from_integer(ESCORE::WINNING_MATE_MIN).get_value();

constexpr Fixed_Point_Int_Storage_Type FP_LOSING_MATE_MAX =
    Matrex_FP_Int::from_integer(ESCORE::LOSING_MATE_MAX).get_value();
constexpr Fixed_Point_Int_Storage_Type FP_LOSING_MATE_MIN =
    Matrex_FP_Int::from_integer(ESCORE::LOSING_MATE_MIN).get_value();

struct Score_Fields {
  // Only using 13 bit values needed to represent the integer portion of the
  // score, 14th bit is in case of overflow during addition of scores. Note: the
  // total number of bits here must be less than 32.
  uint32_t value : 14 + MATREX_FP_INT_FRACTIONAL_BITS;
  uint32_t mate : 1;
  uint32_t sign : 1;  // Needed in order to distinguish between friendly and
                      // enemy checkmates because plys to mate would equal zero.
};

class Score {
 public:
  Score();
  explicit Score(Fixed_Point_Int_Storage_Type evaluation);
  explicit Score(Matrex_FP_Int evaluation);
  Score(Score_Fields fields);

  Fixed_Point_Int_Storage_Type to_int() const;
  static Score from_int(Fixed_Point_Int_Storage_Type i);

  bool is_mating_score() const;
  uint16_t mate_in() const;
  bool is_friendly_mate() const;
  bool is_enemy_mate() const;
  bool is_checkmate() const;

  bool is_draw() const;

  inline auto operator<=>(const Score& other) const;

  Score operator+(const Score& other) const;
  Score operator-(const Score& other) const;
  Score operator-() const;

  Score& operator+=(const Score& other);
  Score& operator-=(const Score& other);

  Score_Fields m_fields;
};

inline auto Score::operator<=>(const Score& other) const {
  return (to_int() <=> other.to_int());
}
