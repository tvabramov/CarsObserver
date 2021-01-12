///////////////////////////////////////////////////////////////////////////////
// Hungarian.h: Header file for Class HungarianAlgorithm.
//
// This is a C++ wrapper with slight modification of a hungarian algorithm
// implementation by Markus Buehren. The original implementation is a few
// mex-functions for use in MATLAB, found here:
// http://www.mathworks.com/matlabcentral/fileexchange/6543-functions-for-the-rectangular-assignment-problem
//
// Both this code and the orignal code are published under the BSD license.
// by Cong Ma, 2016
//

#ifndef VERIFIERS_HUNGARIAN_H
#define VERIFIERS_HUNGARIAN_H

#include <vector>

namespace Hungarian {

double Solve(std::vector<std::vector<double>>& DistMatrix,
             std::vector<int>& Assignment);

}

#endif  // VERIFIERS_HUNGARIAN_H
