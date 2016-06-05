/* alice: A C++ EDA command line interface API
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file command.hpp
 *
 * @brief CLI general command data structure
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#pragma once

#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <functional>
#include <locale>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/variant.hpp>

namespace po = boost::program_options;

namespace alice
{

/******************************************************************************
 * detail                                                                     *
 ******************************************************************************/

namespace detail
{

inline std::string json_escape( const std::string& s )
{
  std::stringstream ss;

  for ( size_t i = 0; i < s.length(); ++i )
  {
    if ( s[i] == '\\' || s[i] == '"' )
    {
      ss << "\\" << s[i];
    }
    else if ( unsigned( s[i] ) < '\x20' )
    {
      ss << "\\u" << std::setfill( '0' ) << std::setw( 4 ) << std::hex << unsigned( s[i] );
    }
    else
    {
      ss << s[i];
    }
  }

  return ss.str();
}

class log_var_visitor : public boost::static_visitor<void>
{
public:
  log_var_visitor( std::ostream& os ) : os( os ) {}

  void operator()( const std::string& s ) const
  {
    os << "\"" << json_escape( s ) << "\"";
  }

  void operator()( int i ) const
  {
    os << i;
  }

  void operator()( unsigned i ) const
  {
    os << i;
  }

  void operator()( double d ) const
  {
    os << d;
  }

  void operator()( bool b ) const
  {
    os << ( b ? "true" : "false" );
  }

  template<typename T>
  void operator()( const std::vector<T>& v ) const
  {
    os << "[";

    bool first = true;

    for ( const auto& element : v )
    {
      if ( !first )
      {
        os << ", ";
      }
      else
      {
        first = false;
      }

      operator()( element );
    }

    os << "]";
  }

private:
  std::ostream& os;
};

inline std::string make_caption( const std::string& caption, const std::string& publications )
{
  std::string c = caption;

  if ( !publications.empty() )
  {
    c += "\n\nBased on the following publication(s):\n" + publications;
  }

  return c;
}

using log_var_t = boost::variant<std::string, int, unsigned, double, bool, std::vector<std::string>, std::vector<int>, std::vector<unsigned>, std::vector<std::vector<int>>, std::vector<std::vector<unsigned>>>;
using log_map_t = std::unordered_map<std::string, log_var_t>;
using log_opt_t = boost::optional<log_map_t>;

}

/******************************************************************************
 * cli_store                                                                  *
 ******************************************************************************/

template<class T>
class cli_store
{
public:
  explicit cli_store( const std::string& name ) : _name( name ) {}

  inline T& current()
  {
    if ( _current < 0 )
    {
      throw boost::str( boost::format( "[e] no current %s available" ) % _name );
    }
    return _data[_current];
  }

  inline const T& current() const
  {
    if ( _current < 0 )
    {
      throw boost::str( boost::format( "[e] no current %s available" ) % _name );
    }
    return _data.at( _current );
  }

  inline T& operator*()
  {
    return current();
  }

  inline const T& operator*() const
  {
    return current();
  }

  inline T& operator[]( unsigned i )
  {
    return _data[i];
  }

  inline const T& operator[]( unsigned i ) const
  {
    return _data.at( i );
  }

  inline bool empty() const
  {
    return _data.empty();
  }

  inline const std::vector<T>& data() const
  {
    return _data;
  }

  inline typename std::vector<T>::size_type size() const
  {
    return _data.size();
  }

  inline int current_index() const
  {
    return _current;
  }

  inline void set_current_index( unsigned i )
  {
    _current = i;
  }

  void extend()
  {
    const auto s = _data.size();
    _data.resize( s + 1u );
    _current = s;
    current() = T();
  }

  void clear()
  {
    _data.clear();
    _current = -1;
  }

private:
  std::string    _name;
  std::vector<T> _data;
  int            _current = -1;
};

template<typename T>
struct store_info {};

/******************************************************************************
 * environment                                                                *
 ******************************************************************************/

class command;

class environment
{
public:
  using ptr = std::shared_ptr<environment>;

  template<typename T>
  void add_store( const std::string& key, const std::string& name )
  {
    stores.insert( {key, std::make_shared<cli_store<T>>( name )} );
  }

  template<typename T>
  cli_store<T>& store() const
  {
    constexpr auto key = store_info<T>::key;
    return *( boost::any_cast<std::shared_ptr<cli_store<T>>>( stores.at( key ) ) );
  }

  template<typename T>
  bool has_store() const
  {
    constexpr auto key = store_info<T>::key;
    return stores.find( key ) != stores.end();
  }

public: /* logging */
  void start_logging( const std::string& filename )
  {
    logger.open( filename.c_str(), std::ofstream::out );
    logger << "[";
  }

  void log_command( const std::shared_ptr<command>& cmd, const std::string& cmdstring, const std::chrono::system_clock::time_point& start );
  void log_command( const detail::log_opt_t& cmdlog, const std::string& cmdstring, const std::chrono::system_clock::time_point& start )
  {
    using boost::format;

    if ( !log_first_command )
    {
      logger << "," << std::endl;
    }
    else
    {
      log_first_command = false;
    }

    const auto start_c = std::chrono::system_clock::to_time_t( start );
    char timestr[20];
    std::strftime( timestr, sizeof( timestr ), "%F %T", std::localtime( &start_c ) );
    logger << format( "{\n"
                      "  \"command\": \"%s\",\n"
                      "  \"time\": \"%s\"" ) % detail::json_escape( cmdstring ) % timestr;

    if ( cmdlog != boost::none )
    {
      detail::log_var_visitor vis( logger );

      for ( const auto& p : *cmdlog )
      {
        logger << format( ",\n  \"%s\": " ) % p.first;
        boost::apply_visitor( vis, p.second );
      }
    }

    logger << "\n}";
  }

  void stop_logging()
  {
    logger << "]" << std::endl;
  }

public: /* variables */
  const std::string& variable_value( const std::string& key, const std::string& def ) const
  {
    const auto it = variables.find( key );
    return it == variables.end() ? def : it->second;
  }

public:
  std::map<std::string, boost::any>               stores;
  std::map<std::string, std::shared_ptr<command>> commands;
  std::map<std::string, std::vector<std::string>> categories;
  std::map<std::string, std::string>              variables;

  bool                                            log = false;
  bool                                            log_first_command = true;
  std::ofstream                                   logger;

  std::map<std::string, std::string>              aliases;

  bool                                            quit = false;
};

/******************************************************************************
 * command                                                                    *
 ******************************************************************************/

class command
{
public:
  using rule_t  = std::pair<std::function<bool()>, std::string>;
  using rules_t = std::vector<rule_t>;

  using log_var_t = detail::log_var_t;
  using log_map_t = detail::log_map_t;
  using log_opt_t = detail::log_opt_t;

  command( const environment::ptr& env, const std::string& caption, const std::string& publications = std::string() )
    : env( env ),
      scaption( caption ),
      opts( detail::make_caption( caption, publications ) )
  {
    opts.add_options()
      ( "help,h", "produce help message" )
      ;
  }

  inline const std::string& caption() const { return scaption; }

  virtual bool run( const std::vector<std::string>& args )
  {
    std::vector<char*> argv( args.size() );
    boost::transform( args, argv.begin(), []( const std::string& s ) { return const_cast<char*>( s.c_str() ); } );
    vm.clear();

    try
    {
      po::store( po::command_line_parser( args.size(), &argv[0] ).options( opts ).positional( pod ).run(), vm );
      po::notify( vm );
    }
    catch ( po::error& e )
    {
      std::cerr << "[e] " << e.what() << std::endl;
      return false;
    }

    if ( vm.count( "help" ) )
    {
      std::cout << opts << std::endl;
      return false;
    }

    for ( const auto& p : validity_rules() )
    {
      if ( !p.first() )
      {
        std::cerr << "[e] " << p.second << std::endl;
        return false;
      }
    }

    return execute();
  }

  inline bool is_set( const std::string& opt ) const { return vm.count( opt ); }

protected:
  virtual rules_t validity_rules() const { return {}; }
  virtual bool execute() = 0;

public:
  virtual log_opt_t log() const { return boost::none; }

protected:
  /* positional arguments */
  void add_positional_option( const std::string& option )
  {
    pod.add( option.c_str(), 1 );
  }

public:
  std::shared_ptr<environment> env;

protected:
  std::string                        scaption;

public:
  po::options_description            opts;
  po::variables_map                  vm;
  po::positional_options_description pod;
};

inline void environment::log_command( const std::shared_ptr<command>& cmd, const std::string& cmdstring, const std::chrono::system_clock::time_point& start )
{
  log_command( cmd->log(), cmdstring, start );
}

/******************************************************************************
 * customize stores                                                           *
 ******************************************************************************/

template<typename T>
std::string store_entry_to_string( const T& element )
{
  return "UNKNOWN";
}

template<typename T>
void print_store_entry( std::ostream& os, const T& element )
{
  os << "UNKNOWN" << std::endl;
}

template<typename T>
struct show_store_entry
{
  show_store_entry( command& cmd ) {}

  bool operator()( T& element, const std::string& dotname, const command& cmd )
  {
    std::cout << "[w] show is not supported for this store element" << std::endl;
    return false; /* don't open the dot file */
  }

  command::log_opt_t log() const
  {
    return boost::none;
  }
};

template<typename T>
void print_store_entry_statistics( std::ostream& os, const T& element )
{
  os << "UNKNOWN" << std::endl;
}

template<typename T>
command::log_opt_t log_store_entry_statistics( const T& element )
{
  return boost::none;
}

template<typename Source, typename Dest>
bool store_can_convert()
{
  return false;
}

template<typename Source, typename Dest>
Dest store_convert( const Source& src )
{
  assert( false );
}

template<typename T, typename Tag>
bool store_can_write_io_type( command& cmd )
{
  return false;
}

template<typename T, typename Tag>
void store_write_io_type( const T& element, const std::string& filename, const command& cmd )
{
  assert( false );
}

template<typename T, typename Tag>
bool store_can_read_io_type( command& cmd )
{
  return false;
}

template<typename T, typename Tag>
T store_read_io_type( const std::string& filename, const command& cmd )
{
  assert( false );
}

/* for the use in commands */
template<typename S>
int add_option_helper( po::options_description& opts )
{
  constexpr auto option   = store_info<S>::option;
  constexpr auto mnemonic = store_info<S>::mnemonic;

  if ( strlen( mnemonic ) == 1u )
  {
    opts.add_options()
      ( ( boost::format( "%s,%s" ) % option % mnemonic ).str().c_str(), store_info<S>::name_plural )
      ;
  }
  else
  {
    opts.add_options()
      ( ( boost::format( "%s" ) % option ).str().c_str(), store_info<S>::name_plural )
      ;
  }
  return 0;
}

template<typename T>
bool any_true_helper( std::initializer_list<T> list )
{
  for ( auto i : list )
  {
    if ( i ) { return true; }
  }

  return false;
}

template<typename T>
bool exactly_one_true_helper( std::initializer_list<T> list )
{
  auto current = false;

  for ( auto i : list )
  {
    if ( i )
    {
      if ( current ) { return false; }
      current = true;
    }
  }

  return current;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
