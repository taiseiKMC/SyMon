#pragma once

#include <iostream>
#include <memory>

#include "non_symbolic_number_constraint.hh"
#include "non_symbolic_string_constraint.hh"

#include "symbolic_number_constraint.hh"
#include "symbolic_string_constraint.hh"
#include "timing_constraint.hh"

namespace NonSymbolic {
  // string atom
  static inline std::ostream &operator<<(std::ostream&, const NonSymbolic::StringAtom&);
  static inline std::istream &operator>>(std::istream&, NonSymbolic::StringAtom&);
  static inline std::istream &operator>>(std::istream &is, std::pair<VariableID, NonSymbolic::StringAtom> &update);
  static inline std::ostream &operator<<(std::ostream &os, const std::pair<VariableID, NonSymbolic::StringAtom> &update);

  // string constraints
  static inline std::istream &operator>>(std::istream &is, NonSymbolic::StringConstraint::kind_t &kind);
  static inline std::istream &operator>>(std::istream &is, NonSymbolic::StringConstraint &constraint);
  static inline std::ostream &operator<<(std::ostream &os, const NonSymbolic::StringConstraint::kind_t &kind);
  static inline std::ostream &operator<<(std::ostream &os, const NonSymbolic::StringConstraint &stringConstraint);
  static inline std::ostream &operator<<(std::ostream &os, const std::vector<NonSymbolic::StringConstraint> &stringConstraints);

  // number expressions / constraints (templates declared, implementations moved to .cc)
  template <typename Number>
  static inline std::ostream &operator<<(std::ostream &os, const NonSymbolic::NumberExpression<Number> &numberExpression);

  static inline std::ostream &operator<<(std::ostream &os, const NonSymbolic::NumberExpressionKind &kind);

  template <typename Number>
  static inline std::ostream &operator<<(std::ostream &os, const std::pair<VariableID, NonSymbolic::NumberExpression<Number>> &update);

  template<typename Number>
  static inline std::istream &operator>>(std::istream &is, std::pair<VariableID, NonSymbolic::NumberExpression<Number>> &update);

  template <typename Number>
  static inline std::ostream &operator<<(std::ostream &os, const std::vector<std::pair<VariableID, NonSymbolic::NumberExpression<Number>>> &updates);

  template <typename Number>
  static inline std::istream &operator>>(std::istream &is, NonSymbolic::NumberExpression<Number> &numberExpression);

  template <typename Number>
  static inline std::ostream &print(std::ostream &os, const typename NonSymbolic::NumberComparatorKind kind);

  template <typename Number>
  static inline std::ostream &operator<<(std::ostream &os, const typename NonSymbolic::NumberConstraint<Number>::kind_t kind);

  template <typename Number>
  static inline std::ostream &operator<<(std::ostream &os, const NonSymbolic::NumberConstraint<Number> &numberConstraint);

  template <typename Number>
  static inline std::istream &scan(std::istream &is, typename NonSymbolic::NumberComparatorKind &kind);

  template <typename Number>
  static inline std::istream &operator>>(std::istream &is, NonSymbolic::NumberConstraint<Number> &numberConstraint);

} // namespace NonSymbolic

namespace Symbolic {
  static inline std::ostream &operator<<(std::ostream &os, const Symbolic::StringAtom &atom);
  static inline std::istream &operator>>(std::istream &is, Symbolic::StringAtom &atom);
  static inline std::istream &operator>>(std::istream &is, std::pair<VariableID, Symbolic::StringAtom> &update);
  static inline std::ostream &operator<<(std::ostream &os, const std::pair<VariableID, Symbolic::StringAtom> &update);
  static inline std::istream &operator>>(std::istream &is, Symbolic::StringConstraint::kind_t &kind);
  static inline std::istream &operator>>(std::istream &is, Symbolic::StringConstraint &constraint);
  static inline std::ostream &operator<<(std::ostream &os, const Symbolic::StringConstraint::kind_t &kind);
  static inline std::ostream &operator<<(std::ostream &os, const Symbolic::StringConstraint &stringConstraint);
  static inline std::ostream &operator<<(std::ostream &os, const std::vector<Symbolic::StringConstraint> &stringConstraints);
} // namespace Symbolic

static inline std::ostream &operator<<(std::ostream &os, const std::pair<VariableID, VariableID> &update);
static inline std::istream &operator>>(std::istream &is, std::pair<VariableID, VariableID> &update);

static inline std::ostream &operator<<(std::ostream &os, const std::vector<std::pair<VariableID, VariableID>> &updates);

template <class Number>
static inline std::ostream &operator<<(std::ostream &os, const std::vector<NonSymbolic::NumberConstraint<Number>> &vector);

static inline std::istream &operator>>(std::istream &is, Symbolic::NumberExpression &numberExpression);

static inline std::istream &operator>>(std::istream &is, Symbolic::NumberConstraint &numberConstraint);

static inline std::ostream &operator<<(std::ostream &os, const std::pair<VariableID, Symbolic::NumberExpression> &update);
static inline std::istream &operator>>(std::istream &is, std::pair<VariableID, Symbolic::NumberExpression> &update);
static inline std::ostream &operator<<(std::ostream &os, const std::vector<std::pair<VariableID, Symbolic::NumberExpression>> &updates);

template <class T>
static inline std::istream &operator>>(std::istream &is, std::vector<T> &resetVars);
template <class T>
static inline std::ostream &operator<<(std::ostream &os, const std::vector<T> &guard);

static inline std::ostream &operator<<(std::ostream &os, const std::vector<Symbolic::NumberConstraint> &vector);
static inline std::istream &operator>>(std::istream &is, Symbolic::NumberExpression &numberExpression);
static inline std::istream &operator>>(std::istream &is, Symbolic::NumberConstraint &numberConstraint);

#include "io_operators.cc"
