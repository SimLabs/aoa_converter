#define NOMINMAX

#include "common/stl.h"
#include "common/boost.h"
#include "boost/filesystem.hpp"

namespace fs = boost::filesystem;

#include "geometry/primitives/point.h"
#include "geometry/primitives/rectangle.h"
#include "geometry/primitives/range.h"

#include "logger/logger.hpp"

#include "boost/optional.hpp"
#include "boost/functional/hash.hpp"

using boost::optional;

#undef Verify
#define Verify assert