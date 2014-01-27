#include "file.h"

namespace mnb {

File::FileStatus File::open(const std::string& filename, const FileMode flags) {
  return nativeOpen(filename, flags);
}

}
