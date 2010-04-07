//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include "cell_records.h"


using namespace Enc;

//const int FieldFRID::MemberCnt;   

const DegBBox & SoundgS57::getBoundingBox()
{
    if (!bBox.isValid())
    {
        for (int i =0; i < SG3Dvec.size(); i += 3)
        {
            bBox.add(SG3Dvec[i], SG3Dvec[i+1]);
        }
    }
    return bBox;
}

