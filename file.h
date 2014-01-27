#ifndef FILE_H_44C4E2DC_00E6_421A_9966_A218E6BABB8E
#define FILE_H_44C4E2DC_00E6_421A_9966_A218E6BABB8E

#include <string>
namespace mnb {

class File {
  public:
    enum FileStatus {
      kStatus_OK = 0,
      kStatus_NotFound = -1,
      kStatus_InUse = -2,
      kStatus_Exists = -3,
      kStatus_AccessDenied = -4,
      kStatus_Unknown = -5
    };

    enum FileMode {
      kMode_Noop = 0x0000,
      kMode_Read = 0x0001,
      kMode_Write = 0x0002,
      kMode_ReadWrite = kMode_Read | kMode_Write,
      kMode_Append = 0x0004,
      kMode_Truncate = 0x0008,
      kMode_Text = 0x0010
    };

  public:
    File()
      : file_(0) {}
    ~File() {}
    // reAPI reIMPL
    FileStatus open(const std::string& filename, const FileMode flags);
    int read(char* data, int size);
    bool readline(std::string& contents);
    bool isEOF() const;

  private:
    FileStatus nativeOpen(const std::string& file, const int flags);
#ifndef _OS_WIN
    static FileStatus OSErrorToFileStatus(int saved_errno);
//#else
//    static FileStatus OSErrorToFileStatus(DWORD last_error);
#endif
  int file_;//description
};

}
#endif /* FILE_H_44C4E2DC_00E6_421A_9966_A218E6BABB8E */

