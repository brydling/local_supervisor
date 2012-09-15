#include "StdAfx.h"
#include "ProcessConfigFile.h"
#include "TokenizeLine.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using std::string;

bool ProcessConfigFile::ReadProcesses(std::map<unsigned int, Process_Type> *processMap) {
	std::ifstream configFile(myFilePath);
	std::string line;

	if (configFile.is_open()) {
		while(configFile.good()) {
			std::getline(configFile, line);
			if(!line.empty() && line[0] != '#') {
				std::vector<std::string> tokens = TokenizeLine(line, ';');
				if(tokens.size() >= 3) {
					Process_Type process;
					process.id = atoi(tokens[0].c_str());
					process.currentDir = tokens[1];
					for(int i=0; i<process.currentDir.length(); i++) {
						process.currentDir[i] = tolower(process.currentDir[i]);
					}
					process.commandLine = tokens[2];
					for(int i=0; i<process.commandLine.length(); i++) {
						process.commandLine[i] = tolower(process.commandLine[i]);
					}
					processMap->insert(std::pair<unsigned int, Process_Type>(process.id, process));
				}
			}
		}
		configFile.close();
	}

	return true;
}