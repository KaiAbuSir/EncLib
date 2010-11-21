#include "dictionaryS52.h"

//*****************************************************************************
/// Dictionay - just a dummy, will become a real dict later
/*!
  *
  *************************************************************************** */
DictionaryS52::DictionaryS52() : cnt(0)
{

    //**** just used 4 debug ****
    penVecDebug.push_back(QPen(QColor(0,0,0)));
    penVecDebug.push_back(QPen(QColor(125,0,0)));
    penVecDebug.push_back(QPen(QColor(250,0,0)));
    penVecDebug.push_back(QPen(QColor(0,125,0)));
    penVecDebug.push_back(QPen(QColor(0,250,0)));
    penVecDebug.push_back(QPen(QColor(0,0,125)));
    penVecDebug.push_back(QPen(QColor(0,0,250)));
    penVecDebug.push_back(QPen(QColor(20,50,100)));
    penVecDebug.push_back(QPen(QColor(100,50,20)));
    penVecDebug.push_back(QPen(QColor(50,100,20)));
    penVecDebug.push_back(QPen(QColor(20,100,50)));
    penVecDebug.push_back(QPen(QColor(50,20,100)));
    penVecDebug.push_back(QPen(QColor(100,20,50)));
}
