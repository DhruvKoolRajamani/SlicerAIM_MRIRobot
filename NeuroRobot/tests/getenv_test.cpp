#include <iostream>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
  char* meshlab_dir_path;
  meshlab_dir_path = getenv("MESHLAB_BIN_DIR");

  if (meshlab_dir_path != NULL)
    std::cout << meshlab_dir_path << std::endl;
  else
    std::cout << "Meshlab Dir Path is NULL" << std::endl;

  return 0;
}