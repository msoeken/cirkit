/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file spectral_canonization2.hpp
 *
 * @brief Spectral domain normalization
 *
 * @author D. Michael Miller (algorithm development)
 * @author Mathias Soeken (C++ implementation, integration into CirKit)
 * @since  2.3
 */

#ifndef SPECTRAL_CANONIZATION2_HPP
#define SPECTRAL_CANONIZATION2_HPP

#include <cstdint>
#include <vector>

#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

struct trans_t
{
  trans_t();
  trans_t( uint16_t kind, uint16_t var1 = 0, uint16_t var2 = 0 );

  union
  {
    struct
    {
      uint16_t kind : 4;
      uint16_t var1 : 6;
      uint16_t var2 : 6;
    };
    uint16_t data;
  };
};

class spectrum
{
public:
  explicit spectrum( const tt& func );
  spectrum( const spectrum& other );

  spectrum& operator=( const spectrum& other );

  tt to_function(); /* this will change the spectrum! */

  void apply( const trans_t& trans );
  trans_t trans1( unsigned i, unsigned j ); /* xi <-> xj */
  trans_t trans2( unsigned i );             /* xi <- !xi */
  trans_t trans3();                         /* f <- !f */
  trans_t trans4( unsigned i, unsigned j ); /* xi <- xi XOR xj */
  trans_t trans5( unsigned i );             /* f <- f XOR xi */

  inline std::vector<int>::reference operator[]( std::vector<int>::size_type pos )
  {
    return _s[pos];
  }

  inline std::vector<int>::const_reference operator[]( std::vector<int>::size_type pos ) const
  {
    return _s[pos];
  }

  inline std::vector<int>::size_type size() const
  {
    return _s.size();
  }

private:
  void fast_hadamard_transform( bool reverse = false );

private:
  std::vector<int> _s;
};

struct spectral_normalization_params
{
  bool verbose = false;
  bool verify  = false;
};

struct spectral_normalization_stats
{
  double runtime;
  std::vector<trans_t> transforms;
};

tt spectral_normalization( const tt& spec, const spectral_normalization_params& params, spectral_normalization_stats& stats );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
