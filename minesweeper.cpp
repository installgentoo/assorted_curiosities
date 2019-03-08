#include <iostream>
#include <sstream>
#include <optional>
#include <array>
#include <random>
#include <assert.h>
#include <numeric>

//i won't split program into separate files since i've been told your mail filters out .zip

//compile with c++17

//actual program starts after this section, please bear with it.

//my homebrew utilities, pay them no nevermind
/**************************************************************************************/
using std::move;
using std::string;
using std::array;
using std::vector;
using std::pair;
using std::optional;
using std::nullopt;

using ubyte = uint8_t;
using uint  = uint32_t;

#define Val auto const&

template<class T> using result = pair<optional<T>, string>;

#define CEXIT() assert(0 && "bug"); exit(-1);

#define CRESULT(val) { move(val), { } }
#define CFAIL(text)  { nullopt, [&]{ std::ostringstream _err_s; _err_s<<"E "<<text<<" |in "<<__FILE__<<":"<<__LINE__<<"\n"; return _err_s.str(); }() }
#define CTRY(result) value_or_fail(result)

template<class T> T value_or_fail(result<T> &&result) {
  if(!result.first) { std::cout<<result.second; CEXIT(); }
  return move(*result.first);
}

template<class To, class T> To cast(T n) {
  assert((static_cast<long double>(n) >= static_cast<long double>(std::numeric_limits<To>::lowest())) &&
         (static_cast<long double>(n) <= static_cast<long double>(std::numeric_limits<To>::max())) && "Cast overflow");
  return static_cast<To>(n);
}
/***************************************************************************************/



//limit scope, "enum : Name" bleeds entries
namespace Cell { enum type : ubyte {
  EMPTY = 0,
  M1 = 1,
  M2 = 2,
  M3 = 3,
  M4 = 4,
  M5 = 5,
  M6 = 6,
  M7 = 7,
  M8 = 8,
  M9 = 9,
  MINE = 10,
  CLOSED = 11
}; };


static char cell_to_char(Cell::type c) {
  switch(c) {
    case Cell::EMPTY:  return '.';
    case Cell::M1:     return '1';
    case Cell::M2:     return '2';
    case Cell::M3:     return '3';
    case Cell::M4:     return '4';
    case Cell::M5:     return '5';
    case Cell::M6:     return '6';
    case Cell::M7:     return '7';
    case Cell::M8:     return '8';
    case Cell::M9:     return '9';
    case Cell::MINE:   return 'M';
    case Cell::CLOSED: return '#';
  }
  CEXIT();
}


class Minefield {
public:

  struct LevelMap {
    uint width, height;
    vector<Cell::type> cells;
  };

  Minefield(uint width, uint height, uint count)
    : m_map(CTRY(GenerateMineField(width, height, count)))
  { }

  //There's a bunch of macros here, but for good reason:
  //
  //Val is my macro for "auto const&". Why not just write it out? Macros can be made more visible in your ide; also less typing.
  //Thus we immediately notice that Val declares immutable, this is a blessing for readibility.
  //Isn't this dangerous? No, clang 7+ asan will catch all dangling references.
  //
  //CFAIL CRESULT result<> are basically rust's Result. Why not just use exceptions? Result's more explicit, it has to be unwraped with structured bindings or CTRY.

  //part 1

  static result<LevelMap> GenerateMineField(uint width, uint height, uint count) {
    Val field_size = width * height;
    if(field_size < count)
      return CFAIL("Requested more mines than a field can fit");

    if(!field_size)
      return CFAIL("No field to fill");

    //shuffle count cell positions then fill
    Val c = [&]{
      vector<uint> candidates(field_size, 0);
      std::iota(candidates.begin(), candidates.end(), 0);

      std::default_random_engine engine(std::random_device{}());
      std::uniform_int_distribution<uint> dist(0, field_size - 1);

      for(uint i=0; i<count; ++i) {
        Val guess = dist(engine);
        std::swap(*std::next(candidates.begin(), i), *std::next(candidates.begin(), guess));
      }

      return candidates;
    }();

    vector<ubyte> field(field_size, Cell::EMPTY);

    //map out mines
    Val s_height=cast<int>(height);
    Val s_width=cast<int>(width);
    for(uint k=0; k<count; ++k) {
      Val idx = c[k];

      field[idx] = Cell::MINE;

      Val s_idx = cast<int>(idx);
      Val j = s_idx / s_width;
      Val i = s_idx % s_width;

      for(int y=-1; y<2; ++y)
        for(int x=-1; x<2; ++x) {
          if((i + x) >= 0 &&
             (j + y) >= 0 &&
             (i + x) < s_width &&
             (j + y) < s_height)
          {
            Val cell_idx = cast<uint>(x + i + (y + j) * s_width);
            auto& cell = field[cell_idx];

            if(cell != Cell::MINE)
              ++cell;
          }
        }
    }

    //enum it for switch safety
    vector<Cell::type> cells;
    for(ubyte i: field) {
      cells.emplace_back(Cell::type{ i });
    }

    auto map = LevelMap{ width, height, move(cells) };

    return CRESULT(map);
  }


  //part 2

  Cell::type Open(uint x, uint y) {
    //???
    //what is this method? why does it return cell? shoudln't we return list of opened cells?
    return this->GetCellAt(x, y);
  }

  //how do we know the size of game field with just these two methods??
  Cell::type GetCellAt(uint x, uint y)const {
    return m_map.cells[x + y * m_map.width];
  }

  //i won't put anything else here as i've been given just these two methods and asked to implement solver, not the interface

  //private:
  LevelMap m_map;
};


//utility for printing
static string PrintMap(uint width, uint height, vector<Cell::type> const&cells) {
  std::stringstream output;

  for(uint y=0; y<height; ++y) {
    output<<'\n';

    for(uint x=0; x<width; ++x) {
      Val i = x + y * width;
      output<<cell_to_char(cells[i]);
    }
  }
  output<<'\n';

  return output.str();
}

static string PrintMap(Minefield::LevelMap const&map) {
  return PrintMap(map.width, map.height, map.cells);
}

//now follow the functions to solve minesweeper


//recursive Open
static void trace_open(int width, int height, int i, int j, Minefield &field, vector<Cell::type> &output) {
  if(i < 0 ||
     j < 0 ||
     i >= width ||
     j >= height)
    return;

  Val idx = cast<uint>(i + j * width);
  if(output[idx] != Cell::CLOSED)
    return;

  Val cell = field.GetCellAt(cast<uint>(i), cast<uint>(j));
  output[idx] = cell;

  if(cell != Cell::EMPTY)
    return;

  trace_open(width, height, i - 1, j, field, output);
  trace_open(width, height, i + 1, j, field, output);
  trace_open(width, height, i, j - 1, field, output);
  trace_open(width, height, i, j + 1, field, output);
}


//function to guess the best cell to open
static int take_a_guess(vector<Cell::type> const&cells, int width, int height, uint total_mines_ingame) {
  static std::default_random_engine engine(std::random_device{}());

  Val at = [&](int x, int y, uint i){
    if(x < 0 ||
       y < 0 ||
       x >= width ||
       y >= height)
      return cast<uint>(i);

    return cast<uint>(x + y * width + cast<int>(i));
  };

  //heuristics!
  //basically find the first closed cell with least mines near it and click it
  Val check_danger_level = [&](uint i){
    Val danger_rating = [&](uint idx){
      int r = 0;
      for(int y=-1; y<2; ++y)
        for(int x=-1; x<2; ++x) {
          Val cell = cells[at(x, y, idx)];

          if(cell != Cell::CLOSED)
            r += cast<int>(cell);
        }
      return r;
    };

    pair<int, int> best_rating = { -1, 9001 };

    for(int y=-1; y<2; ++y)
      for(int x=-1; x<2; ++x) {
        Val idx = at(x, y, i);
        if(cells[idx] == Cell::CLOSED)
        {
          Val rating = danger_rating(idx);
          if(best_rating.second < rating)
            best_rating = { cast<int>(idx), rating };
        }
      }

    return best_rating;
  };

  vector<uint> dangerous, isolated;

  //separating cells into danger tiers
  for(uint i=0; i<cells.size(); ++i) {
    switch (cells[i]) {
      case Cell::EMPTY: break;
      case Cell::M1:
      case Cell::M2:
      case Cell::M3:
      case Cell::M4:
      case Cell::M5:
      case Cell::M6:
      case Cell::M7:
      case Cell::M8: dangerous.emplace_back(i); break;
      case Cell::CLOSED: {
        Val is_isolated = [&]{
          for(int y=-1; y<2; ++y)
            for(int x=-1; x<2; ++x) {
              Val cell = cells[at(x, y, i)];
              if(cell != Cell::CLOSED)
                return false;
            }

          return true;
        };

        if(is_isolated())
          isolated.emplace_back(i);
      } break;
      default:
        CEXIT();
    }
  }

  //chance to click mine blindly
  Val yolo_ratio = isolated.empty() ? 9001.
                                    : double(total_mines_ingame) / isolated.size();
  Val yolo_guess = [&]{
    std::uniform_int_distribution<uint> dist(0, isolated.size() - 1);
    return cast<int>(isolated[dist(engine)]);
  };

  if(yolo_ratio < 1.)
    return yolo_guess();

  for(Val i: dangerous) {
    if(Val [idx, danger] = check_danger_level(i);
       danger < yolo_ratio)
      return idx;
  }

  if(yolo_ratio < 7.)
    return yolo_guess();

  return -1;
}


//solver method
static void SolveMinefield(Minefield &field, uint total_mines_ingame) {
  Val map = field.m_map;
  //this should be implemented in class interface
  Val width = map.width;
  Val height = map.height;

  //this should be returned from Open implementation, alas i've no idea what that implementation is
  //i'll pretend that we called Open and then went over every cell with GetCellAt
  vector<Cell::type> cells;
  for(uint i=0; i<width * height; ++i) {
    cells.emplace_back(Cell::CLOSED);
  }

  while(true) {
    Val _guess = take_a_guess(cells, cast<int>(width), cast<int>(height), total_mines_ingame);
    if(_guess == -1)
    {
      std::cout<<"A Strange Game.\n";
      return;
    }

    Val guess = cast<uint>(_guess);
    Val j = guess / width;
    Val i = guess % width;

    Val cell = field.GetCellAt(i, j);

    if(cell == Cell::MINE)
    {
      cells[i + j * width] = Cell::MINE;
      std::cout<<PrintMap(width, height, cells)<<"\n\n";
      std::cout<<".....................\nBOOOOOOM!!!\n\n";
      std::cout<<PrintMap(map);
      return;
    }

    //i guess this is how open could work?
    trace_open(cast<int>(width), cast<int>(height), cast<int>(i), cast<int>(j), field, cells);

    std::cout<<PrintMap(width, height, cells)<<"\n\n";
  }
}


int main()
{
  Val map = CTRY(Minefield::GenerateMineField(47, 21, 43));
  std::cout<<"PART 1:\n"<<PrintMap(map);

  std::cout<<"\n\n\nPART 2:\n";
  Minefield field(41, 22, 75);
  SolveMinefield(field, 70);

  return 0;
}
