/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2014  University of Bremen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <z3++.h>

#include <core/utils/program_options.hpp>

namespace cirkit {

void check_error( const z3::context& ctx )
{
  const Z3_error_code e = Z3_get_error_code( ctx );
  if ( e != Z3_OK )
  {
    std::cerr << Z3_get_error_msg_ex( ctx, e ) << std::endl;
    exit(1);
  }
}

enum pp_style
{
  Z3        /* use Z3 specific commands */
, SMTLIB2   /* use only standard SMT-LIB2 */
}; // pp_style

/*
 * SMT-LIB2 pretty printer based on
 * http://z3.codeplex.com/SourceControl/latest#examples/tptp/tptp5.cpp
 * (branch: unstable)
 */
class pp_smt2
{
public:
  pp_smt2( z3::context& ctx, const pp_style style = SMTLIB2 );

  void collect_decls( const z3::expr& e );
  void collect_sort( const z3::sort& s );
  void collect_fun( const z3::func_decl& f );

  void display_sort_decls( std::ostream& os ) const;
  void display_func_decls( std::ostream& os ) const;

private:
  const bool contains_id( const unsigned id ) const;
  void display_sort( std::ostream& os, const z3::sort& s ) const;
  void display_sort_decl( std::ostream& os, const z3::sort& s ) const;
  void display_func_decl( std::ostream& os, const z3::func_decl& f ) const;

  z3::context& ctx;
  const pp_style style;
  std::vector< z3::expr > todo;
  std::vector< z3::sort > sorts;
  std::vector< z3::func_decl > funs;
  std::set< unsigned > seen_ids;
}; // pp_smt2

pp_smt2::pp_smt2( z3::context& ctx, const pp_style style )
  : ctx(ctx)
  , style(style) {}

void pp_smt2::collect_decls( const z3::expr& e )
{
  todo.push_back( e );
  while ( !todo.empty() )
  {
    z3::expr e = todo.back();
    todo.pop_back();
    const unsigned id = Z3_get_ast_id( ctx, e );
    if ( contains_id( id ) )
    {
      continue;
    }
    seen_ids.insert( id );
    if ( e.is_app() )
    {
      collect_fun( e.decl() );
      const unsigned size = e.num_args();
      for ( unsigned i = 0u; i < size; ++i )
      {
        todo.push_back( e.arg(i) );
      }
    }
    else if ( e.is_quantifier() )
    {
      const unsigned nb = Z3_get_quantifier_num_bound( e.ctx(), e );
      for ( unsigned i = 0u; i < nb; ++i )
      {
        z3::sort srt( ctx, Z3_get_quantifier_bound_sort( e.ctx(), e, i ) );
        collect_sort( srt );
      }
      todo.push_back( e.body() );
    }
    else if ( e.is_var() )
    {
      collect_sort( e.get_sort() );
    }
  }
}

void pp_smt2::collect_sort( const z3::sort& s )
{
  const unsigned id = Z3_get_sort_id( ctx, s );
  if ( s.sort_kind() == Z3_UNINTERPRETED_SORT &&
       !contains_id(id) )
  {
    seen_ids.insert( id );
    sorts.push_back( s );
  }
}

void pp_smt2::collect_fun( const z3::func_decl& f )
{
  const unsigned id = Z3_get_func_decl_id( ctx, f );
  if ( contains_id(id ) )
  {
    return;
  }
  seen_ids.insert( id );
  if ( f.decl_kind() == Z3_OP_UNINTERPRETED )
  {
    funs.push_back( f );
  }
  for ( unsigned i = 0u; i < f.arity(); ++i )
  {
    collect_sort( f.domain(i) );
  }
  collect_sort( f.range() );
}

const bool pp_smt2::contains_id( const unsigned id ) const
{
  return ( seen_ids.find(id) != seen_ids.end() );
}

void pp_smt2::display_sort( std::ostream& os, const z3::sort& s ) const
{
  os << s;
}

void pp_smt2::display_sort_decls( std::ostream& os ) const
{
  for ( unsigned i = 0u; i < sorts.size(); ++i )
  {
    display_sort_decl( os, sorts[i] );
  }
}

void pp_smt2::display_sort_decl( std::ostream& os, const z3::sort& s ) const
{
  os << "(declare-sort " << s << ")" << std::endl;
}

void pp_smt2::display_func_decls( std::ostream& os ) const
{
  for ( std::size_t i = 0u; i < funs.size(); ++i )
  {
    display_func_decl( os, funs[i] );
  }
}

void pp_smt2::display_func_decl( std::ostream& os, const z3::func_decl& f ) const
{
  const std::string name = f.name().str();
  if ( f.is_const() )
  {
    if ( style == Z3 )
    {
      os << "(declare-const " << name << ' ';
    }
    else
    {
      os << "(declare-fun " << name << " () ";
    }
    z3::sort srt( f.range() );
    display_sort( os, srt );
    os << ")" << std::endl;
    return;
  }

  os << "(declare-fun " << name << " (";

  const unsigned na = f.arity();
  switch ( na )
  {
  case 0:
    break;
  case 1:
    {
      z3::sort s( f.domain(0) );
      display_sort( os, s );
      break;
    }
  default:
    {
      for ( unsigned j = 0u; j < na; ++j )
      {
        z3::sort s( f.domain( j ) );
        display_sort( os, s );
        if ( j + 1 < na )
        {
          os << ' ';
        }
      }
    }
  }
  os << ") ";

  z3::sort srt( f.range() );
  display_sort( os, srt );
  os << ")" << std::endl;
}


void display_smt2( std::ostream& os, const std::string& filename, const pp_style style, const bool simplify = false )
{
  z3::context ctx;

  const Z3_ast ast = Z3_parse_smtlib2_file(ctx, filename.c_str(), 0, 0, 0, 0, 0, 0);
  check_error( ctx );

  z3::expr formula( ctx, ast );
  if ( simplify ) {
    formula = formula.simplify();
  }

  pp_smt2 pp( ctx, style );
  pp.collect_decls( formula );
  pp.display_sort_decls( os );
  pp.display_func_decls( os );

  if ( formula.decl().decl_kind() == Z3_OP_AND )
  {
    for ( unsigned i = 0u; i < formula.num_args(); ++i )
    {
      os << "(assert " << formula.arg( i ) << ")" << std::endl;
    }
  }
  else
  {
    os << "(assert " << formula << ")" << std::endl;
  }
  os << "(check-sat)" << '\n';
}

}

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string filename;

  program_options opts;
  opts.add_options()
    ( "filename",   value<std::string>( &filename ),   "SMT-LIB2 file" )
    ( "simplify,s",                                    "Simplify instance" )
    ( "no-rewrite,n",                                  "Disable instance rewriting" )
    ( "allow-z3,z",                                    "Allow non-standard Z3 commands" )
    ;
  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "filename" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  if ( opts.is_set( "no-rewrite" ) ) {
    z3::set_param("pp.min_alias_size", 1000000);
    z3::set_param("pp.max_depth", 1000000);
  }

  pp_style style;
  if ( opts.is_set( "allow-z3" ) )
  {
    style = Z3;
  }
  else
  {
    style = SMTLIB2;
  }

  display_smt2( std::cout, filename, style, opts.is_set( "simplify" ) );

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
