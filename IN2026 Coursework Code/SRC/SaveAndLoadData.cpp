#include "SaveAndLoadData.h"

SaveAndLoadData::SaveAndLoadData()
{
    SaveAndLoadData("Data.txt");
}

SaveAndLoadData::SaveAndLoadData(std::string filename)
{
    if (FileExist(filename))
    {
        this->filename = filename;
    }
    else
    {
        // Create a file object and open the file for writing
        std::fstream file(filename, std::ios::out);
        file.close();
    }
}

bool SaveAndLoadData::SaveData(std::pair<std::string, int> playerData)
{
    // Open the file for writing in append mode
    std::ofstream file(filename, std::ios::app);

    // Check if the file is successfully opened
    if (file.is_open()) {
        // Write player's name and score to the file
        file << playerData.first << "," << playerData.second << std::endl;
        // Close the file
        file.close();
        return true; // Return true indicating success
    }
    else
        return false; // Return false indicating failure
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
            char comma;
            if (iss >> storedPlayerName >> comma >> playerScore && comma == ',') {
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
    // Open the file in read mode
    std::fstream file(filename, std::ios::in | std::ios::out);

    // Check if the file is opened successfully
    if (!file.is_open()) {
        // File not found or unable to open
        return false;
    }

    // Search for the player's data in the file
    std::string line;
    while (std::getline(file, line)) {
        // Split the line into player name and score
        std::istringstream iss(line);
        std::string playerName;
        int score;
        iss >> playerName >> score;

        // Check if the player name matches
        if (playerName == playerData.first) {
            // Update the score
            file.seekp(file.tellg()); // Move write position to the current line
            file << playerData.first << " " << playerData.second << std::endl;
            file.close(); // Close the file
            return true; // Data updated successfully
        }
    }

    // Player data not found in the file
    file.close(); // Close the file
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