/***************************************************************
 *
 * Copyright (C) 1990-2007, Condor Team, Computer Sciences Department,
 * University of Wisconsin-Madison, WI.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You may
 * obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************/


#ifndef __CONVERSION_H__
#define __CONVERSION_H__

#define WANT_NAMESPACES
#include "condor_fix_iostream.h"
#include "classad_distribution.h"
#include "condor_classad.h"

classad::ClassAd * toNewClassAd( ClassAd * );

ClassAd * toOldClassAd( classad::ClassAd * );

//classad::ExprTree *AddExplicitConditionals( classad::ExprTree *expr );

// AddExplicitTargets creates a new ClassAd (the caller owns it)
// that is similar to the original ClassAd, except that if it refers
// to attributes that are not in the current classad and they are not
// scoped, then they are renamed "target.attribute"
classad::ClassAd *AddExplicitTargets( classad::ClassAd *ad );

// recursive helper method
classad::ExprTree *AddExplicitTargets( classad::ExprTree *,
									   std::set< std::string,
									   classad::CaseIgnLTStr > &);

#endif
