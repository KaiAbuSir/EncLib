//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include "cell_record_fields.h"
#include "cell_parser_iso8211dirty.h"

#include <QtCore/QVariant>

#include "iso8211.h"
#include "cpl_vsi.h"
using namespace Enc;
 
//const int FieldDSID::MemberCnt;
//const int FieldDSSI::MemberCnt;
//const int FieldDSPM::MemberCnt;

FieldDSID::FieldDSID() : RCNM(0), RCID(0), EXPP(0), INTU(0), EDTN(-1), UPDN(-1), PRSP(0), PROF(0), AGEN(0)
{
    //** init Update/Issue date with 8 valid chars - They Must always be 8 Chars long! **
    memset(UADT,' ',8);
    UADT[8]=0;
    memset(ISDT,' ',8);
    ISDT[8]=0;
    //** init STED with 4 valid chars - STED must always be 4 Chars long! **
    memset(STED ,' ',4);
    STED[4]=0;
}
FieldDSID::FieldDSID(const FieldDSID & cpDSID)
{
    *this = cpDSID;
}

//******************************************************************************
/// index-operator: use this Struct like a vector :-)
/*!
***************************************************************************** */
QVariant FieldDSID::operator[](int index)const
{
    if (index == COL_RCNM) return RCNM;
    else if (index == COL_RCID) return (uint)RCID; //QVariant has problems with unsigned long
    else if (index == COL_EXPP) return EXPP;
    else if (index == COL_INTU) return INTU;
    else if (index == COL_DSNM) return DSNM; 
    else if (index == COL_EDTN) return EDTN;
    else if (index == COL_UPDN) return UPDN;
    else if (index == COL_UADT) return UADT;
    else if (index == COL_ISDT) return ISDT;
    else if (index == COL_STED) return STED;
    else if (index == COL_PRSP) return PRSP;
    else if (index == COL_PSDN) return PSDN;
    else if (index == COL_PRED) return PRED;
    else if (index == COL_PROF) return PROF;
    else if (index == COL_AGEN) return AGEN;
    else if (index == COL_COMT) return COMT;
    return QVariant();
}
FieldDSID & FieldDSID::operator=(const FieldDSID & otherDSID)
{
    if (this == &otherDSID) *this;

    RCNM = otherDSID.RCNM;
    RCID = otherDSID.RCID;  //Record ID - unimportant
    EXPP = otherDSID.EXPP;  //Exchange purpose - 'N' (new) or 'R' (revision)
    INTU = otherDSID.INTU;  //usage
    EDTN = otherDSID.EDTN;  //Edition Number
    UPDN = otherDSID.UPDN;  //Update Number

    DSNM = otherDSID.DSNM;  //Data Set Name
    PSDN = otherDSID.PSDN;  //Product Specification description - unimportant
    PRED = otherDSID.PRED;  //Product Specification edition number - unimportant
    COMT = otherDSID.COMT;  //Comment

    PRSP = otherDSID.PRSP;  //Product Specification - normally "ENC" 
    PROF = otherDSID.PROF;  //Application profile identification - "EN" "ER" or "DD" 
    AGEN = otherDSID.AGEN;  //Producing agency

    strncpy(UADT, otherDSID.UADT, sizeof(UADT));  //Update application Date
    strncpy(ISDT, otherDSID.ISDT, sizeof(ISDT));  //Issue Date
    strncpy(STED, otherDSID.STED, sizeof(STED));  //Edition Number S-57 always "03.1" - unimportant

    return *this;
}

bool FieldDSID::operator==(const FieldDSID & otherDSID) const
{
    //**** at first: simple type compares (fast)
    if (RCNM != otherDSID.RCNM ||
        RCID != otherDSID.RCID ||  //Record ID - unimportant
        EXPP != otherDSID.EXPP ||  //Exchange purpose - 'N' (new) or 'R' (revision)
        INTU != otherDSID.INTU ||  //Usage
        EDTN != otherDSID.EDTN ||  //Edition Number
        UPDN != otherDSID.UPDN ||  //Update Number
        PRSP != otherDSID.PRSP ||  //Product Specification - normally "ENC" 
        PROF != otherDSID.PROF ||  //Application profile identification - "EN" "ER" or "DD" 
        AGEN != otherDSID.AGEN)    //Producing agency
            return false;

    //**** then char compares, slower ****
    if (strncmp(UADT, otherDSID.UADT, sizeof(UADT))!= 0 ||  //Update application Date
        strncmp(ISDT, otherDSID.ISDT, sizeof(ISDT))!= 0 ||  //Issue Date
        strncmp(STED, otherDSID.STED, sizeof(STED))!= 0)    //Edition Number S-57 always "03.1" - unimportant
            return false;

    //**** even slower: QString compares ****
    if(DSNM != otherDSID.DSNM ||  //Data Set Name
       PSDN != otherDSID.PSDN ||  //Product Specification description - unimportant
       PRED != otherDSID.PRED ||  //Product Specification edition number - unimportant
       COMT != otherDSID.COMT)  //Comment
            return false;

    return true;
}

unsigned long FieldDSID::getIso8211Length() const
{
    return 31                   //all fixed length values 
           +DSNM.length() +1    //dsnm + UT
           +Digits4EditionNo +1 //EDTN: Assuming: 4 digits + UT
           +Digits4UpdateNo +1  //UPDT: Assuming: 3 digits + UT
           +PSDN.length() +1    //PSDN +UT
           +PRED.length() +1    //PRED +UT
           +COMT.length() +1    //Comment + UT
           +1;                  //Final FT
}

//******************************************************************************
//******************************************************************************
/// Struct containig the data (subfields) of the DSSI Field 
/*! 
* found in S-57 cell header record: first record, second field
***************************************************************************** */
//******************************************************************************
FieldDSSI::FieldDSSI() : DSTR(0), AALL(0), NALL(0), NOMR(0), NOCR(0), NOGR(0), NOLR(0), NOIN(0), NOCN(0), NOED(0), NOFA(0)
{}

QVariant FieldDSSI::operator[](int index)const
{
    if (index == COL_DSTR) return DSTR;
    else if (index == COL_AALL) return AALL; 
    else if (index == COL_NALL) return NALL; 
    else if (index == COL_NOMR) return NOMR; 
    else if (index == COL_NOCR) return NOCR; 
    else if (index == COL_NOGR) return NOGR; 
    else if (index == COL_NOLR) return NOLR; 
    else if (index == COL_NOIN) return NOIN; 
    else if (index == COL_NOCN) return NOCN; 
    else if (index == COL_NOED) return NOED; 
    else if (index == COL_NOFA) return NOFA; 
    return QVariant();
}


unsigned long FieldDSSI::getIso8211Length() const
{
    return 36;
}
//******************************************************************************
//******************************************************************************
/// Struct containig the data (subfields) of the DSPM Field 
/*! 
* found in S-57 cell header record: second record, first field
***************************************************************************** */
//******************************************************************************
FieldDSPM::FieldDSPM() : RCNM(0), RCID(0), HDAT(0), VDAT(0), SDAT(0), CSCL(0), DUNI(0), HUNI(0), PUNI(0), COMF(0), SOMF(0)
{}

QVariant FieldDSPM::operator[](int index)const
{
    if (index == COL_RCNM) return COL_RCNM;
    else if (index == COL_RCID) return (uint)RCID; 
    else if (index == COL_HDAT) return HDAT; 
    else if (index == COL_VDAT) return VDAT; 
    else if (index == COL_SDAT) return SDAT; 
    else if (index == COL_CSCL) return (uint)CSCL; 
    else if (index == COL_DUNI) return DUNI; 
    else if (index == COL_HUNI) return HUNI; 
    else if (index == COL_PUNI) return PUNI; 
    else if (index == COL_COUN) return COUN; 
    else if (index == COL_COMF) return (uint)COMF; 
    else if (index == COL_SOMF) return (uint)SOMF; 
    else if (index == COL_COMT) return COMT; 
    return QVariant();
}

unsigned long FieldDSPM::getIso8211Length() const
{
    return 24 + COMT.length() +1 +1 ;
}