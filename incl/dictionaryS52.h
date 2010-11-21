#ifndef DICTIONARYS52_H
#define DICTIONARYS52_H

#include <vector>

#include <QPen>

//*****************************************************************************
/// Dictionay - just a dummy, will become a real dict later
/*!
  *
  *************************************************************************** */
class DictionaryS52
{
public:
    DictionaryS52();
    QPen getPen() const
    {
      return penVecDebug[cnt % 13];
    }

private:
    int cnt;
    std::vector<QPen> penVecDebug;  //just used to make chart colorfull for easier debugging

};

#endif // DICTIONARYS52_H
