//
// file:			qt_compress_directory.cpp
// created on:		2020 Jun 04
// created by:		D. Kalantaryan (davit.kalantaryan@gmail.com)
// 

#include <qt_zlib_compression_routines.hpp>
#include <vector>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QByteArray>
#include <string>

namespace qtcore{

static int IterateDirectoryIntoList(const QString& a_foldePath, ::std::vector< SCompressedFileItem >* a_pOutputVector );
static QByteArray PrepareByteStremFromHeadersList( const ::std::vector< SCompressedFileItem >& a_headerList, int a_nHeaderSize );
static QByteArray WriteByteArrayToZipFile(z_stream* a_strm, int a_flush, const void* a_byteStream, int a_byteStreamSize);
static bool CompressFolder(const QString& a_foldePath, const QString& a_targetFilePath, int a_compressionLevel);
static bool CompressFile(const QString& a_filePath, const QString& a_targetFilePath, int a_compressionLevel);


bool CompressFileOrFolder(const QString& a_fileOrFolderPath, const QString& a_targetFilePath, int a_compressionLevel)
{
	if(QFileInfo(a_fileOrFolderPath).isDir()){
		return CompressFolder(a_fileOrFolderPath,a_targetFilePath,a_compressionLevel);
	}
	
	return CompressFile(a_fileOrFolderPath,a_targetFilePath,a_compressionLevel);
}

static bool CompressFile(const QString& a_filePath, const QString& a_targetFilePath, int a_compressionLevel)
{
	bool bReturnFromFunc = false;
	int nReturnFromZlibRoutine;
	QFile inputFile(a_filePath);
	QFile targetFile(a_targetFilePath);
	QByteArray arrayToCompress;
	QByteArray compressedData;
	z_stream strm;
	SRawCompressHeader zipRawHeader = INIT_CD_MAIN_HEADER(0);
	
	zipRawHeader.stats.bits.isDirectory = 0;
	
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	nReturnFromZlibRoutine = deflateInit(&strm, a_compressionLevel);
	if (nReturnFromZlibRoutine != Z_OK)
		return false;
	
	if(!targetFile.open(QIODevice::WriteOnly)){
		goto returnPoint;
	}
	
	if(!inputFile.open(QIODevice::ReadOnly)){
		goto returnPoint;
	}
	
	zipRawHeader.itemsHeaderSizeOrFileSize = inputFile.size();
	
	compressedData = WriteByteArrayToZipFile(&strm,0,&zipRawHeader,sizeof(SRawCompressHeader));
	targetFile.write(compressedData);
	
	arrayToCompress = inputFile.readAll();
	inputFile.close();
	compressedData = WriteByteArrayToZipFile(&strm,1,arrayToCompress.data(),arrayToCompress.size());
	targetFile.write(compressedData);
		
	bReturnFromFunc = true;
returnPoint:
	targetFile.close();
	deflateEnd(&strm); // if not inited then this line will not run
	return bReturnFromFunc;
}


static bool CompressFolder(const QString& a_foldePath, const QString& a_targetFilePath, int a_compressionLevel)
{
	int nReturnFromZlibRoutine;
	bool bReturnFromFunc = false;
	int nHeadersSize;
	z_stream strm;
	QFile targetFile(a_targetFilePath);
	QFile inputFile;
	::std::vector< SCompressedFileItem > outputVector;
	QByteArray arrayToCompress;
	QByteArray compressedData;
	SRawCompressHeader zipRawHeader = INIT_CD_MAIN_HEADER(1);
	size_t unOutVectorSize, unIndex;
	QString filePath;
	
	zipRawHeader.stats.bits.isDirectory = 1;
	
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	nReturnFromZlibRoutine = deflateInit(&strm, a_compressionLevel);
	if (nReturnFromZlibRoutine != Z_OK)
		return false;
	
	nHeadersSize = IterateDirectoryIntoList(a_foldePath,&outputVector);
	if(nHeadersSize<=0){
		goto returnPoint;
	}
	
	if(!targetFile.open(QIODevice::WriteOnly)){
		goto returnPoint;
	}
	
	zipRawHeader.itemsHeaderSizeOrFileSize = nHeadersSize;
		
	compressedData = WriteByteArrayToZipFile(&strm,0,&zipRawHeader,sizeof(SRawCompressHeader));
	targetFile.write(compressedData);
	
	arrayToCompress = PrepareByteStremFromHeadersList(outputVector,nHeadersSize);
	compressedData = WriteByteArrayToZipFile(&strm,0,arrayToCompress.data(),arrayToCompress.size());
	targetFile.write(compressedData);
	
	unOutVectorSize = outputVector.size();
	for(unIndex=0;unIndex<unOutVectorSize;++unIndex){
		if(!outputVector[unIndex].raw.stats.bits.isDirectory){
			if(outputVector[unIndex].raw.stats.bits.shouldRootDirBeAdded){
				filePath = a_foldePath + outputVector[unIndex].itemFullPath;
			}
			else{
				filePath = outputVector[unIndex].itemFullPath;
			}
			qDebug()<<filePath;
			inputFile.setFileName(filePath);
			if(inputFile.open(QIODevice::ReadOnly)){
				arrayToCompress = inputFile.readAll();
				inputFile.close();
				compressedData = WriteByteArrayToZipFile(&strm,0,arrayToCompress.data(),arrayToCompress.size());
				targetFile.write(compressedData);
			}
		}
	}
	
	compressedData = WriteByteArrayToZipFile(&strm,1,&zipRawHeader,sizeof(SRawCompressHeader));
	targetFile.write(compressedData);
	
	bReturnFromFunc = true;
returnPoint:
	targetFile.close();
	deflateEnd(&strm); // if not inited then this line will not run
	return bReturnFromFunc;
}


static QByteArray PrepareByteStremFromHeadersList( const ::std::vector< SCompressedFileItem >& a_headerList, int a_nHeaderSize )
{
	QByteArray returnArray(a_nHeaderSize,0);
	char* pcData = returnArray.data();
	const size_t cunHeadersCount = a_headerList.size();
	size_t unIndex;
	::std::wstring asciiPath;
	
	for(unIndex=0;unIndex<cunHeadersCount;++unIndex){
		asciiPath = a_headerList[unIndex].itemFullPath.toStdWString();
		memcpy(pcData,&(a_headerList[unIndex].raw),sizeof(SRawCompressedFileItem));
		memcpy(pcData+sizeof(SRawCompressedFileItem),asciiPath.c_str(),sizeof(wchar_t)*a_headerList[unIndex].raw.itemPathLengthPlus1);
		qDebug()<< QString::fromWCharArray(reinterpret_cast<wchar_t*>(pcData+sizeof(SRawCompressedFileItem)));
		pcData += ITEM_TOTAL_LENGTH_FROM_ITEM_PTR(&(a_headerList[unIndex].raw));
	}
	
	return returnArray;
}


static int IterateDirectoryIntoList(const QString& a_foldePath, ::std::vector< SCompressedFileItem >* a_pOutputVector )
{
	QFileInfo itemFileInfo;
	QString itemPath, itemPathRelative;
	QString itemFileName;
	QDirIterator it(a_foldePath, QDirIterator::Subdirectories);
	size_t i, unVectorSize;
	bool bItemNotFound;
	int nHeaderSize = -1;
	SCompressedFileItem aNewItem;
	
	a_pOutputVector->clear();
	
	// check if directory exist
	if(!QFileInfo(a_foldePath).exists()){
		return nHeaderSize;
	}
	
	//// add first item as root of directory
	//a_pOutputVector->push_back(NULL_ITEM_FROM_PATH(a_foldePath));
	//nHeaderSize = sizeof(SRawCompressedFileItem);
	//a_pOutputVector->push_back(NULL_ITEM_FROM_PATH(""));
	//nHeaderSize = sizeof(SRawCompressedFileItem);
	nHeaderSize = 0;
	
	while (it.hasNext()) {	
		itemPath = it.next();
		itemFileInfo = QFileInfo(itemPath);
		itemFileName = itemFileInfo.fileName();
		if((itemFileName==".")||(itemFileName=="..")){
			continue;
		}
		
		aNewItem = NULL_ITEM_FROM_PATH(itemPathRelative);
		
		if(itemPath.startsWith(a_foldePath)){
			itemPathRelative = itemPath.mid(a_foldePath.size());
			aNewItem.raw.stats.bits.shouldRootDirBeAdded = 1;
		}
		else{
			itemPathRelative = itemPath;
		}
		
		// check if item already exist
		bItemNotFound = true;
		unVectorSize = a_pOutputVector->size();
		for(i=0;i<unVectorSize;++i){
			if(itemPathRelative==a_pOutputVector->at(i).itemFullPath){
				bItemNotFound = false;
				break;
			}			
		}  // for(i=0;i<unVectorSize;++i){
		
		if(!bItemNotFound){
			// this item already added
			continue;
		}
		
		if(itemFileInfo.isDir()){
			aNewItem.raw.stats.bits.isDirectory = 1;
		}
		else{
			aNewItem.raw.fileSize = static_cast<uint32_t>(itemFileInfo.size());
		}
		aNewItem.raw.itemPathLengthPlus1 = static_cast<uint16_t>(itemPathRelative.toStdWString().length())+1;
		aNewItem.itemFullPath = itemPathRelative;
		
		a_pOutputVector->push_back(aNewItem);
		nHeaderSize += ITEM_TOTAL_LENGTH_FROM_ITEM_PTR(&(aNewItem.raw));
	}
	
	return nHeaderSize;
	
}

#define DEF_CHUNK_SIZE				16384


static QByteArray WriteByteArrayToZipFile(z_stream* a_strm, int a_flush, const void* a_byteStream, int a_byteStreamSize)
{
	QByteArray returnByteArray;
	unsigned char out[DEF_CHUNK_SIZE];
	int retZlib;
	int nOldArraySize;
	char* pcReturnArrayPointer;
	
	a_strm->next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(a_byteStream));	
	a_strm->avail_in = a_byteStreamSize;
	
	do {
	
		a_strm->avail_out = DEF_CHUNK_SIZE;
		a_strm->next_out = (Bytef*)out;
		
		retZlib = deflate(a_strm, a_flush);		/* no bad return value */
		assert(retZlib != Z_STREAM_ERROR);		/* state not clobbered */
		
		nOldArraySize = returnByteArray.size();
		returnByteArray.resize(nOldArraySize + (DEF_CHUNK_SIZE-a_strm->avail_out));
		pcReturnArrayPointer = returnByteArray.data();
		
		memcpy(pcReturnArrayPointer+nOldArraySize,out,DEF_CHUNK_SIZE-a_strm->avail_out);
	
	
	} while (a_strm->avail_out == 0);
	assert(a_strm->avail_in == 0);     /* all input will be used */
	return returnByteArray;
}




}  // namespace qt{
