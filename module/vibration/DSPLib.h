#ifndef __DSPLIB_H__
#define __DSPLIB_H__

#ifdef __USE_IQMATHLIB__ // Include and use QmathLib and IQmathLib libraries

#include "QmathLib.h"
#include "IQmathLib.h"

#endif  //__USE_IQMATHLIB__

#include <vibration/DSPLib_types.h>               // Include DSPLib type definitions
#include <vibration/DSPLib_support.h>             // Include DSPLib support functions
#include <vibration/DSPLib_vector.h>              // Include DSPLib vector functions
#include <vibration/DSPLib_matrix.h>              // Include DSPLib matrix functions
#include <vibration/DSPLib_filter.h>              // Include DSPLib filter functions
#include <vibration/DSPLib_transform.h>           // Include DSPLib transform functions
#include <vibration/DSPLib_utility.h>             // Include DSPLib utility functions

#endif //__DSPLIB_H__
