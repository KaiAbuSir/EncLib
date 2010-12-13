//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#ifndef ENC_LIB_VERSION_H
#define ENC_LIB_VERSION_H

namespace Enc
{
    const int MajorVersion = 0;
    const int MinorVersion = 2;
    const int SubMinorVers = 1;
    const char * EncLibName = "ENClib";
#ifdef _DEBUG
    QString VersionString() {return QString("%1.%2.%3 DEBUG").arg(MajorVersion).arg(MinorVersion).arg(SubMinorVers);}
#else
    QString VersionString() {return QString("%1.%2.%3").arg(MajorVersion).arg(MinorVersion).arg(SubMinorVers);}
#endif
}

#endif