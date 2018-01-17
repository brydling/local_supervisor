#ifndef PROCESSCONFIGFILE_H
#define PROCESSCONFIGFILE_H

#include <string>
#include <map>
#include <vector>

/* Class ProcessConfigFile
   (not process as in processing data, but process as in Windows process)
   
   This class handles parsing of the process config file, or shall we say that it processes the process config file?
   Maybe it should be called ProcessProcessConfigFile.
*/

class ProcessConfigFile {
public:

   struct Process_Type {
      std::string currentDir;
      std::string commandLine;
   };

   /* Constructor
      ARGS:
         filePath (in):
            The path to the config file containing our processes.
   */
   ProcessConfigFile(std::string filePath) { myFilePath = filePath; }

   /* Read the config file containing processes to be controllable by local_supervisor on this host.
      ARGS:
         processMap (out):
            A pointer to a map that will be filled with key-value-pairs (process-id, process-info)
      RETVAL:
         True on success. Currently the function cannot fail.
   */
   bool ReadProcesses(std::map<unsigned int, Process_Type> *processMap);

private:
   std::string myFilePath;
};

#endif