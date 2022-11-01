
// qt_zlib_decompress_routines.hpp
// to include ->  #include "qt_zlib_decompress_routines.hpp"
// 2018 Feb 12

#ifndef qt_zlib_decompress_routines_hpp
#define qt_zlib_decompress_routines_hpp

#include <qt_zlib_compress_decompress_common.hpp>
#include <vector>


namespace qtcore{

bool DecompressFile(const QString& a_compressedFilePath, const QString& targetFolderOrFilePath, ::std::vector< ::qtcore::SCompressedFileItem >* a_pVector);

}

#endif  // #ifndef qt_zlib_decompress_routines_hpp
