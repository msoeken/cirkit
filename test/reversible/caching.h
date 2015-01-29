/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2015  University of Bremen
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

#include <fstream>
#include <future>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>

namespace cirkit {
namespace test {

template<typename Res, typename _Fn, typename _Rep, typename _Period>
boost::optional<Res> timeout(_Fn&& __fn, const std::chrono::duration<_Rep, _Period>& __rel)
{
  std::packaged_task<Res()> task(__fn);
  std::future<Res> future = task.get_future();
  std::thread(std::move(task)).detach();
  return ( future.valid() && future.wait_for(__rel) == std::future_status::ready )
           ? future.get()
           : boost::optional<Res>();
}

void enable_caching( const std::string& basket )
{
  using namespace boost::filesystem;
  using boost::format;
  using boost::str;

  create_directories( str( format( "logs/%s" ) % basket ) );
}

template<typename Res, typename _Fn, typename _Rep, typename _Period>
boost::optional<Res> compute_and_cache(_Fn&& __fn, const std::chrono::duration<_Rep, _Period>& __rel, const std::string& basket, const std::string& id, bool write_timeout = false )
{
  using namespace boost::filesystem;
  using boost::format;
  using boost::str;

  path p( str( format( "logs/%s/%s.log" ) % basket % id ) );

  if ( exists( p ) )
  {
    std::ifstream is( p.string() );
    std::string result;
    is >> result;
    is.close();

    if ( result == "TO" ) return boost::optional<Res>();
    else                  return boost::lexical_cast<Res>( result );
  }
  else
  {
    boost::optional<Res> result = timeout<Res>( __fn, __rel );

    if ( result || write_timeout )
    {
      std::filebuf fb;
      fb.open( p.string(), std::ios::out );
      std::ostream os( &fb );

      if ( result )
      {
        os << *result << std::endl;
      }
      else
      {
        os << "TO" << std::endl;
      }

      fb.close();
    }

    return result;
  }
}

template<typename Res, typename _Fn>
Res compute_and_cache(_Fn&& __fn, const std::string& basket, const std::string& id )
{
  using namespace boost::filesystem;
  using boost::format;
  using boost::str;

  path p( str( format( "logs/%s/%s.log" ) % basket % id ) );

  if ( exists( p ) )
  {
    std::ifstream is( p.string() );
    std::string result;
    is >> result;
    is.close();

    return boost::lexical_cast<Res>( result );
  }
  else
  {
    Res result = __fn();

    std::filebuf fb;
    fb.open( p.string(), std::ios::out );
    std::ostream os( &fb );
    os << result << std::endl;
    fb.close();

    return result;
  }
}

}
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
