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

#include "create_image.hpp"

#include <fstream>
#include <sstream>

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>

#include "../target_tags.hpp"

using namespace boost::assign;

namespace cirkit
{

  //// class: create_image_settings ////
  void create_image_settings::draw_before( std::ostream& os ) const
  {
    os << draw_before_text;
  }

  void create_image_settings::draw_in_between( std::ostream& os ) const
  {
    os << draw_in_between_text;
  }

  void create_image_settings::draw_after( std::ostream& os ) const
  {
    os << draw_after_text;
  }

  //// class: create_pstricks_settings ////

  create_pstricks_settings::create_pstricks_settings()
    : math_emph( true )
  {
    elem_width = 0.5;
    elem_height = 0.5;
    line_width = 0.01;
    control_radius = 0.1;
    target_radius = 0.2;
  }

  void create_pstricks_settings::draw_begin( std::ostream& os ) const
  {
    os << "\\begin{pspicture}(" << width << "," << height << ")" << std::endl;
  }

  void create_pstricks_settings::draw_line( std::ostream& os, float x1, float x2, float y ) const
  {
    os << boost::format( "\\psline[linewidth=%f](%f,%f)(%f,%f)" ) % line_width % x1 % y % x2 % y << std::endl;
  }

  void create_pstricks_settings::draw_input( std::ostream& os, float x, float y, const std::string& text, bool is_constant ) const
  {
    std::string input = text;
    if ( math_emph )
    {
      input = "$" + input + "$";
    }
    os << boost::format( "\\rput[r](%f,%f){%s%s}" ) % ( x - 0.1 ) % y % ( is_constant ? "\\color{red}" : "" ) % input << std::endl;
  }

  void create_pstricks_settings::draw_output( std::ostream& os, float x, float y, const std::string& text, bool is_garbage ) const
  {
    std::string output = text;
    if ( math_emph )
    {
      output = "$" + output + "$";
    }
    os << boost::format( "\\rput[l](%f,%f){%s}" ) % ( x + 0.1 ) % y % output << std::endl;
  }

  void create_pstricks_settings::draw_control( std::ostream& os, float x, float y, bool polarity ) const
  {
    os << boost::format( "\\pscircle%s(%f,%f){%f}" ) % ( polarity ? "*" : "" ) % x % y % control_radius << std::endl;
  }

  void create_pstricks_settings::draw_targets( std::ostream& os, float x, const std::vector<float>& ys, const boost::any& target_tag ) const
  {
    if ( is_type<toffoli_tag>( target_tag ) )
    {
      os << boost::format( "\\pscircle[linewidth=%1%](%2%,%3%){%4%}\\psline[linewidth=%1%](%2%,%5%)(%2%,%6%)" ) % line_width % x % ys.at( 0 ) % target_radius % ( ys.at( 0 ) - target_radius ) % ( ys.at( 0 ) + target_radius );
    }
    else if ( is_type<fredkin_tag>( target_tag ) )
    {
      for ( const auto& y : ys )
      {
        os << boost::format( "\\psline[linewidth=%1%](%2%,%3%)(%4%,%5%)\\psline[linewidth=%1%](%2%,%5%)(%4%,%3%)" )
          % line_width % ( x - control_radius ) % ( y - control_radius ) % ( x + control_radius ) % ( y + control_radius )
           << std::endl;
      }
    }
  }

  void create_pstricks_settings::draw_peres_frame( std::ostream& os, float x1, float y1, float x2, float y2 ) const
  {
    os << boost::format( "\\psframe[linewidth=%1%,linestyle=dashed](%2%,%3%)(%4%,%5%)" ) % line_width % x1 % y1 % x2 % y2 << std::endl;
  }

  void create_pstricks_settings::draw_gate_line( std::ostream& os, float x, float y1, float y2 ) const
  {
    os << boost::format( "\\psline[linewidth=%f](%f,%f)(%f,%f)" ) % line_width % x % y1 % x % y2 << std::endl;
  }

  void create_pstricks_settings::draw_end( std::ostream& os ) const
  {
    os << "\\end{pspicture}" << std::endl;
  }

  //// class: create_tikz_settings ////

  create_tikz_settings::create_tikz_settings()
    : math_emph( true )
  {
    elem_width = 0.5;
    elem_height = 0.5;
    line_width = 0.3;
    control_radius = 0.1;
    target_radius = 0.2;
  }

  void create_tikz_settings::draw_begin( std::ostream& os ) const
  {
    os << "\\begin{tikzpicture}" << std::endl;
  }

  void create_tikz_settings::draw_line( std::ostream& os, float x1, float x2, float y ) const
  {
    os << boost::format( "\\draw[line width=%.2f] (%.2f,%.2f) -- (%.2f,%.2f);" ) % line_width % x1 % y % x2 % y << std::endl;
  }

  void create_tikz_settings::draw_input( std::ostream& os, float x, float y, const std::string& text, bool is_constant ) const
  {
    std::string input = text;
    if ( math_emph )
    {
      input = "$" + input + "$";
    }
    os << boost::format( "\\draw (%.2f,%.2f) node [left] {%s%s};" ) % ( x - 0.1 ) % y % ( is_constant ? "\\color{red}" : "" ) % input << std::endl;
  }

  void create_tikz_settings::draw_output( std::ostream& os, float x, float y, const std::string& text, bool is_garbage ) const
  {
    std::string output = text;
    if ( math_emph )
    {
      output = "$" + output + "$";
    }
    os << boost::format( "\\draw (%.2f,%.2f) node [right] {%s};" ) % ( x + 0.1 ) % y % output << std::endl;
  }

  void create_tikz_settings::draw_control( std::ostream& os, float x, float y, bool polarity ) const
  {
    os << boost::format( "\\draw[fill%s] (%.2f,%.2f) circle (%.2f);" ) % ( polarity ? "" : "=white" ) % x % y % control_radius << std::endl;
  }

  void create_tikz_settings::draw_targets( std::ostream& os, float x, const std::vector<float>& ys, const boost::any& target_tag ) const
  {
    if ( is_type<toffoli_tag>( target_tag ) )
    {
      os << boost::format( "\\draw[line width=%1%] (%2%,%3%) circle (%4%) (%2%,%5%) -- (%2%,%6%);" ) % line_width % x % ys.at( 0 ) % target_radius % ( ys.at( 0 ) - target_radius ) % ( ys.at( 0 ) + target_radius ) << std::endl;
    }
    else if ( is_type<fredkin_tag>( target_tag ) )
    {
      for ( const auto& y : ys )
      {
        os << boost::format( "\\draw[line width=%1%] (%2%,%3%) -- ++(%5%,%5%) (%2%,%4%) -- ++(%5%,-%5%);" )
          % line_width % ( x - control_radius ) % ( y - control_radius ) % ( y + control_radius ) % ( 2 * control_radius )
           << std::endl;
      }
    }
  }

  void create_tikz_settings::draw_peres_frame( std::ostream& os, float x1, float y1, float x2, float y2 ) const
  {
    os << boost::format( "\\draw[line width=%1%,dashed] (%2%,%3%) rectangle (%4%,%5%);" ) % line_width % x1 % y1 % x2 % y2 << std::endl;
  }

  void create_tikz_settings::draw_gate_line( std::ostream& os, float x, float y1, float y2 ) const
  {
    os << boost::format( "\\draw[line width=%.2f] (%.2f,%.2f) -- (%.2f,%.2f);" ) % line_width % x % y1 % x % y2 << std::endl;
  }

  void create_tikz_settings::draw_end( std::ostream& os ) const
  {
    os << "\\end{tikzpicture}" << std::endl;
  }

  //// function: create_image ////

  void create_image( std::ostream& os, const circuit& circ, create_image_settings& settings )
  {
    if ( circ.lines() == 0 )
    {
      return;
    }

    unsigned peres_count = std::count_if( circ.begin(), circ.end(), [](const gate& g) { return is_peres( g ); } );

    settings.width = settings.elem_width * ( 2 + std::max( circ.num_gates(), 1u ) + peres_count );
    settings.height = settings.elem_height * circ.lines();

    settings.draw_begin( os );

    settings.draw_before( os );

    float x1 = settings.elem_width;
    float x2 = settings.width - settings.elem_width;

    float y = settings.elem_height / 2;

    for ( unsigned i = 0; i < circ.lines(); ++i )
    {
      settings.draw_line( os, x1, x2, settings.height - y );

      if ( settings.draw_io )
      {
        settings.draw_input( os, x1, settings.height - y, ( circ.inputs().size() > i ? circ.inputs().at( i ) : "" ), (bool)circ.constants().at( i ) );
        settings.draw_output( os, x2, settings.height - y, ( circ.outputs().size() > i ? circ.outputs().at( i ) : "" ), circ.garbage().at( i ) );
      }

      y += settings.elem_height;
    }

    settings.draw_in_between( os );

    float x = settings.elem_width * 3 / 2;

    for ( const auto& g : circ )
    {
      std::stringstream sstr;

      if ( !is_peres( g ) )
      {
        float ymin = settings.height;
        float ymax = 0;

        for ( const auto& v : g.controls() )
        {
          float y = settings.height - ( v.line() + 0.5 ) * settings.elem_height;
          ymin = std::min( ymin, y );
          ymax = std::max( ymax, y );
          settings.draw_control( sstr, x, y, v.polarity() );
        }

        std::vector<float> ys;
        boost::transform( g.targets(), std::back_inserter( ys ), [&settings]( unsigned target ) { return settings.height - ( target + 0.5 ) * settings.elem_height; } );
        ymin = std::min( ymin, *std::min_element( ys.begin(), ys.end() ) );
        ymax = std::max( ymax, *std::max_element( ys.begin(), ys.end() ) );
        settings.draw_targets( sstr, x, ys, g.type() );

        settings.draw_gate_line( os, x, ymin, ymax );

        x += settings.elem_width;
      }
      else
      {
        std::vector<float> yts;

        float y = settings.height - ( g.controls().front().line() + 0.5 ) * settings.elem_height;
        settings.draw_control( sstr, x, y, g.controls().front().polarity() );
        settings.draw_control( sstr, x + settings.elem_width, y, g.controls().front().polarity() );

        float yt1 = settings.height - ( g.targets().at( 0u ) + 0.5 ) * settings.elem_height;
        yts += yt1;
        settings.draw_control( sstr, x, yt1, true );
        settings.draw_targets( sstr, x + settings.elem_width, yts, toffoli_tag() );

        float yt2 = settings.height - ( g.targets().at( 1u ) + 0.5 ) * settings.elem_height;
        yts.clear();
        yts += yt2;
        settings.draw_targets( sstr, x, yts, toffoli_tag() );

        settings.draw_gate_line( os, x, std::min( std::min( y, yt1 ), yt2 ), std::max( std::max( y, yt1 ), yt2 ) );
        settings.draw_gate_line( os, x + settings.elem_width, std::min( y, yt1 ), std::max( y, yt1 ) );

        settings.draw_peres_frame( os, x - settings.elem_width / 2, std::min( std::min( y, yt1 ), yt2 ) - settings.elem_height / 2,
                                   x + settings.elem_width + settings.elem_width / 2, std::max( std::max( y, yt1 ), yt2 ) + settings.elem_height / 2 );

        x += 2 * settings.elem_width;
      }

      os << sstr.str();
    }

    settings.draw_after( os );

    settings.draw_end( os );
  }

  void create_image( const std::string& filename, const circuit& circ, create_image_settings& settings )
  {
    std::filebuf fb;
    fb.open( filename.c_str(), std::ios::out );
    std::ostream os( &fb );
    create_image( os, circ, settings );
    fb.close();
  }
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
