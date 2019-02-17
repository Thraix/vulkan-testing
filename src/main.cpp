#include <iostream>

#include <math/Vec4.h>
#include <math/Mat4.h>
#include <Application.h>

int main()
{

  Application app;
    app.run();
  try
  {
  }
  catch(const std::exception& e)
  {
    std::cerr << "ERROR: " << e.what() << std::endl;
    abort();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
