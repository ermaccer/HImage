// himage.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>
#include <memory>
#include "image.h"

enum eModes {
	MODE_EXPORT = 1,
	MODE_CREATE,
};

int main(int argc, char* argv[])
{
	if (argc == 1) {
		std::cout << "HImage - a tool to convert Harvester image files by ermaccer (.BM)\n"
			<< "Usage: himage <params> <file>\n"
			<< "    -e              Extract\n"
			<< "    -p              Specifies PAL file\n";

		return 1;
	}

	int mode = 0;
	std::string o_param;
	std::string p_param;
	// params
	for (int i = 1; i < argc - 1; i++)
	{
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) {
			return 1;
		}
		switch (argv[i][1])
		{
		case 'e': mode = MODE_EXPORT;
			break;
		case 'o':
			i++;
			o_param = argv[i];
			break;
		case 'p':
			i++;
			p_param = argv[i];
			break;
		default:
			std::cout << "ERROR: Param does not exist: " << argv[i] << std::endl;
			break;
		}
	}

	if (mode == MODE_EXPORT)
	{
		std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open: " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		if (p_param.empty())
		{
			std::cout << "ERROR: Palette not specified!" << std::endl;
			return 1;
		}

		std::ifstream palFile(p_param, std::ifstream::binary);
		if (!palFile)
		{
			std::cout << "ERROR: Could not open: " << p_param.c_str() << "!" << std::endl;
			return 1;
		}



		harvester_bm bm;
		pFile.read((char*)&bm, sizeof(harvester_bm));
		unsigned int fileSize = std::filesystem::file_size(argv[argc - 1]);
		std::unique_ptr<char[]> imageData = std::make_unique<char[]>(fileSize);
		pFile.read(imageData.get(), fileSize);
		// 256 colors
		std::unique_ptr<rgb_pal_entry[]> palData = std::make_unique<rgb_pal_entry[]>(256);
		std::unique_ptr<rgbr_pal_entry[]> palData4 = std::make_unique<rgbr_pal_entry[]>(256);

		for (int i = 0; i < 256; i++)
			palFile.read((char*)&palData[i], sizeof(rgb_pal_entry));

		for (int i = 0; i < 256; i++)
		{
			palData4[i].r = palData[i].r;
			palData4[i].g = palData[i].g;
			palData4[i].b = palData[i].b;
			palData4[i].reserved = 0x00;
		}

		// create bmp
		bmp_header bmp;
		bmp_info_header bmpf;
		bmp.bfType = 'MB';
		bmp.bfSize = fileSize;
		bmp.bfReserved1 = 0;
		bmp.bfReserved2 = 0;
		bmp.bfOffBits = sizeof(bmp_header) + sizeof(bmp_info_header) + 1024;
		bmpf.biSize = sizeof(bmp_info_header);
		bmpf.biWidth = bm.width;
		bmpf.biHeight = bm.height;
		bmpf.biPlanes = 1;
		bmpf.biBitCount = 8;
		bmpf.biCompression = 0;
		bmpf.biXPelsPerMeter = 2835;
		bmpf.biYPelsPerMeter = 2835;
		bmpf.biClrUsed = 256;
		bmpf.biClrImportant = 0;


		std::string output = argv[argc - 1];
		output += ".bmp";

		std::ofstream oFile(output, std::ofstream::binary);

		oFile.write((char*)&bmp, sizeof(bmp_header));
		oFile.write((char*)&bmpf, sizeof(bmp_info_header));
		// swap red and blue 
		for (int i = 0; i < 256; i++)
		{
			char r = palData4[i].r;
			char b = palData4[i].b;

			palData4[i].r = b;
			palData4[i].b = r;
			oFile.write((char*)&palData4[i], sizeof(rgbr_pal_entry));
		}

		oFile.write(imageData.get(), fileSize);

		std::cout << "Saved as " << output.c_str() << std::endl;
		

	}
}