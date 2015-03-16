#include "formal_equivalence_check.hpp"

#if defined(HAS_METASMT)

#include <metaSMT/frontend/Logic.hpp>

#include <iostream>

namespace cirkit {

std::vector < metaSMT::logic::predicate > allocate_inputs (
  unsigned lines
) {
  std::vector < metaSMT::logic::predicate > inputs ( lines );

  for ( auto i : boost::counting_range ( 0u, lines ) ) {
      inputs[i] = metaSMT::logic::new_variable ();
  }

  return inputs;
}
} // namespace cirkit

#endif
