/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 Module:       FGSensor.cpp
 Author:       Jon Berndt
 Date started: 9 July 2005

 ------------- Copyright (C) 2005 -------------

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 details.

 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 Place - Suite 330, Boston, MA  02111-1307, USA.

 Further information about the GNU General Public License can also be found on
 the world wide web at http://www.gnu.org.

FUNCTIONAL DESCRIPTION
--------------------------------------------------------------------------------

HISTORY
--------------------------------------------------------------------------------

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
COMMENTS, REFERENCES,  and NOTES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
INCLUDES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#include "FGSensor.h"

namespace JSBSim {

static const char *IdSrc = "$Id: FGSensor.cpp,v 1.2 2005/07/13 13:04:08 jberndt Exp $";
static const char *IdHdr = ID_SENSOR;

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
CLASS IMPLEMENTATION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/


FGSensor::FGSensor(FGFCS* fcs, Element* element) : FGFCSComponent(fcs, element)
{
  // inputs are read from the base class constructor

  dt = fcs->GetState()->Getdt();

  bits = quantized = divisions = 0;
  min = max = bias = noise_variance = lag = drift_rate = drift = span = 0.0;
  granularity = 0.0;
  noise_type = 0;

  Element* quantization_element = element->FindElement("quantization");
  if ( quantization_element) {
    if ( quantization_element->FindElement("bits") ) {
      bits = quantization_element->FindElementValueAsNumber("bits");
    }
    divisions = (1<<bits);
    if ( quantization_element->FindElement("min") ) {
      min = quantization_element->FindElementValueAsNumber("min");
    }
    if ( quantization_element->FindElement("max") ) {
      max = quantization_element->FindElementValueAsNumber("max");
    }
    span = max - min;
    granularity = span/divisions;
  }
  if ( element->FindElement("bias") ) {
    bias = element->FindElementValueAsNumber("bias");
  }
  if ( element->FindElement("drift_rate") ) {
    drift_rate = element->FindElementValueAsNumber("drift_rate");
  }
  if ( element->FindElement("lag") ) {
    lag = element->FindElementValueAsNumber("lag");
  }
  if ( element->FindElement("noise") ) {
    noise_variance = element->FindElementValueAsNumber("noise");
    string variation = element->FindElement("noise")->GetAttributeValue("variation");
    if (variation == "PERCENT") {
      NoiseType = ePercent;
    } else if (variation == "ABSOLUTE") {
      NoiseType = eAbsolute;
    } else {
      NoiseType = ePercent;
      cerr << "Unknown noise type in sensor: " << Name << endl;
      cerr << "  defaulting to PERCENT." << endl;
    }
  }

  FGFCSComponent::bind();

  Debug(0);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

FGSensor::~FGSensor()
{
  Debug(1);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

bool FGSensor::Run(void )
{
  Input = InputNodes[0]->getDoubleValue() * InputSigns[0];

  Output = Input; // perfect sensor

  // Degrade signal as specified

  if (lag != 0.0)            Lag();       // models sensor lag
  if (noise_variance != 0.0) Noise();     // models noise
  if (drift_rate != 0.0)     Drift();     // models drift over time
  if (bias != 0.0)           Bias();      // models a finite bias
  if (bits != 0)             Quantize();  // models quantization degradation
//  if (delay != 0.0)          Delay();     // models system signal transport latencies

  return true;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void FGSensor::Noise(void)
{
  double random_value = ((double)rand()/(double)RAND_MAX) - 0.5;

  switch( NoiseType ) {
  case ePercent:
    Output *= (1.0 + noise_variance*random_value);
    break;

  case eAbsolute:
    Output += noise_variance*random_value;
    break;
  }
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void FGSensor::Bias(void)
{
  Output += bias;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void FGSensor::Drift(void)
{
  drift += drift_rate*dt;
  Output += drift;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void FGSensor::Quantize(void)
{
  if (Output < min) Output = min;
  if (Output > max) Output = max;
  double portion = Output - min;
  quantized = (int)(portion/granularity);
  Output = quantized*granularity + min;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void FGSensor::Lag(void)
{
  // not done, yet
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//    The bitmasked value choices are as follows:
//    unset: In this case (the default) JSBSim would only print
//       out the normally expected messages, essentially echoing
//       the config files as they are read. If the environment
//       variable is not set, debug_lvl is set to 1 internally
//    0: This requests JSBSim not to output any messages
//       whatsoever.
//    1: This value explicity requests the normal JSBSim
//       startup messages
//    2: This value asks for a message to be printed out when
//       a class is instantiated
//    4: When this value is set, a message is displayed when a
//       FGModel object executes its Run() method
//    8: When this value is set, various runtime state variables
//       are printed out periodically
//    16: When set various parameters are sanity checked and
//       a message is printed out when they go out of bounds

void FGSensor::Debug(int from)
{
  if (debug_lvl <= 0) return;

  if (debug_lvl & 1) { // Standard console startup message output
    if (from == 0) { // Constructor

    }
  }
  if (debug_lvl & 2 ) { // Instantiation/Destruction notification
    if (from == 0) cout << "Instantiated: FGSensor" << endl;
    if (from == 1) cout << "Destroyed:    FGSensor" << endl;
  }
  if (debug_lvl & 4 ) { // Run() method entry print for FGModel-derived objects
  }
  if (debug_lvl & 8 ) { // Runtime state variables
  }
  if (debug_lvl & 16) { // Sanity checking
  }
  if (debug_lvl & 64) {
    if (from == 0) { // Constructor
      cout << IdSrc << endl;
      cout << IdHdr << endl;
    }
  }
}
}