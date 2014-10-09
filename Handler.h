#pragma once

#include <string>
#include <stdint.h>
#include <INI.h>
#include "Global.h"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <list>
#include <vector>
#include <sstream>
#include <algorithm>

using std::string;

using namespace std;
#include <iostream>

class File
{
public:
  typedef uintmax_t size_t;

  string dir;
  string file;
  size_t size;

  File(string dir, string file, size_t size);
  string getFilename();
};

class Handler
{
public:
  typedef INI<string, string, string> ini_t;

  static ini_t Config;

  const File::size_t MAX_SIZE;

  list<File> files;
  vector<string> ignore;
  vector<string> ignoreClean;
  string baseDir;
  File::size_t percent, percentAddon;
  unsigned int filesDone, fileCount;
  string from;

  Handler(File::size_t maxSize);
  bool validExtension(string ext);
  bool scan(string in);
  void splitsingle(string outDir);
  void split(string outDir, string baseDir);
  template<class T>
    void strexplode(T& into, string str2, const char* del);
  void strclean(string& str, char chars[]);
};
