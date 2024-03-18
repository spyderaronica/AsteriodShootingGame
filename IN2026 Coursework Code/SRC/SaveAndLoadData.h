#ifndef __SAVEANDLOADDATA_H__
#define __SAVEANDLOADDATA_H__

#include <string>
#include <fstream>
#include <sstream>
#include <vector>

class SaveAndLoadData
{
public:
	SaveAndLoadData();
	SaveAndLoadData(std::string filename);

	bool SaveData(std::pair<std::string, int> playerData);
	std::pair<std::string, int> LoadData(std::string playername);
	bool UpdateData(std::pair<std::string, int> playerData);

	~SaveAndLoadData();

private:
	std::string filename;

	bool FileExist(std::string filename);
	bool FileClear();
};

#endif