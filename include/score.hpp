#pragma once

#include <compare>
#include <cstdint>

// Sign is Positive && Mating Bit Set = Friendly player has opponent in mating
// net (value = encoded plys to mate, if 0 plys = checkmate) Sign is Negative &&
// Mating Bit Set = Friendly player is in an opponent's mating net (value =
// encoded plys to mate, if 0 plys = checkmate) Sign is Positive && Mating Bit
// Unset = Friendly player has advantage (value > 0) or is drawing (value = 0).
// Sign is Negative && Mating Bit Unset = Friendly player has disadvantage
// (value > 0) or is drawing (value = 0).

constexpr int16_t MAXIMUM_PLYS_TO_MATE = 128;

enum ESCORE : int16_t {
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

struct Score_Fields {
  uint16_t
      value : 14;  // Only using 13 bit values, 14th bit is in case of overflow.
  uint16_t mate : 1;
  uint16_t sign : 1;  // Needed in order to distinguish between friendly and
                      // enemy checkmates because plys to mate would equal zero.
};

class Score {
 public:
  Score();
  Score(int16_t evaluation);
  Score(Score_Fields fields);

  int16_t to_int() const;
  static Score from_int(int16_t i);

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
