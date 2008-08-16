/** 
 * @file head.h
 * @brief Head class definition
 *
 * Copyright (c) 2000-2007, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */

// description of a class that models the motion of a human head.
// The physics isn't quite right, but it will suffice for right now.

#ifndef LL_HEAD_H
#define LL_HEAD_H

//#include <sys/time.h>

class Head {
  public:

  Head();
  Head(F32 m, F32 r);

  F32 setMass(F32 m);
  F32 setRadius(F32 r);

  F32 getMass();
  F32 getRadius();
  F32 getInertia();

  void propagate(F32 horizontal_force, F32 vertical_force, F32 dt);

  F32 phi, theta;                    // azimuth and elevation
  //struct timeval t;
  //struct timezone tz;

  private:
  F32 mass, radius, inertia;
};

#endif


