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
