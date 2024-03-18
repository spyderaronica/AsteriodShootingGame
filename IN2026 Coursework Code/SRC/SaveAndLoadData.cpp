#include "SaveAndLoadData.h"

SaveAndLoadData::SaveAndLoadData()
{
    SaveAndLoadData("Data.txt");
}

SaveAndLoadData::SaveAndLoadData(std::string filename)
{
    if (!FileExist(filename))
    {
        // Create a file object and open the file for writing
        std::fstream file(filename, std::ios::out);
        file.close();
    }
    this->filename = filename;
}

bool SaveAndLoadData::SaveData(std::pair<std::string, int> playerData)
{
    // Open the file
    std::fstream file(filename, std::ios::in | std::ios::out);

    // Check if the file is opened successfully
    if (!file.is_open()) {
        // File not found or unable to open
        return false;
    }

    // Read the contents of the file and store them in a temporary vector
    std::vector<std::pair<std::string, int>> tempData;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string name;
        int score;

        if (std::getline(iss, name, ',') && iss >> score) {
            tempData.push_back(std::make_pair(name, score));
        }
    }

    tempData.push_back(playerData);

    file.close();

    FileClear();

    file.open(filename, std::ios::in | std::ios::out);

    // Check if the file is opened successfully
    if (!file.is_open()) {
        // File not found or unable to open
        return false;
    }

    // Write the data to the file
    for (const auto& data : tempData) {
        file << data.first << "," << data.second << std::endl;
    }

    file.close();

    return true;
}

std::pair<std::string, int> SaveAndLoadData::LoadData(std::string playername)
{
    // Open the file for reading
    std::ifstream file(filename);

    // Check if the file is successfully opened
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            // Parse the line to extract player's name and score
            std::istringstream iss(line);
            std::string storedPlayerName;
            int playerScore;

            if (std::getline(iss, storedPlayerName, ',') && iss >> playerScore) {
                // Check if the stored player name matches the requested player name
                if (storedPlayerName == playername) {
                    // Close the file
                    file.close();

                    // Return the player data as a pair
                    return { playername, playerScore };
                }
            }
        }

        // Close the file
        file.close();
    }

    // Return an empty pair indicating failure
    return {};
}

bool SaveAndLoadData::UpdateData(std::pair<std::string, int> playerData)
{
    // Open the file
    std::fstream file(filename, std::ios::in | std::ios::out);

    // Check if the file is opened successfully
    if (!file.is_open()) {
        // File not found or unable to open
        return false;
    }

    // Read the contents of the file and store them in a temporary vector
    std::vector<std::pair<std::string, int>> tempData;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string name;
        int score;

        if (std::getline(iss, name, ',') && iss >> score) {
            tempData.push_back(std::make_pair(name, score));
        }
    }

    // Update the player's data in the temporary vector
    bool found = false;
    for (auto& data : tempData) {
        if (data.first == playerData.first) {
            data.second = playerData.second;
            found = true;
            break;
        }
    }

    // Close the file
    file.close();

    // If the player's data was found and updated, write the updated data to the file
    if (found) {
        FileClear();

        // Open the file
        file.open(filename, std::ios::in | std::ios::out);

        // Check if the file is opened successfully
        if (!file.is_open()) {
            // File not found or unable to open
            return false;
        }

        // Write the updated data to the file
        for (const auto& data : tempData) {
            file << data.first << "," << data.second << std::endl;
        }

        return true;
    }

    // Close the file
    file.close();

    // Player's data not found
    return false;
}

SaveAndLoadData::~SaveAndLoadData()
{
}

bool SaveAndLoadData::FileExist(std::string filename)
{
    std::ifstream file(filename);
    return file.good();
}

bool SaveAndLoadData::FileClear()
{
    std::ofstream file(filename, std::ofstream::trunc);

    // Check if the file is opened successfully
    if (file.is_open()) {
        file.close();
        return true;
    }

    return false;
}