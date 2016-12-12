#include "client.h"

#include <iostream>

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
    return 1;
  }

  Client client{argv[1], argv[2]};

  if (!client.connect()) {
    std::cerr << "Error connecting to " << argv[1] << ":" << argv[2] << std::endl;
    return 1;
  } else {
    std::cout << "Connected to " << argv[1] << ":" << argv[2] << std::endl;
  }

  client.run();

  return 0;
}
