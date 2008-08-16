/** 
 * @file head.cpp
 * @brief Head class implementation
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

// implementation of a class that models the motion of a human head.
// The physics aren't quite right, but it will suffice for right now.

#include "llviewerprecompiledheaders.h"

#include "head.h"

#ifndef PI
  #define PI ((F32) 3.14159265358979323846264338327)
#endif

// The friction force of the neck is modeled as:
//   friction = -E*phi_dot - F*phi_dot  (for positive phi_dot)
#define E1 0.75
#define F1 0.0

// TODO -- use the other PI defined in the math libs somewhere.

// --------------------------------------------------
// Misc. function delclarations...
// --------------------------------------------------
F32 phi_spring(F32 phi, F32 theta);
F32 theta_spring(F32 theta, F32 phi);


// --------------------------------------------------
// Class Head member functions...
// --------------------------------------------------

Head::Head() {
  mass = 1.0f;
  radius = 1.0f;
  inertia = 2.0f * mass * radius * radius / 5.0f;
  phi = 0.0f;
  theta = 0.0f;
  //gettimeofday(&t, &tz);
}

Head::Head(F32 m, F32 r) {
  mass = m;
  radius = r;
  inertia = 2.0f * mass * radius * radius / 5.0f;
  phi = 0.0f;
  theta = 0.0f;
  //gettimeofday(&t, &tz);
}

F32 Head::setMass(F32 m) { 
  mass = m; 
  inertia = 2.0f * mass * radius * radius / 5.0f;
  return mass;
}

F32 Head::setRadius(F32 r) { 
  radius = r; 
  inertia = 2.0f * mass * radius * radius / 5.0f;
  return radius;
}

F32 Head::getMass()    { return mass;    }
F32 Head::getRadius()  { return radius;  }
F32 Head::getInertia() { return inertia; }

void Head::propagate(F32 horizontal_force, F32 vertical_force, F32 dt) {
  //struct timeval t1;
  //F32 dt, temp;
  F32 phi_torque, theta_torque;
  F32 phi_dot, theta_dot;

  //gettimeofday(&t1, &tz);
  //dt = ((t1.tv_sec - t.tv_sec) * 1000000.0  + (t1.tv_usec - t.tv_usec)) / 1000000.0;
  //dt = 0.033;  // This is a kluge to prevent instabilities when using a stepper

  // Calculate the return forces...
  // Note: horizontal_force is positive toward right, so it's resultant torque
  // is negative, since the phi spin angle is out the top of the head (using 
  // right-hand rule).
  phi_torque   = radius * (phi_spring(phi, theta)   - horizontal_force); 
  theta_torque = radius * (theta_spring(phi, theta) + vertical_force);

  // Propagate phi...
  phi_dot = dt * phi_torque / inertia;
 
  phi += dt * phi_dot;
  if (phi >= PI/2.0f) {
    phi = PI/2.0f;
  }
  else if (phi <= -PI/2.0f) {
    phi = -PI/2.0f;
  }

  // Propagate theta...
  theta_dot = dt * theta_torque / inertia;
  theta += dt * theta_dot;
  if (theta >= PI/2.0f) {
    theta = PI/2.0f;
  }
  else if (theta <= -PI/2.0f) {
    theta = -PI/2.0f;
  }

  //t = t1;
  return;
}


F32 phi_spring(F32 phi, F32 theta) {
// The srping force is linear with -phi and the magnitude of theta.
  F32 phi_force;
  phi_force = - 30 * phi;
  phi_force = phi_force * (1.0f + (F32)fabs(theta) / PI);
  return phi_force;
}


F32 theta_spring(F32 phi, F32 theta) {
  return phi_spring(theta, phi);
}




