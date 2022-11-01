

// https://desycloud.desy.de/index.php/s/XvNhzouZlV5s12W/download?path=%2FARM&files=doocs_server.dsi
#include "zlib_decompress_routines.h"

#pragma comment(lib,"Wininet.lib")

int main()
{
	//char** argv2 = a_argv + 1;

	int nReturn = ZlibDecompressFromWeb("https://desycloud.desy.de/index.php/s/XvNhzouZlV5s12W/download?path=%2FARM&files=doocs_server.dsi", "D:\\arm");

	return nReturn;
}
