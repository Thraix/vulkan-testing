#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <execinfo.h>
#include <link.h>
#include <stdlib.h>
#include <stdio.h>

struct Utils
{

  // converts a function's address in memory to its VMA address in the executable file. VMA is what addr2line expects
  static size_t ConvertToVMA(size_t addr)
  {
    Dl_info info;
    link_map* link_map;
    dladdr1((void*)addr,&info,(void**)&link_map,RTLD_DL_LINKMAP);
    return addr-link_map->l_addr;
  }

  static void PrintCallStack()
  {
    void *callstack[128];
    int frame_count = backtrace(callstack, sizeof(callstack)/sizeof(callstack[0]));
    for (int i = 0; i < frame_count; i++)
    {
      Dl_info info;
      if(dladdr(callstack[i],&info))
      {
        // use addr2line; dladdr itself is rarely useful (see doc)
        char command[256];
        size_t VMA_addr=ConvertToVMA((size_t)callstack[i]);
        //if(i!=crash_depth)
          VMA_addr-=1;    // https://stackoverflow.com/questions/11579509/wrong-line-numbers-from-addr2line/63841497#63841497
        snprintf(command,sizeof(command),"addr2line -e %s -Ci %zx",info.dli_fname,VMA_addr);
        system(command);
      }
    }
  }
  static void AbortTrace()
  {
    PrintCallStack();
    abort();
  }

  static void LogBytes(const std::vector<char>& data)
  {
    std::cout << std::hex;
    for(size_t i = 0; i < data.size(); i++)
    {
      std::cout << std::setw(2) << std::setfill('0') << (int32_t)(uint8_t)(data[i]);
      if(i % 16 == 15)
      {
        std::cout << " ";
        for(size_t j = i - 15; j < i+1; j++)
        {
          if((data[j] >= 'a' && data[j] <= 'z') || (data[j] >= 'A' && data[j] <= 'Z') || (data[j] >= '0' && data[j] <= '9') || data[j] == '_')
            std::cout << data[j];
          else
            std::cout << '.';
        }
        std::cout << std::endl;
      }
    }
    std::cout << std::dec;
  }
};
