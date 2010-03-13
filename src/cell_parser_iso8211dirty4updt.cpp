//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include <QtCore/QFile>

#include "cell_parser_iso8211dirty4updt.h"
#include "cell_s57_update.h"
#include "cell_records.h"
#include "iso8211_simple.h"

using namespace Enc;

CellParser8211Dirty4Updt::CellParser8211Dirty4Updt(CellS57_Update * cell, unsigned long parseOptions) : CellParser8211Dirty(parseOptions)                      
{
    setCell(cell);
}
CellParser8211Dirty4Updt::CellParser8211Dirty4Updt(double factorXY, double factorZ, unsigned long parseOptions) : CellParser8211Dirty(parseOptions, factorXY, factorZ) 
{}

CellParser8211Dirty4Updt::~CellParser8211Dirty4Updt()
{}


void CellParser8211Dirty4Updt::setCell(CellS57_Header * cell)
{
    if (!cell)
    {
        cellS57 = NULL;
        return;
    }
    if (cell->getClass() != CellS57_Header::UpdtCellClass) 
    {
        throw QString("ERROR: Cannot set cell pointer: wrong class!");
    }
    cellS57 = dynamic_cast<CellS57_Update *>(cell);

    //** rem: update cell has no DSPM record, but maybe it already got the DSPM from its base-cell **
    if (cellS57->getDspm().getCOMF()) facXY = cellS57->getDspm().getCOMF();
    if (cellS57->getDspm().getSOMF()) facZ = cellS57->getDspm().getSOMF();
}

//*****************************************************************************
///
/*!
*
****************************************************************************** */
void CellParser8211Dirty4Updt::parseS57Cell(QString cellName)
{
    if (facXY == 0) throw QString("ERROR: Cannot parse %1, coordinate multiplication factor not set!").arg(cellName);
    if (facZ == 0)  throw QString("ERROR: Cannot parse %1, sounding multiplication factor not set!").arg(cellName);

    parseInit(cellName, cellS57);
    parseS57CellIntern(cellS57);
}

//*****************************************************************************
///
/*!
*
****************************************************************************** */
void CellParser8211Dirty4Updt::parseFeatureRecord(Iso8211fieldIterator & fieldIt)
{
    FeatureS57_Updt * feat = new FeatureS57_Updt;
    parseFeatureUpdtRecord(fieldIt, *feat);

    //** rember this Feature by Record-id and FeatObjID **
    cellS57->features.push_back(feat);    
}

//*****************************************************************************
/// 
/*!
*
****************************************************************************** */
void CellParser8211Dirty4Updt:: parseVectorRecord(Iso8211fieldIterator & fieldIt)
{
    unsigned int fieldInd = 0;
    unsigned int iso8211recid = 0;
    for (; fieldInd < fieldIt.fieldCount(); ++fieldInd)
    {
        int fieldSize;
        const char * tag = 0;
        const char * field = fieldIt.at(fieldInd, &fieldSize, tag);
        
        FieldVRID vrid; //1st Field after "0001" Field 
        SpatialS57 * updtSpatial = 0;
        //ISO8211 Record-id: is not really needed, only for iso8211 internally
        if (strncmp(tag, "0001", 4) == 0) 
        {
            if (fieldSize>4)
            {
                iso8211recid = ISO8211::bytes2ulong(field);  //this is against the S-57 Standard!!!
            }
            else iso8211recid = ISO8211::bytes2ushort(field); //normal case: recid is 2bytes, binary, unsigned
        }
        else if (strncmp(tag, "VRID", 4) == 0)
        {
            parseVRIDField(field, fieldSize,  vrid);
            //** Parse a Sounding (SEVERAL 3D-positions)**
            if (vrid.getRCNM() == RCNM_VI && fieldIt["SG3D"])
            {
                    updtSpatial = new SoundgS57_Updt(vrid, iso8211recid);
            }
            //** Parse a normal Node Spatial (bounding node or isolated node)**
            else if (vrid.getRCNM() == ISO8211::RCNM_VI || vrid.getRCNM() == ISO8211::RCNM_VC)
            {
                updtSpatial = new NodeS57_Updt(vrid, iso8211recid);
            }
            //** Parse a Edge Spatial **
            else if (vrid.getRCNM() == RCNM_VE)
            {
                updtSpatial = new EdgeS57_Updt(vrid, iso8211recid);
            }
            else
            {
                throw QString("ERROR: Cannot Parse Spatial Record: Neither Node nor Edge!").arg(ISO8211::makeRecNameASCII(vrid.getRCNM(), vrid.getRCID()));
            }
            parseVectorUpdtRecord(fieldIt, updtSpatial);
            cellS57->spatials.push_back(updtSpatial);
        }
    }
}

void CellParser8211Dirty4Updt::parseVectorUpdtRecord(Iso8211fieldIterator & fieldIt, SpatialS57 * updtSpatial)
{
    parseVectorUpdtFields(fieldIt, *reinterpret_cast<SpatialS57_Updt*>(updtSpatial)); //somehow bold  :)
    
    if (updtSpatial->getType() == Record8211::SoundingUPDT)
    {
        parseSndgRecord(fieldIt, *static_cast<SoundgS57_Updt*>(updtSpatial));
    }
    else if (updtSpatial->getType() == Record8211::NodeUPDT)
    {
        parseNodeRecord(fieldIt, *static_cast<NodeS57_Updt*>(updtSpatial));
    }
    else if (updtSpatial->getType() == Record8211::EdgeUPDT)
    {
        parseEdgeRecord(fieldIt, *static_cast<EdgeS57_Updt*>(updtSpatial));
    }
}

//*****************************************************************************
/// Parse Fields needed for Updating a Vector record ("ER" file)
/*!
* VRPC or SGCC may not be present, Update Records might have missing fields
****************************************************************************** */
void CellParser8211Dirty4Updt::parseVectorUpdtFields(Iso8211fieldIterator & fieldIt, SpatialS57_Updt & spatial_updat)
{
    const char * fieldPtr = fieldIt["VRPC"];
    if (fieldPtr) parseVRPCField(fieldPtr, 0, spatial_updat.getVrpc()); 
    fieldPtr = fieldIt["SGCC"];
    if (fieldPtr) parseSGCCField(fieldPtr, 0, spatial_updat.getSgcc()); 
}
