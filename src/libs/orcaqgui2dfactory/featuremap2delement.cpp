#include "featuremap2delement.h"
#include <iostream>
#include <orcaqgui/exceptions.h>
#include <QFileDialog>

using namespace std;

namespace orcaqgui {

FeatureMap2dElement::FeatureMap2dElement( const orcaice::Context  &context,
                                          const std::string       &proxyString,
                                          IHumanManager*           humanManager )
    : PtrIceStormElement<FeatureMap2dPainter,
                      orca::FeatureMap2dData,
                      orca::FeatureMap2dDataPtr,
                      orca::FeatureMap2dPrx,
                      orca::FeatureMap2dConsumer,
                      orca::FeatureMap2dConsumerPrx>(context, proxyString, painter_, -1 ),
      humanManager_(humanManager),
      featureMapFileName_("/home"),
      featureMapFileNameSet_(false)
{
}

QStringList
FeatureMap2dElement::contextMenu()
{
    QStringList s;
    s<<"Toggle Uncertainty"<<"Toggle Feature Numbers"<<"Save Feature Map"<<"Save Feature Map As";
    return s;
}

void
FeatureMap2dElement::execute( int action )
{
    switch ( action )
    {
    case 0 :
    {
        painter_.toggleUncertainty();
        break;
    }
    case 1 :
    {
        painter_.toggleFeatureNumbers();
        break;
    }
    case 2 :
    {
        saveFeatureMap();
        break;
    }
    case 3 :
    {
        saveFeatureMapAs();
        break;
    }
    default:
    {
        throw orcaqgui::Exception( "FeatureMap2dElement::execute(): What the hell? bad action." );
        break;
    }
    }
}

void FeatureMap2dElement::saveFeatureMapAs()
{
    featureMapFileName_ = QFileDialog::getSaveFileName(
        0,
        //this,
        "Choose a filename to save under",
        featureMapFileName_,
        "*.txt");

    if ( featureMapFileName_ != "" )
    {
        painter_.saveMap( featureMapFileName_, humanManager_ );
        featureMapFileNameSet_ = true;
    }
}

void FeatureMap2dElement::saveFeatureMap()
{
    if (!featureMapFileNameSet_)
    {   
        saveFeatureMapAs();
    }
    else
    {
        painter_.saveMap( featureMapFileName_, humanManager_ );
    }
}

}
