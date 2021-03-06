#include <iostream>
#include <cstdlib>

#include "opendihu.h"

int main(int argc, char *argv[])
{
  // 0D sub-cellular model
  
  // initialize everything, handle arguments and parse settings from input file
  DihuContext settings(argc, argv);
  
  TimeSteppingScheme::Heun<
    CellmlAdapter<57>
  > equationDiscretized(settings);
  
  equationDiscretized.run();
  
  return EXIT_SUCCESS;
}
