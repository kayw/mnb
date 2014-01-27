#include "file.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "posix_einter.h"

namespace mnb {

File::FileStatus File::nativeOpen(const std::string& file, const int flags) {
  int openflags = O_CREAT;
  if (flags & kMode_ReadWrite) {
    openflags |= O_RDWR;
  }
  else if (flags & kMode_Write) {
    openflags |= O_WRONLY;
  }
  if (flags & kMode_Append && flags & kMode_Read)
    openflags |= O_APPEND | O_RDWR;
  else if (flags & kMode_Append)
    openflags |= O_APPEND | O_WRONLY;

  int mode = S_IRUSR | S_IWUSR;

  FileStatus fsError;
  int descriptor = HANDLE_EINTR(::open(file.c_str(), openflags, mode));
  if (descriptor >= 0)
    fsError = kStatus_OK;
  else
    fsError = File::OSErrorToFileStatus(errno);
  file_ = descriptor;
  return fsError;
}

int File::read(char* data, int size) {
  if (size < 0)
    return -1;

  int bytes_read = 0;
  int rv;
  do {
    rv = HANDLE_EINTR(::read(file_, data, size));
    if (rv <= 0)
      break;

    bytes_read += rv;
  } while (bytes_read < size);

  return bytes_read ? bytes_read : rv;
}

// slow impl from qabstractfileengine TODO
// need buffer reading
bool File::readline(std::string& contents) {
  int readsofar = 0;
  char c = '\0';
  while(c != '\n') {
    int readResult = read(&c, 1);
    //http://www.techytalk.info/linux-system-programming-open-file-read-file-and-write-file/
    if (readResult <= 0)
      return false;
    ++readsofar;
    if (c != '\n')
      contents.append(1, c);
  }
  return true;
}

// Static.
File::FileStatus File::OSErrorToFileStatus(int saved_errno) {
  switch (saved_errno) {
    case EACCES:
    case EISDIR:
    case EROFS:
    case EPERM:
      return kStatus_AccessDenied;
    case ETXTBSY:
      return kStatus_InUse;
    case EEXIST:
      return kStatus_Exists;
    case ENOENT:
      return kStatus_NotFound;
    //case EMFILE:
    //  return FILE_ERROR_TOO_MANY_OPENED;
    //case ENOMEM:
    //  return FILE_ERROR_NO_MEMORY;
    //case ENOSPC:
    //  return FILE_ERROR_NO_SPACE;
    //case ENOTDIR:
    //  return FILE_ERROR_NOT_A_DIRECTORY;
    default:
      return kStatus_Unknown;
  }
}

}
