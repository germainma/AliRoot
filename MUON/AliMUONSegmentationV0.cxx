/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/
/*
$Log$
Revision 1.3  2000/06/29 12:34:09  morsch
AliMUONSegmentation class has been made independent of AliMUONChamber. This makes
it usable with any other geometry class. The link to the object to which it belongs is
established via an index. This assumes that there exists a global geometry manager
from which the pointer to the parent object can be obtained (in our case gAlice).

Revision 1.2  2000/06/15 07:58:48  morsch
Code from MUON-dev joined

Revision 1.1.2.2  2000/06/12 07:57:23  morsch
include TMath.h

Revision 1.1.2.1  2000/06/09 21:30:33  morsch
AliMUONSegmentationV0 code  from  AliMUONSegResV0.cxx

*/

#include "AliMUONSegmentationV0.h"
#include "TArc.h"
#include "TMath.h"
#include "AliMUONChamber.h"
#include "AliRun.h"
#include "AliMUON.h"

ClassImp(AliMUONSegmentationV0)
    AliMUONSegmentationV0::AliMUONSegmentationV0(const AliMUONSegmentationV0& segmentation)
{
// Dummy copy constructor
}

    void AliMUONSegmentationV0::Init(Int_t  chamber)
{
//  Initialises member data of the segmentation from geometry data 
//  owned by Chamber
//
    AliMUON *pMUON  = (AliMUON *) gAlice->GetModule("MUON");
    AliMUONChamber* iChamber=&(pMUON->Chamber(chamber));
    
//  Initialise maximum number of pads in x ans y
    fNpx=(Int_t) (iChamber->ROuter()/fDpx+1);
    fNpy=(Int_t) (iChamber->ROuter()/fDpy+1);
//  Initialize inner and outer radius of the sensitive region     
    fRmin=iChamber->RInner();
    fRmax=iChamber->ROuter();    
    fCorr=0;
    
}


Float_t AliMUONSegmentationV0::GetAnod(Float_t xhit)
{
// Returns for a hit position xhit the position of the nearest anode wire    
    Float_t wire= (xhit>0)? Int_t(xhit/fWireD)+0.5:Int_t(xhit/fWireD)-0.5;
    return fWireD*wire;
}

void AliMUONSegmentationV0::SetPadSize(Float_t p1, Float_t p2)
{
//  Sets the padsize 
//  
    fDpx=p1;
    fDpy=p2;
}
void AliMUONSegmentationV0::
    GetPadI(Float_t x, Float_t y, Int_t &ix, Int_t &iy)
{
//  Returns pad coordinates (ix,iy) for given real coordinates (x,y)
//
    ix = (x>0)? Int_t(x/fDpx)+1 : Int_t(x/fDpx)-1;
    iy = (y>0)? Int_t(y/fDpy)+1 : Int_t(y/fDpy)-1;
    if (iy >  fNpy) iy= fNpy;
    if (iy < -fNpy) iy=-fNpy;
    if (ix >  fNpx) ix= fNpx;
    if (ix < -fNpx) ix=-fNpx;
}
void AliMUONSegmentationV0::
GetPadC(Int_t ix, Int_t iy, Float_t &x, Float_t &y)
{
//  Returns real coordinates (x,y) for given pad coordinates (ix,iy)
//
// Comments and Critics: 

//  The Pad(0,0) does not exist, this causes in the present version errors 
//  during iteration when used with hits close to zero.
//  Since we have frame crosses at  x=0 or y=0 this does not cause any problems
//  Nevertheless, should be corrected in the  next version !!
//  The name fRmin is misleading although we use this version with 
//  a circular chamber geometry.

    x = (ix>0) ? Float_t(ix*fDpx)-fDpx/2. : Float_t(ix*fDpx)+fDpx/2.;
    y = (iy>0) ? Float_t(iy*fDpy)-fDpy/2. : Float_t(iy*fDpy)+fDpy/2.;
}

void AliMUONSegmentationV0::
SetHit(Float_t xhit, Float_t yhit)
{
    //
    // Sets virtual hit position, needed for evaluating pad response 
    // outside the tracking program 
    fxhit=xhit;
    fyhit=yhit;
}

void AliMUONSegmentationV0::
SetPad(Int_t ix, Int_t iy)
{
    //
    // Sets virtual pad coordinates, needed for evaluating pad response 
    // outside the tracking program 
    GetPadC(ix,iy,fx,fy);
}

void AliMUONSegmentationV0::
FirstPad(Float_t xhit, Float_t yhit, Float_t dx, Float_t dy)
{
// Initialises iteration over pads for charge distribution algorithm
//
    //
    // Find the wire position (center of charge distribution)
    Float_t x0a=GetAnod(xhit);
    fxhit=x0a;
    fyhit=yhit;
    //
    // and take fNsigma*sigma around this center
    Float_t x01=x0a  - dx;
    Float_t x02=x0a  + dx;
    Float_t y01=yhit - dy;
    Float_t y02=yhit + dy;
    //
    // find the pads over which the charge distributes
    GetPadI(x01,y01,fixmin,fiymin);
    GetPadI(x02,y02,fixmax,fiymax);    
    // 
    // Set current pad to lower left corner
    fix=fixmin;
    fiy=fiymin;
    GetPadC(fix,fiy,fx,fy);
}

void AliMUONSegmentationV0::NextPad()
{
  // Stepper for the iteration over pads   
  // 
  // Comments and Critics:
  // Boundary crossing at x=0 or y=0 not correctly handled !
  // Step to next pad in the integration region
    if (fix != fixmax) {
	if (fix==-1) fix++;
	fix++;
    } else if (fiy != fiymax) {
	fix=fixmin;
	if (fiy==-1) fiy++;
	fiy++;
    } else {
	printf("\n Error: Stepping outside integration region\n ");
    }
    GetPadC(fix,fiy,fx,fy);
}

Int_t AliMUONSegmentationV0::MorePads()
// Stopping condition for the iterator over pads
//
// Are there more pads in the integration region ? 
{
    if (fix == fixmax && fiy == fiymax) {
	return 0;
    } else {
	return 1;
	
    }
}

void AliMUONSegmentationV0::SigGenInit(Float_t x,Float_t y,Float_t z)
{
//
//  Initialises pad and wire position during stepping
    fxt =x;
    fyt =y;
    GetPadI(x,y,fixt,fiyt);
    fiwt= (x>0) ? Int_t(x/fWireD)+1 : Int_t(x/fWireD)-1 ;
}

Int_t AliMUONSegmentationV0::SigGenCond(Float_t x,Float_t y,Float_t z)
{
//  Signal generation condition during stepping 
//  0: don't generate signal
//  1: generate signal 
//  Comments and critics: 

//  Crossing of pad boundary and mid plane between neighbouring wires is checked.
//  To correctly simulate the dependence of the spatial resolution on the angle 
//  of incidence signal must be generated for constant steps on 
//  the projection of the trajectory along the anode wire.

//
//  Signal will be generated if particle crosses pad boundary or
//  boundary between two wires. 
    Int_t ixt, iyt;
    GetPadI(x,y,ixt,iyt);
    Int_t iwt=(x>0) ? Int_t(x/fWireD)+1 : Int_t(x/fWireD)-1;
    if ((ixt != fixt) || (iyt !=fiyt) || (iwt != fiwt)) {
	return 1;
    } else {
	return 0;
    }
}
void AliMUONSegmentationV0::
IntegrationLimits(Float_t& x1,Float_t& x2,Float_t& y1, Float_t& y2)
{
//  Returns integration limits for current pad
//
    x1=fxhit-fx-fDpx/2.;
    x2=x1+fDpx;
    y1=fyhit-fy-fDpy/2.;
    y2=y1+fDpy;    
}

void AliMUONSegmentationV0::
Neighbours(Int_t iX, Int_t iY, Int_t* Nlist, Int_t Xlist[10], Int_t Ylist[10])
{
// Returns list of next neighbours for given Pad (iX, iY)
//
// Comments and critics
//     "Diagonal" neighbours are not passed
//      Is this ok when we search for local maxima ??
//      No test whether neighbours have valid indices id performed
    *Nlist=4;
    Xlist[0]=Xlist[1]=iX;
    Xlist[2]=iX-1;
    Xlist[3]=iX+1;
    Ylist[0]=iY-1;
    Ylist[1]=iY+1;
    Ylist[2]=Ylist[3]=iY;
}

Float_t AliMUONSegmentationV0::Distance2AndOffset(Int_t iX, Int_t iY, Float_t X, Float_t Y
, Int_t *dummy)
// Returns the square of the distance between 1 pad
// labelled by its Channel numbers and a coordinate
{
  Float_t x,y;
  GetPadC(iX,iY,x,y);
  return (x-X)*(x-X) + (y-Y)*(y-Y);
}


void  AliMUONSegmentationV0::GiveTestPoints(Int_t &n, Float_t *x, Float_t *y)
{
// Returns test point on the pad plane.
// Used during determination of the segmoid correction of the COG-method
    n=1;
    x[0]=(fRmax+fRmin)/2/TMath::Sqrt(2.);
    y[0]=x[0];
}

void AliMUONSegmentationV0::Draw()
{
// Draws the segmentation zones
//
    TArc *circle;
    Float_t scale=0.95/fRmax/2.;
    

    circle = new TArc(0.5,0.5,fRmax*scale,0.,360.);
    circle->SetFillColor(2);
    circle->Draw();

    circle = new TArc(0.5,0.5,fRmin*scale,0.,360.);
    circle->SetFillColor(1);
    circle->Draw();
}

AliMUONSegmentationV0& AliMUONSegmentationV0::operator =(const AliMUONSegmentationV0 & rhs)
{
// Dummy assignment operator
    return *this;
}
