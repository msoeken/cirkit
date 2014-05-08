template<class... Arguments>
class benchmark_table
{
public:
  typedef boost::tuple<Arguments...> benchmark;

  benchmark_table( std::initializer_list<std::string> column_names )
  {
    using boost::adaptors::transformed;
    boost::push_back( _column_names, column_names );
    boost::push_back( lengths, _column_names | transformed( []( const std::string& s ) { return s.size(); } ) );
  }

  template<typename T>
  void add_length( const T& value )
  {
    unsigned l = boost::lexical_cast<std::string>( value ).size();
    if ( l > lengths[length_i] )
    {
      lengths[length_i] = l;
    }
    ++length_i;
  }

  template<typename T>
  void compute_lengths(const T& value)
  {
    add_length( value );
  }

  template<typename U, typename... T>
  void compute_lengths(const U& head, const T&... tail)
  {
    add_length( head );
    compute_lengths(tail...);
  }

  template<class... Args>
  void add(Args&&... args)
  {
    results.push_back(boost::make_tuple(std::forward<Args>(args)...));
    length_i = 0u;
    compute_lengths( args... );
  }

  void print() const
  {
    using namespace boost::spirit::karma;

    for ( unsigned n : boost::irange( 0u, (unsigned)_column_names.size() ) )
    {
      std::cout << "| " << std::setw( lengths[n] ) << _column_names[n] << " ";
    }
    std::cout << "|" << std::endl;

    std::string table;
    std::back_insert_iterator<std::string> sink( table );
    generate( sink, *( "| " << left_align( lengths[0] )[string]
                       << " | " << right_align( lengths[1] )[uint_]
                       << " | " << right_align( lengths[2] )[uint_]
                       << " | " << right_align( lengths[3] )[uint_]
                       << " | " << right_align( lengths[4] )[uint_]
                       << " | " << right_align( lengths[5] )[uint_ | "T/O"]
                       << " | " << right_align( lengths[6] )[uint_ | "T/O"]
                       << " |" << eol ), results );
    std::cout << table << std::endl;
  }

private:
  std::vector<std::string> _column_names;
  std::vector<benchmark> results;
  std::vector<unsigned> lengths;
  unsigned length_i;
};
