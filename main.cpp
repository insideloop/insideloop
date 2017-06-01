//==============================================================================
//
//                                  InsideLoop
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.txt for details.
//
//==============================================================================

#include <iostream>

#include <il/String.h>

int main() {
  il::String welcome = "Hello World!";

  std::cout << welcome.c_string() << std::endl;

  return 0;
}
