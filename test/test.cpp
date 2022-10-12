
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#ifndef macintosh
#include <sys/stat.h>
#include <sys/types.h>
#else
#include <stat.h>
#endif

#ifdef WIN32
#include <io.h>
#endif

#include "TiffImageFile.h"
#include "TiffDirectory.h"
#include "TiffStripImage.h"
#include "TiffTileImage.h"
#include "TiffError.h"
#include "TiffFile.h"
#include "TiffUtil.h"

#ifdef WIN32
#include <WinInet.h>
#pragma comment (lib, "WinInet")
#endif


void copyImageData(TiffDirectory* outdir, TiffDirectory* indir)
{
	// if no image
	if (!indir->getImage())
		return;

	if (indir->isTiled())
	{
		TiffTileImage* inImg = (TiffTileImage*)indir->getImage();
		TiffTileImage* outImg = (TiffTileImage*)outdir->getImage();
		int size = inImg->tileSize();
		tdata_t buf = new unsigned char[size];
		tstrip_t numTiles = inImg->numOfTiles();
		for (unsigned int i=0; i<numTiles; i++)
		{
			size = inImg->readRawTile(i, buf, -1);
			outImg->writeRawTile(i, buf, size);
//			size = inImg->readEncodedTile(i, buf, -1);
//			outImg->writeEncodedTile(i, buf, size);
		}

		outImg->flush();

		delete (unsigned char*)buf;
	}
	else
	{
		TiffStripImage* inImg = (TiffStripImage*)indir->getImage();
		TiffStripImage* outImg = (TiffStripImage*)outdir->getImage();
		int size = inImg->scanlineSize();
		tdata_t buf = new unsigned char[size];
		tiff_uint32 length = indir->imageLength();
		tstrip_t numStrips = inImg->numOfStrips();

		tstrip_t curStrip;
		for (unsigned int row=0; row<length; row++)
		{
			inImg->readScanline(buf, row);
			outImg->writeScanline(buf, row);
			curStrip = inImg->currentStrip();
		}
		outImg->flush();

		delete (unsigned char*)buf;
	}
}


// create sub dirs in outdir with the data from indir
void createSubDirs(TiffDirectory* outdir, TiffDirectory* indir)
{

	// create SubIFDs
	TiffTagEntry* tagEntry = new TiffTagEntryT<vector<tiff_uint32> >(330, EKTIFF_ULONG, 2, vector<tiff_uint32>(2, 0));
	outdir->setGenericTag(*tagEntry);
	delete tagEntry;
	TiffDirectory* subdir = outdir->getSubDir(330, 0);
	subdir->copy(*indir);
	copyImageData(subdir, indir);

	subdir = outdir->getSubDir(330, 1);
	subdir->copy(*indir);

	// create ExifIFD
	tagEntry = new TiffTagEntryT<tiff_uint32>(34665, EKTIFF_ULONG, 1, 0);
	outdir->setGenericTag(*tagEntry);
	delete tagEntry;
	TiffDirectory* exifdir = outdir->getSubDir(34665);
	// ScannerSoftware
	string str = "ScannerSoftware";
	tagEntry = new TiffTagEntryT<string>(50012, EKTIFF_ASCII, str.size() + 1, str); 
	exifdir->setGenericTag(*tagEntry);
	delete tagEntry;
	// Fnumber
	float rVal = (float)(23.0/34);
	tagEntry = new TiffTagEntryT<float>(33437, EKTIFF_RATIONAL, 1, rVal);
	exifdir->setGenericTag(*tagEntry);
	delete tagEntry;
	
	// create InterIFD under ExifIFD
	tagEntry = new TiffTagEntryT<tiff_uint32>(40965, EKTIFF_ULONG, 1, 0);
	exifdir->setGenericTag(*tagEntry);
	delete tagEntry;
	subdir = exifdir->getSubDir(40965);
	// InteroperabilityIndex
	str = "InteroperabilityIndex";
	tagEntry = new TiffTagEntryT<string>(1, EKTIFF_ASCII, str.size() + 1, str); 
	subdir->setGenericTag(*tagEntry);
	delete tagEntry;

	// create private IFD
	TiffDirectory::registerDirTag(16868);
	tagEntry = new TiffTagEntryT<tiff_uint32>(16868, EKTIFF_ULONG, 1, 0);
	outdir->setGenericTag(*tagEntry);
	delete tagEntry;
	subdir = outdir->getSubDir(16868);
	// set private tag
	str = "This is a private tag under a private dir";
	tagEntry = new TiffTagEntryT<string>(1, EKTIFF_ASCII, str.size() + 1, str); 
	subdir->setGenericTag(*tagEntry);
	delete tagEntry;
}


typedef enum
{
	TEST_BASIC,
	TEST_ADVANCE
} TestMode;


void testWriting(char* outFile, char* outMode, toff_t outMapSize, TiffDirectory* indir, TestMode mode)
{
	// create output file for writing
	TiffImageFile outImageFile;
	printf("Creating output file...\n");
	outImageFile.open(outFile, outMode, outMapSize);
	printf("Creating Directory 0 of output file...\n");
	TiffDirectory* outdir = outImageFile.createDirectory();
	
	// copy from inImageFile
	printf("Copying directory from input file... \n\n");
	outdir->copy(*indir);
	
	printf("Copying image data from input file... \n\n");
	copyImageData(outdir, indir);
	if (mode == TEST_ADVANCE)
	{
		printf("creating more sub dirs in directory 0...\n");
		createSubDirs(outdir, indir);
	}

	if (mode == TEST_ADVANCE)
	{
		printf("Creating directory 1 of output file...\n");
		outdir = outImageFile.createDirectory();
		outdir->copy(*indir);
		copyImageData(outdir, indir);
	}

	outImageFile.close();

	// read in and verify
	printf("Opening and Reading output file... \n");
	outMapSize = 0;
	outImageFile.open(outFile, "r", outMapSize);

	for (int i=0; ; i++)
	{
		outdir = outImageFile.getDirectory(i);
		if (outdir)
		{
			outdir->print();
			printf("\n");
		}
		else
			break;
	}
	
	outImageFile.close();
}


void testReadWrite(TestMode mode = TEST_BASIC)
{
	try
	{
        char inputFile[] = "input.txt";
		char inFile[256] = "";
		char outFile[256] = "TestOut.tif";
		char inMode[10] = "r";
		char outMode[10] = "w";
		toff_t inMapSize = 0;
		toff_t outMapSize = 0;
        FILE* f = fopen(inputFile, "r");
		while (true) // loop for input file
		{
			printf("\n\n******** Input Info -- FileName, OpenMode, MapSize (e.g. test.tif r 0):\n");
			if (fscanf(f, "%s %s %d", inFile, inMode, &inMapSize) == EOF)
                break;
//			if (!strcmp(inFile, "None"))
//				break;
			printf("%s %s %d\n", inFile, inMode, inMapSize);

			TiffImageFile inImageFile, outImageFile;
			inImageFile.open(inFile, inMode, inMapSize);
			
			while (true) // loop for input DirPath
			{
				char tmpStr[256];
				char seps[] = " ";
				printf("\n\n---------- Dir to read (e.g. 0/0 330/0): \n");
				fscanf(f, "%s", tmpStr);
				printf("%s\n", tmpStr);
				
				// look for main ifd DirPath
				TiffImageFile::DirPath dirPath;				
				int tag, idx;
				char* tok = strtok(tmpStr, seps);
				sscanf(tok, "%d/%d", &tag, &idx);
				if (tag == -1)
					break;
				dirPath.push_back(TiffImageFile::DirPath::value_type(tag, idx));

				// look for sub ifd DirPath
				for (int i=1;  ;i++)
				{
					tok = strtok(NULL, seps);
					if (tok == NULL)
						break;
					sscanf(tok, "%d/%d", &tag, &idx);
					dirPath[i].first = tag;
					dirPath[i].second = idx;
				}

				// read ifd entries
				printf("\n****Test reading IFD entries**** \n");
				TiffDirectory* indir = inImageFile.getDirectory(dirPath);
				if (indir)
				{
					indir->print();
				}
				else
				{
					printf("No IFD entry\n");
				}
				printf("\n");

				// test reading and writing
				printf("****Test Reading and Writing**** \n");
				printf("\n**Output Info -- FileName, OpenMode, MapSize (e.g. out.tif w 0):\n");
				fscanf(f, "%s %s %d", outFile, outMode, &outMapSize);
				printf("%s %s %d\n", outFile, outMode, outMapSize);
				if (!strcmp(outFile, "None"))
					continue; // skip writing test
				else
					testWriting(outFile, outMode, outMapSize, indir, mode);

			}

			inImageFile.close();

		}
	}
	catch (TiffError &e)
	{
		cout << "TIFF ERROR: " << e << endl;
	}
	catch (...)
	{
		cout << "OTHER ERROR" << endl;
	}
}


void testTiffFile()
{
	try 
	{
		TiffTagEntry* entry;
		tiff_uint32 tiff_uint32Val;
		uint16 uint16Val;
		float floatVal;
		string strVal;

		printf("\n\n\n*************** Testing TiffFile class ******************\n");
		TiffFile file;
		printf("\n****** Open DC290 file\n");
		file.open("DC290.tif", "r", FMT_ULTRATIFF);

		// PRIMARY IMAGE
		entry = file.getTag(TIFFTAG_IMAGEWIDTH, CIN_PRIMARYIMAGE);
		tiff_uint32Val = (dynamic_cast<TiffTagEntryT<tiff_uint32>*>(entry))->getValue();
		printf("Primary Image Width: %ld\n", tiff_uint32Val);

		entry = file.getTag(TIFFTAG_IMAGELENGTH, CIN_PRIMARYIMAGE);
		tiff_uint32Val = (dynamic_cast<TiffTagEntryT<tiff_uint32>*>(entry))->getValue();
		printf("Primary Image Length: %ld\n", tiff_uint32Val);

		entry = file.getTag(TIFFTAG_COMPRESSION, CIN_PRIMARYIMAGE);
		uint16Val = (dynamic_cast<TiffTagEntryT<uint16>*>(entry))->getValue();
		printf("Primary Image Compression: %d\n", uint16Val);

		printf("\n");

		// THUMBNAIL
		entry = file.getTag(TIFFTAG_IMAGEWIDTH, CIN_THUMBNAIL);
		tiff_uint32Val = (dynamic_cast<TiffTagEntryT<tiff_uint32>*>(entry))->getValue();
		printf("Thumbnail Width: %ld\n", tiff_uint32Val);

		entry = file.getTag(TIFFTAG_IMAGELENGTH, CIN_THUMBNAIL);
		tiff_uint32Val = (dynamic_cast<TiffTagEntryT<tiff_uint32>*>(entry))->getValue();
		printf("Thumbnail Length: %ld\n", tiff_uint32Val);

		entry = file.getTag(TIFFTAG_COMPRESSION, CIN_THUMBNAIL);
		uint16Val = (dynamic_cast<TiffTagEntryT<uint16>*>(entry))->getValue();
		printf("Thumbnail Compression: %d\n", uint16Val);

		printf("\n");

		// SCREENNAIL
		entry = file.getTag(TIFFTAG_IMAGEWIDTH, CIN_SCREENNAIL);
		tiff_uint32Val = (dynamic_cast<TiffTagEntryT<tiff_uint32>*>(entry))->getValue();
		printf("Screennail Width: %ld\n", tiff_uint32Val);

		entry = file.getTag(TIFFTAG_IMAGELENGTH, CIN_SCREENNAIL);
		tiff_uint32Val = (dynamic_cast<TiffTagEntryT<tiff_uint32>*>(entry))->getValue();
		printf("Screennail Length: %ld\n", tiff_uint32Val);

		entry = file.getTag(TIFFTAG_COMPRESSION, CIN_SCREENNAIL);
		uint16Val = (dynamic_cast<TiffTagEntryT<uint16>*>(entry))->getValue();
		printf("Screennail Compression: %d\n", uint16Val);

		printf("\n");

		// EXIF IFD
		entry = file.getTag(33434, CIN_EXIF);
		floatVal = (dynamic_cast<TiffTagEntryT<float>*>(entry))->getValue();
		printf("ExifIFD ExposureTime: %f\n", floatVal);

		entry = file.getTag(36868, CIN_EXIF);
		strVal = (dynamic_cast<TiffTagEntryT<string>*>(entry))->getValue();
		printf("ExifIFD DateTimeDigitized: %s\n", strVal.c_str());

		// ----- test for DCS
		file.close();
		printf("\n****** Open DCS file\n");
		file.open("Dcs420-1.tif", "r", FMT_DCS);

		// PRIMARY IMAGE
		entry = file.getTag(TIFFTAG_IMAGEWIDTH, CIN_PRIMARYIMAGE);
		tiff_uint32Val = (dynamic_cast<TiffTagEntryT<tiff_uint32>*>(entry))->getValue();
		printf("Primary Image Width: %ld\n", tiff_uint32Val);

		entry = file.getTag(TIFFTAG_IMAGELENGTH, CIN_PRIMARYIMAGE);
		tiff_uint32Val = (dynamic_cast<TiffTagEntryT<tiff_uint32>*>(entry))->getValue();
		printf("Primary Image Length: %ld\n", tiff_uint32Val);

		// THUMBNAIL
		entry = file.getTag(TIFFTAG_IMAGEWIDTH, CIN_THUMBNAIL);
		tiff_uint32Val = (dynamic_cast<TiffTagEntryT<tiff_uint32>*>(entry))->getValue();
		printf("Thumbnail Width: %ld\n", tiff_uint32Val);

		entry = file.getTag(TIFFTAG_IMAGELENGTH, CIN_THUMBNAIL);
		tiff_uint32Val = (dynamic_cast<TiffTagEntryT<tiff_uint32>*>(entry))->getValue();
		printf("Thumbnail Length: %ld\n", tiff_uint32Val);

	}
	catch (TiffError &e)
	{
		cout << "EKC ERROR: " << e << endl;
	}
	catch (...)
	{
		cout << "OTHER ERROR" << endl;
	}
}


void testAudio()
{
	char infile[] = "LS_None.tif";
	char outfile[] = "TestAudio.tif";
	try
	{
		printf("\n\n****** Test reading/writing audio stream\n");
		TiffImageFile inImgFile, outImgFile;
		inImgFile.open(infile, "r");
		outImgFile.open(outfile, "w");
		TiffDirectory* indir = inImgFile.getDirectory(0);
		TiffDirectory* outdir = outImgFile.createDirectory();
		outdir->copy(*indir);
		copyImageData(outdir, indir);

		// create SoundIFD
		TiffTagEntry* tagEntry = new TiffTagEntryT<tiff_uint32>(TIFFTAG_SOUNDIFD, EKTIFF_ULONG, 1, 0);
		outdir->setGenericTag(*tagEntry);
		delete tagEntry;
		TiffDirectory* adir = outdir->getSubDir(TIFFTAG_SOUNDIFD);

		tsize_t size = 88;
		unsigned char* buf = new unsigned char[size];
		printf("Writing audio data...\n");
		printf("Size: %ld\n", size);
		printf("Data: ");
		int i;
		for (i=0; i<size; i++)
		{
			buf[i] = (unsigned char)size - i;
			printf("%d,", buf[i]);
		}
		printf("\n");
		adir->writeAudio(buf, size);

		inImgFile.close();
		outImgFile.close();

		inImgFile.open(outfile, "r");
		indir = inImgFile.getDirectory(0);
		adir = indir->getSubDir(TIFFTAG_SOUNDIFD);
		memset(buf, 0, size);
		printf("Reading audio data...\n");
		size = adir->readAudio(buf, size);
		printf("Size: %ld\n", size);
		printf("Data: ");
		for (i=0; i<size; i++)
		{
			printf("%d,", buf[i]);
		}
		printf("\n");

		inImgFile.close();

		delete buf;
	}
	catch (TiffError &e)
	{
		cout << "EKC ERROR: " << e << endl;
	}
	catch (...)
	{
		cout << "OTHER ERROR" << endl;
	}
}

#ifdef __GNUC__
#define T_BINARY 0
#else
#define T_BINARY O_BINARY
#endif

void testMemoryIO()
{
	printf("\n\n****** Testing Memory IO:\n\n");

	char infile[] = "LS_None.tif";
	char outfile[] = "test.tif";
	int bufSize = 100000;

	try
	{
		TiffImageFile inFile, outFile;
		inFile.open(infile, "r");
		TiffDirectory *indir = inFile.getDirectory(0);

		// create in-mem image
		char* imgBuf = new char[bufSize];
		outFile.open(imgBuf, bufSize, "w");
		outFile.createDirectory();
		TiffDirectory *outdir = outFile.getDirectory(0);
		outdir->copy(*indir);
		copyImageData(outdir, indir);
		inFile.close();

		// write in-mem image
		tsize_t imgSize;
		outFile.close(&imgSize);
		int hFile;
//#if (defined _MSC_VER)
//		hFile = open(outfile, O_CREAT|O_WRONLY|T_BINARY|O_TRUNC, S_IREAD | S_IWRITE);
//#else
		hFile = open(outfile, O_CREAT|O_WRONLY|T_BINARY|O_TRUNC, S_IRUSR | S_IWUSR);
//#endif

		write(hFile, imgBuf, imgSize);
		close(hFile);

		// read in-mem image
		hFile = open(outfile, O_RDONLY|T_BINARY);
		struct stat sb;
		imgSize = (fstat(hFile, &sb) < 0)? 0: sb.st_size;
		read(hFile, imgBuf, imgSize);
		inFile.open(imgBuf, imgSize, "r");
		indir = inFile.getDirectory(0);
		indir->print();
		inFile.close();
		close(hFile);

		delete imgBuf;

	}
	catch (TiffError &e)
	{
		cout << "EKC ERROR: " << e << endl;
	}
	catch (...)
	{
		cout << "OTHER ERROR" << endl;
	}
}


#ifdef WIN32
void testInternetAccess()
{
	printf("\n\n****** Testing Internet Access:\n\n");

	HINTERNET hSession, hInet;
	if ((hSession = InternetOpen("EkTiff test",INTERNET_OPEN_TYPE_DIRECT,0,0,0)) == 0)
	{
		cout << "InternetOpen failed" << endl;
		return;
	}
	char url[] = "http://www.image.kodak.com/~lin/LS_None.tif";
	if ((hInet = InternetOpenUrl(hSession, url, NULL,-1L,INTERNET_FLAG_RAW_DATA,0)) == 0)
	{
		InternetCloseHandle(hSession);
		cout << "InternetOpenUrl failed" << endl;
		return;
	}
	
	try
	{
		TiffImageFile imgFile;
		imgFile.open(hInet, url);
		TiffDirectory* dir = imgFile.getDirectory(0);
		dir->print();
		imgFile.close();
	}
	catch (TiffError &e)
	{
		cout << "EKC ERROR: " << e << endl;
	}
	catch (...)
	{
		cout << "OTHER ERROR" << endl;
	}

	InternetCloseHandle(hInet);
	InternetCloseHandle(hSession);
}
#endif // WIN32

void test16Bits()
{
    cout << "\n\n***** Testing 16 bits image:\n\n";
    unsigned short codeValue = 1000;
    char inFile1[] = "LS_16.tif";
    char inFile2[] = "BS_16.tif";
    char outFile[] = "TestOut.tif";

    // --- test LS_16.tif
    printf("\n--- test 16 bits file %s for reading and writing\n", inFile1);
    TiffImageFile iImgFile, oImgFile;
    iImgFile.open(inFile1, "r");
    TiffDirectory* inDir = iImgFile.getDirectory(0);
    inDir->print();
    TiffImage* inImg = inDir->getImage();
    int size = ((TiffStripImage*)inImg)->scanlineSize();
    int length = inDir->imageLength();
    unsigned short* buf = new unsigned short[size/2];
    int row;

    oImgFile.open(outFile, "wb");
    TiffDirectory* outDir = oImgFile.createDirectory();
    outDir->copy(*inDir);
    TiffImage* outImg = outDir->getImage();

    for (row=0; row<length; row++)
    {
        ((TiffStripImage*)inImg)->readScanline(buf, row);
        for (int i=0; i<size/2; i++)
        {
            if (*(buf+i) != 1000)
            {
                printf("\nError: test16Bits() -- code value isn't 1000 at (%d, %d).\n", row, i);
                exit(0);
            }
        }
        if (outImg->needSwapForWriting())
            TiffUtil::swabArrayOfShort(buf, size/2);
        ((TiffStripImage*)outImg)->writeScanline(buf, row);
    }
    outImg->flush();
    iImgFile.close();
    oImgFile.close();

    // read back to verify
    oImgFile.open(outFile, "r");
    outDir = oImgFile.getDirectory(0);
    outImg = outDir->getImage();
    size = ((TiffStripImage*)outImg)->scanlineSize();
    length = outDir->imageLength();
    memset(buf, size, 0);
    for (row=0; row<length; row++)
    {
        ((TiffStripImage*)outImg)->readScanline(buf, row);
        for (int i=0; i<size/2; i++)
        {
            if (*(buf+i) != 1000)
            {
                printf("\nError: test16Bits() -- code value isn't 1000 at (%d, %d).\n", row, i);
                exit(0);
            }
        }
    }
    oImgFile.close();
    delete buf;

    // ---- test BS_16.tif
    printf("\n--- test 16 bits file %s for reading and writing\n", inFile2);
    iImgFile.open(inFile2, "r");
    inDir = iImgFile.getDirectory(0);
    inDir->print();
    inImg = inDir->getImage();
    size = ((TiffStripImage*)inImg)->scanlineSize();
    length = inDir->imageLength();
    buf = new unsigned short[size/2];

    oImgFile.open(outFile, "w");
    outDir = oImgFile.createDirectory();
    outDir->copy(*inDir);
    outImg = outDir->getImage();

    for (row=0; row<length; row++)
    {
        ((TiffStripImage*)inImg)->readScanline(buf, row);
        for (int i=0; i<size/2; i++)
        {
            if (*(buf+i) != 1000)
            {
                printf("\nError: test16Bits() -- code value isn't 1000 at (%d, %d).\n", row, i);
                exit(0);
            }
        }
        if (outImg->needSwapForWriting())
            TiffUtil::swabArrayOfShort(buf, size/2);
        ((TiffStripImage*)outImg)->writeScanline(buf, row);
    }
    outImg->flush();
    iImgFile.close();
    oImgFile.close();

    // read back to verify
    oImgFile.open(outFile, "r");
    outDir = oImgFile.getDirectory(0);
    outImg = outDir->getImage();
    size = ((TiffStripImage*)outImg)->scanlineSize();
    length = outDir->imageLength();
    for (row=0; row<length; row++)
    {
        ((TiffStripImage*)outImg)->readScanline(buf, row);
        for (int i=0; i<size/2; i++)
        {
            if (*(buf+i) != 1000)
            {
                printf("\nError: test16Bits() -- code value isn't 1000 at (%d, %d).\n", row, i);
                exit(0);
            }
        }
    }
    oImgFile.close();

    delete buf;
}

int main(int argc, char* argv[])
{
//	testReadWrite(TEST_BASIC);
	testReadWrite(TEST_ADVANCE);
	testAudio();
	testMemoryIO();
    test16Bits();

#ifdef WIN32
	testInternetAccess();
#endif

//	testTiffFile();

	return 0;
}
