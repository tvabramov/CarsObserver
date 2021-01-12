#ifndef LAUNCHPARAMS_H
#define LAUNCHPARAMS_H

#include <boost/optional.hpp>

struct launchParams {
  boost::optional<std::string> configFileName;
};

#endif  // LAUNCHPARAMS_H
