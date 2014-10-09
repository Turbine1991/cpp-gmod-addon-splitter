#include <iostream>
#include "Handler.h"
#include <stdio.h>

int main()
{
  Handler::Config.select("splitter");
  int maxsizemb = Handler::Config.get("maxsizemb", 200);
  string inputpath = Handler::Config.get("inputpath", (string)"input");
  string outputpath = Handler::Config.get("outputpath", (string)"output"), outputname = Handler::Config.get("outputname", (string)"addon");

  Handler handler(maxsizemb);
  if (handler.scan(inputpath)){
    handler.split(outputpath, outputname);}
  else
    cerr << "Splitting Failed!" << endl;

  //std::system("%gmad% create -folder css1 -out maps1.gma");
  //std::system("%pub% create -addon maps1.gma -icon css.jpg");

  return 0;
}
