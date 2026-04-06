#include "score.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

Score::Score() : m_fields{.value = 0, .mate = 0, .sign = 0} {}

Score::Score(Fixed_Point_Int_Storage_Type evaluation) {
  Score s = Score::from_int(evaluation);
  m_fields = s.m_fields;
}

Score::Score(Matrex_FP_Int evaluation) : Score(evaluation.get_value()) {}

Score::Score(Score_Fields fields) { m_fields = fields; }

Fixed_Point_Int_Storage_Type Score::to_int() const {
  if (m_fields.mate) {  // Mating evaluation.

    if (m_fields.sign) {  // Negative
      return FP_LOSING_MATE_MIN + m_fields.value;
    } else {  // Positive
      return FP_WINNING_MATE_MAX - m_fields.value;
    }

  } else {  // Normal evaluation.

    if (m_fields.sign) {  // Negative
      return -m_fields.value;
    } else {  // Positive
      return m_fields.value;
    }
  }
}

Score Score::from_int(Fixed_Point_Int_Storage_Type i) {
  Score return_score;
  return_score.m_fields.sign = (i < 0);

  const uint32_t abs_i = std::abs(i);

  if (i == FP_NEGATIVE_INFINITY) {
    return_score.m_fields.value = abs_i;
    return_score.m_fields.mate = 0;
    return return_score;
  }

  if (i == FP_POSITIVE_INFINITY) {
    return_score.m_fields.value = abs_i;
    return_score.m_fields.mate = 0;
    return return_score;
  }

  if (abs_i >= FP_WINNING_MATE_MIN) {  // Mating evaluation.

    return_score.m_fields.mate = 1;

    if (return_score.m_fields.sign) {  // i is negative

      // This gives the plys to mate because:
      //    i - FP_LOSING_MATE_MIN
      //    = (FP_LOSING_MATE_MIN + N) - FP_LOSING_MATE_MIN
      //    = N
      const uint32_t plys_to_mate = i - FP_LOSING_MATE_MIN;
      return_score.m_fields.value = plys_to_mate;

    } else {  // i is positive

      const uint32_t plys_to_mate =
          FP_WINNING_MATE_MAX - i;  // A higher i means closer to checkmate.
      return_score.m_fields.value = plys_to_mate;
    }

  } else {  // Normal evaluation.

    return_score.m_fields.mate = 0;
    return_score.m_fields.value = abs_i;
  }

  return return_score;
}

bool Score::is_mating_score() const { return m_fields.mate; }

uint16_t Score::mate_in() const {
  return m_fields.value;  // Assumes a check for is_mating_score is done.
}

bool Score::is_friendly_mate() const {
  return ((m_fields.mate == 1) && (m_fields.sign == 0));
}

bool Score::is_enemy_mate() const {
  return ((m_fields.mate == 1) && (m_fields.sign == 1));
}

bool Score::is_checkmate() const {
  return ((m_fields.mate == 1) && (m_fields.value == 0));
}

bool Score::is_draw() const {
  return ((m_fields.mate == 0) && (m_fields.value == 0));
}

Score Score::operator+(const Score& other) const {
  Fixed_Point_Int_Storage_Type this_int = to_int();
  Fixed_Point_Int_Storage_Type other_int = other.to_int();

  if (m_fields.mate || other.m_fields.mate) {
    return Score::from_int(std::max(this_int, other_int));
  } else {
    Fixed_Point_Int_Storage_Type sum = this_int + other_int;
    return Score(std::clamp(sum, FP_NEGATIVE_INFINITY, FP_POSITIVE_INFINITY));
  }
}

Score Score::operator-(const Score& other) const {
  Fixed_Point_Int_Storage_Type this_int = to_int();
  Fixed_Point_Int_Storage_Type other_int = other.to_int();

  if (m_fields.mate || other.m_fields.mate) {
    return Score::from_int(std::min(this_int, other_int));
  } else {
    Fixed_Point_Int_Storage_Type difference = this_int - other_int;
    return Score(
        std::clamp(difference, FP_NEGATIVE_INFINITY, FP_POSITIVE_INFINITY));
  }
}

Score Score::operator-() const { return Score::from_int(-to_int()); }

Score& Score::operator+=(const Score& other) {
  Score sum = (*this) + other;
  m_fields = sum.m_fields;

  return *this;
}

Score& Score::operator-=(const Score& other) {
  Score difference = (*this) - other;
  m_fields = difference.m_fields;

  return *this;
}
