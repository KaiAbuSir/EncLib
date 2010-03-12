#include <QtCore/QFile>

#include "cell_parser_iso8211.h"
#include "iso8211_simple.h"

using namespace Enc;

CellParser8211::CellParser8211(unsigned long parseOptions) : parseOpts(parseOptions)
{}

CellParser8211::~CellParser8211()
{}

