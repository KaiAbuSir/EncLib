#include <QtCore/QFile>

#include "cell_parser_iso8211dirty4base.h"
#include "cell_s57_base.h"
#include "cell_records.h"
#include "iso8211_simple.h"

using namespace Enc;

//*****************************************************************************
///
/*!
*
****************************************************************************** */
CellParser8211Dirty4Base::CellParser8211Dirty4Base(CellS57_Base * cell, unsigned long parseOptions) : CellParser8211Dirty(parseOptions)
{
    setCell(cell);
}

CellParser8211Dirty4Base::~CellParser8211Dirty4Base()
{}

//*****************************************************************************
///
/*!
*
****************************************************************************** */
void CellParser8211Dirty4Base::setCell(CellS57_Header * cell)
{
    if (!cell)
    {
        cellS57 = NULL;
        return;
    }
    if (cell->getClass() != CellS57_Header::BaseCellClass) 
    {
        throw QString("ERROR: Cannot set cell pointer: wrong class!");
    }
    cellS57 = dynamic_cast<CellS57_Base *>(cell);
}

//*****************************************************************************
///
/*!
*
****************************************************************************** */
void CellParser8211Dirty4Base::parseS57Cell(QString cellName)
{
    parseInit(cellName, cellS57);
    parseS57CellIntern(cellS57);
#ifdef _DEBUG
    printf("DEBUG: parsed: %d Features, %d iNodes, %d bNodes, %d Edges, %d 3Dcluster \n", 
        cellS57->getFeatures().size(), cellS57->getINodes().size(), cellS57->getBNodes().size(), cellS57->getEdges().size(), cellS57->getSoundingds().size());
#endif
}

//*****************************************************************************
///
/*!
*
****************************************************************************** */
void CellParser8211Dirty4Base::parseFeatureRecord(Iso8211fieldIterator & fieldIt)
{
    FeatureS57 * feat = new FeatureS57;
    CellParser8211Dirty::parseFeatureRecord(fieldIt, *feat);

    //** rember this Feature by Record-id and FeatObjID **
    cellS57->features[feat->getFRID().getRCID()] = feat;
    LongNAMe foidHandle = feat->getFOID().getLongNAMe();
    if (foidHandle == 0)
    {
        cellS57->checkResult.featMissingFOID.push_back(feat->getFRID().getRCID() );
    }
    cellS57->foid2rcid.insert(std::pair <LongNAMe , unsigned long>(foidHandle, feat->getFRID().getRCID()));
}

//*****************************************************************************
/// 
/*!
*
****************************************************************************** */
void CellParser8211Dirty4Base::parseVectorRecord(Iso8211fieldIterator & fieldIt)
{
    unsigned int fieldInd = 0;
    unsigned int iso8211recid = 0;
    for (; fieldInd < fieldIt.fieldCount(); ++fieldInd)
    {
        int fieldSize;
        const char * tag = 0;
        const char * field = fieldIt.at(fieldInd, &fieldSize, tag);
        
        FieldVRID vrid; //1st Field after "0001" Field 
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
            //** Parse a Node Spatial: might be node (ONE 2D-position) or Sounding (SEVERAL 3D-positions)**
            if (vrid.getRCNM() == ISO8211::RCNM_VI)
            {
                if (fieldIt["SG3D"])
                {
                    SoundgS57 * sndgRec = new SoundgS57(iso8211recid);
                    sndgRec->setVRID(vrid);
                    parseSndgRecord(fieldIt, *sndgRec);
                    cellS57->soundings[sndgRec->getVRID().getRCID()] = sndgRec;
                }
                else
                {
                    NodeS57 * nodeRec = new NodeS57(iso8211recid);
                    nodeRec->setVRID(vrid);
                    parseNodeRecord(fieldIt, *nodeRec);
                    cellS57->iNodes[nodeRec->getVRID().getRCID()] = nodeRec;
                }
            }
            //** Parse a Bounding Node Spatial (parsing is same as or isoNodes, but has pointers to Edges) **
            else if (vrid.getRCNM() == ISO8211::RCNM_VC)
            {
                BoundNodeS57 * nodeRec = new BoundNodeS57(iso8211recid);
                nodeRec->setVRID(vrid);
                parseNodeRecord(fieldIt, *nodeRec);
                cellS57->bNodes[nodeRec->getVRID().getRCID()] = nodeRec;
            }
            //** Parse a Edge Spatial **
            else if (vrid.getRCNM() == ISO8211::RCNM_VE)
            {
                EdgeS57 * edgeRec = new EdgeS57(iso8211recid);
                edgeRec->setVRID(vrid);
                parseEdgeRecord(fieldIt, *edgeRec);
                cellS57->edges[edgeRec->getVRID().getRCID()] = edgeRec;
            }
            else
            {
                throw QString("ERROR: Cannot Parse Spatial Record: Neither Node nor Edge!");
            }

        }
    }
}
