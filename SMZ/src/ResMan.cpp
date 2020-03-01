#include "ResMan.h"

#include <ios>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
using namespace std;

#include "constantsRender.h"
#include "constantsFallback.h"

ResMan::ResMan()
{
	if (!filesystem::exists("resources"))
		filesystem::create_directory("resources");

	if (!filesystem::exists("resources/levels"))
		filesystem::create_directory("resources/levels");
	if (!filesystem::exists("resources/textures"))
		filesystem::create_directory("resources/textures");

	// domyslny konstruktor tworzy iterator koncowy
	filesystem::directory_iterator end;

	filesystem::directory_iterator levelIter("resources/levels");
	while (levelIter != end)
	{
		if(levelIter->path().extension().string() == ".levelgeo")
			levelNames.push_back(levelIter->path().stem().string());
		levelIter++;
	}
	if (levelNames.size() == 0)
	{
		std::ofstream levout("resources/levels/fallback_level.levelgeo", std::ios::out);
		levout << cnst::fallbackLevel;
		levout.close();

		levelNames.push_back("fallback_level");
	}

	// najwyrazniej porzadek alfabetyczny nie jest gwarantowany, sortujemy znalezione nazwy
	std::sort(levelNames.begin(), levelNames.end());

	for (unsigned int i = 0; i < levelNames.size(); i++)
	{
		for (int j = 0; j < 2; j++)
		{
			if (!filesystem::exists("resources/textures/" + levelNames[i] + "_" + (char)(j + '0') + ".png"))
			{
				std::ofstream texout("resources/textures/" + levelNames[i] + "_" + (char)(j + '0') + ".png",
					std::ios::out | std::ios::binary);
				texout.write(reinterpret_cast<const char *>(cnst::fallbackTexture), sizeof(cnst::fallbackTexture));
				texout.close();
			}
		}
	}

	if (!filesystem::exists(cnst::interfacePath))
	{
		std::ofstream texout(cnst::interfacePath, std::ios::out | std::ios::binary);
		texout.write(reinterpret_cast<const char *>(cnst::fallbackTexture), sizeof(cnst::fallbackTexture));
		texout.close();
	}

	if (!filesystem::exists(cnst::cylinderPath))
	{
		std::ofstream objout(cnst::cylinderPath, std::ios::out);
		objout << cnst::fallbackUntexturedModel;
		objout.close();
	}
}
