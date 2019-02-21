//
// Created by Jaewon Choi on 2019-02-21.
//

#ifndef SIMPLE_DEMO_NOTIMPLEMENTEDEXCEPTION_H
#define SIMPLE_DEMO_NOTIMPLEMENTEDEXCEPTION_H

#include <stdexcept>

class NotImplementedException : public std::logic_error {
public:
  explicit NotImplementedException(const char *msg) : std::logic_error{msg} {}

  char const *what() const noexcept override { return "Function not yet implemented."; }
};

#endif //SIMPLE_DEMO_NOTIMPLEMENTEDEXCEPTION_H
