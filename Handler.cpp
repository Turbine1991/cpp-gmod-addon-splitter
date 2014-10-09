#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <list>
#include <vector>
#include <sstream>
#include <algorithm>
#include "Handler.h"

using namespace std;

Handler::ini_t Handler::Config("config.ini", true);

///Public
//File
File::File(string dir, string file, size_t size):
  dir(dir), file(file), size(size)
{

}

string File::getFilename()
{
   return dir + "/" + file;
}

//Handler
bool Handler::validExtension(string ext)
{
   //ext == ".json" || ext == ".psd" || ext == ".vcproj" || ext == ".svn" || ext == ".dxf" || ext == ".vmf" || ext == ".vmx" || ext == ".3ds")
   if (std::count(ignoreClean.begin(), ignoreClean.end(), ext) > 0 or ext == ".json")
      return false;

   return true;
}

bool Handler::scan(string in)
{
   int trimLength = 0;
   bool first = true;
   File::size_t fileSize = 0;
   fileCount = 0;
   from = in;

   for(filesystem::recursive_directory_iterator dir_end, dir(in); dir != dir_end; ++dir)
   {
      try
      {
         filesystem::path p(*dir);
         if (first && filesystem::is_directory(p))
         {
            baseDir = p.parent_path().generic_string();
            trimLength = baseDir.length() + 1;
            first = false;
         }
         else if (!filesystem::is_directory(p))
         {
            string temp = p.parent_path().generic_string();

            if (temp != baseDir && baseDir != "" && validExtension(p.extension().generic_string()))
            {
               fileSize = file_size(p);

               if (fileSize > MAX_SIZE)
               {
                  cerr << "File exceeds split limit: " << p.filename().generic_string() << endl;
                  cout << "Continue without file? (y/n) ";
                  char c = getchar();
                  if (c == 'n')
                     return false;
               }
               else
               {
                  temp.erase(0, trimLength);  //Stop it coming to this again
                  files.push_back(File(temp, p.filename().generic_string(), fileSize));

                  fileCount++;
               }
            }
         }
      }

      catch(filesystem::filesystem_error e)
      {
         cout << e.what() << endl;
      }
   }

   percentAddon = fileCount / 100;

   return true;
}

void Handler::splitsingle(string outDir)
{
   if (filesystem::exists(outDir))
   {
      filesystem::remove_all(outDir);
      cout << "Deleted existing folder." << endl;
   }

   File::size_t curSize = 0;
   for(auto it = files.begin(); it != files.end();)
   {
      File& file = (*it);

      if (curSize + file.size <= MAX_SIZE)
      {
         string outFilename = outDir + "/" + file.getFilename();
         string baseFilename = baseDir + "/" + file.getFilename();
         string dir = outDir + "/" + file.dir;

         if (!filesystem::exists(dir))
            filesystem::create_directories(dir);

         filesystem::copy(baseFilename, outFilename);

         curSize += file.size;
         filesDone++;
         files.erase(it++);

         if (percentAddon > 0 && filesDone % percentAddon == 0 && filesDone > 0)
         {
            percent++;
            cout << percent << "%" << endl;
         }
      }
      else
         it++;
   }

   cout << "Final Size: " << curSize / 1024 << "kb" << endl;
}

void Handler::split(string outDir, string baseDir)
{
   int count = 1;
   File::size_t lastCount;
   percent = 0;
   string dname = baseDir;
   baseDir = outDir + "/" + baseDir;

   while((lastCount = fileCount - filesDone) > 0)
   {
      string num = Converters::Convert<string>(count);
      string pname = baseDir + num;

      splitsingle(pname);

      ///Save addon.json
      Config.select("addon");

      string title = Config.get("title", "ExampleAddon")  + " \%" + num;
      string type = Config.get("type", "map");
      string ignoreStr = "\n\t[\n";

      for(unsigned int i = 0; i < ignore.size(); i++)
      {
         if (i > 0)
            ignoreStr += ",\n";

         ignoreStr += (string)"\t\t\"" + ignore[i] + "\"";
      }

      std::ofstream file(pname + "/" + "addon.json");
      file << str(boost::format("{\n\t\"title\" \t: \"%1%\",\n\t\"type\"  \t: \"%2%\",\n\t\"ignore\"\t: %3%\n\t]\n}") % title % type % ignoreStr);
      file.close();
      ///

      ///Make an addon
      Config.select("splitter");

      string spath = "%programfiles%/Steam";
      if (!filesystem::exists(spath))
      spath = "%programfiles(x86)%/Steam";

      string bin = Config.get("gmodbin", spath + "/steamapps/common/GarrysMod/bin/");

      if (Config.get("gma", "true") == "true")
      {
         if (!filesystem::exists(bin))
            cerr << "Please specify valid GMOD bin folder: " + bin << endl;
         else
         {
            string fname = outDir + "/" + dname + num + ".gma";
            string tmp = str(boost::format("%1%./gmad create -folder %2% -out %3%") % bin % (baseDir + num) % (outDir + "/" + dname + num + ".gma"));
            std::system(tmp.c_str());

            if (Config.get("upload", "true") == "true")
            {
               Config.select("addon");
               string image = from + "/" + Config.get("image", "addon.jpg");
               string tmp = str(boost::format("%1%./gmpublish create -addon %2% -icon %3%") % bin % (outDir + "/" + dname + num + ".gma") % image);
               std::system(tmp.c_str());
            }
         }
      }

      count++;
   }

   cout << "Parts: " << count - 1 << endl;
   cout << "Outputted: " << baseDir << endl;
}

template<class T>
   void Handler::strexplode(T& into, string str2, const char* del)
{
   string str = "" + str2;  //Make copy
   for (char* p = strtok((char*)str.c_str(), del);  p;  p = strtok(NULL, del))
      into.push_back(p);
}

void Handler::strclean(string& str, char chars[])
{
   for (size_t i = 0; i < strlen(chars); ++i)
      str.erase(std::remove(str.begin(), str.end(), chars[i]), str.end());
}

Handler::Handler(File::size_t maxSize):
 MAX_SIZE(maxSize * 1024 * 1024), baseDir(""), percent(0), percentAddon(0), filesDone(0), fileCount(0), from("")
{
   Config.select("addon");

   string str = Config.get("ignore", "*.psd,*.vcproj*,*.svn*,*.dxf,*.vmf,*.vmx,*.3ds");
   strexplode(ignore, str, ",");
   strclean(str, (char*)"*");
   strexplode(ignoreClean, str, ",");  //We're not using regex, just the * wildcard before extension
}
