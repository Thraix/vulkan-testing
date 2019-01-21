#include <iostream>

#include <math/Vec4.h>
#include <math/Mat4.h>
#include <Application.h>

int main()
{

  Application app;
  try
  {
    app.run();
  }
  catch(const std::exception& e)
  {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
