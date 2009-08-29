/** 
 * @file mass_properties.cpp
 * @author andrew@lindenlab.com
 * @date 2007-12-20
 * @brief Tests for the LLPrimMassProperties and LLObjectMassProperties classes
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "linden_common.h"
#include "lltut.h"
#include <llmath/v3math.h>
#include <llmath/llquaternion.h>
#include <llphysics/abstract/utils/llinertiatensorutils.h>
#include <llphysics/abstract/utils/llobjectmassproperties.h>
#include <llphysics/abstract/utils/llprimmassproperties.h>
#include <llphysics/abstract/utils/llphysicsvolumemanager.h>
#include <llprimitive/llmaterialtable.h>
#include <llprimitive/llprimitive.h>


const F32 SMALL_RELATIVE_ERROR = 0.001f;	// 0.1%
const F32 SQRT_THREE = 1.732050808f;
const F32 SQRT_SIX = 2.449489743f;

namespace tut
{
	struct mass_properties_data
	{
		LLPrimMassProperties prim_properties;
		LLObjectMassProperties object_properties;
	};

	typedef test_group<mass_properties_data> mass_properties_group;
	typedef mass_properties_group::object mass_properties;
	tut::mass_properties_group mp_test_group("mass properties");

	template<> template<>
	void mass_properties::test<1>()
	{
		// test SPHERE mass properties
		LLPrimMassProperties prim_sphere;
		prim_sphere.setUnitSphere();

		F32 density = 1000.f;
		F32 radius = 5.f;
		F32 diameter = 2.f * radius;
		LLVector3 scale(diameter, diameter, diameter);
		LLObjectMassProperties obj_sphere(prim_sphere, scale, density);
		
		F32 computed_mass = obj_sphere.getMass();
		//LLVector3 center_of_mass
		//obj_sphere.getCenterOfMass(center_of_mass);
		LLMatrix3 inertia;
		obj_sphere.getInertiaLocal(inertia);
		F32 computed_inertia_eigenvalue = inertia.mMatrix[0][0];


		// volume is normalized for scale = <1,1,1>
		// V = 4/3 * PI * r^3
		// inertia_eigenvalue = (2/5) * M * r^2
		F32 volume = ( 4.f / 3.f ) * radius * radius * radius * F_PI;
		F32 expected_mass = density * volume;
		F32 expected_inertia_eigenvalue = ( 2.f / 5.f ) * expected_mass * radius * radius;
	
		F32 error = fabs(computed_mass - expected_mass) / expected_mass;
		ensure("expected sphere mass should match computed", error < SMALL_RELATIVE_ERROR);

		error = fabs(computed_inertia_eigenvalue - expected_inertia_eigenvalue) / expected_inertia_eigenvalue;
		ensure("expected sphere inertia should match computed", error < SMALL_RELATIVE_ERROR);
	}

	template<> template<>
	void mass_properties::test<2>()
	{
		// test LLInertiaTensorUtils

		// define a known inertia tensor in the center of mass frame
		// from the numerical example in this paper:
		// http://www.journaldatabase.org/articles/87064/Explicit_Exact_Formulas_f.html
		F32 known_mass = 1873.23f;
		LLVector3 known_center( 0.f, 0.f, 0.f );
		LLMatrix3 known_inertia;
		known_inertia.mMatrix[0][0] = 43520.33257f;
		known_inertia.mMatrix[1][1] = 194711.28938f;
		known_inertia.mMatrix[2][2] = 191168.76173f;

		known_inertia.mMatrix[0][1] = -11996.20119f;
		known_inertia.mMatrix[1][0] = -11996.20119f;

		known_inertia.mMatrix[0][2] =  46343.16662f;
		known_inertia.mMatrix[2][0] =  46343.16662f;

		known_inertia.mMatrix[2][1] = -4417.66150f;
		known_inertia.mMatrix[1][2] = -4417.66150f;

		// the following two shifts should have null effect
		{
			LLVector3 first_shift(2.f, 3.f, 4.f);
			LLVector3 second_shift = - first_shift;
			LLMatrix3 inertia = known_inertia;
			
			LLInertiaTensorUtils::shiftCenteredInertiaTensor(inertia, first_shift, known_mass);
			LLInertiaTensorUtils::centerInertiaTensor(inertia, second_shift, known_mass);

			// we should now have the same inertia with which we started
			for (S32 i=0; i<3; ++i)
			{
				for (S32 j=0; j<3; ++j)
				{
					F32 error = fabs(1.f - inertia.mMatrix[i][j] / known_inertia.mMatrix[i][j]);
					ensure("LLInertiaTensorUtils shift+sclae-shift-scale should be no-op", error < SMALL_RELATIVE_ERROR);
				}
			}
		}

		// the following series operations should have null effect
		{
			LLVector3 first_shift(1.f, 5.f, 10.f);
			LLVector3 second_scale(2.f, 3.f, 4.f);
			LLVector3 third_shift;
			LLVector3 fourth_scale;
			for (S32 i = 0; i < 3; ++i)
			{
				third_shift.mV[i] = -first_shift.mV[i] * second_scale.mV[i];
				fourth_scale.mV[i] = 1.f / second_scale.mV[i];
			}
	
			F32 mass = known_mass;
			LLVector3 center = known_center;
			LLMatrix3 inertia = known_inertia;
	
			// first
			LLInertiaTensorUtils::shiftCenteredInertiaTensor(inertia, first_shift, mass);
			center += first_shift;
	
			// second
			LLInertiaTensorUtils::scaleInertiaTensor(inertia, second_scale);
			mass *= second_scale.mV[VX] * second_scale.mV[VY] * second_scale.mV[VZ];
			for (S32 i = 0; i < 3; ++i)
			{
				center.mV[i] *= second_scale.mV[i];
			}
	
			// third
			LLInertiaTensorUtils::centerInertiaTensor(inertia, third_shift, mass);
			center -= third_shift;
	
			// foruth
			LLInertiaTensorUtils::scaleInertiaTensor(inertia, fourth_scale);
	
			// we should now have the same inertia with which we started
			for (S32 i=0; i<3; ++i)
			{
				for (S32 j=0; j<3; ++j)
				{
					F32 error = fabs(1.f - inertia.mMatrix[i][j] / known_inertia.mMatrix[i][j]);
					ensure("LLInertiaTensorUtils shift+sclae-shift-scale should be no-op", error < SMALL_RELATIVE_ERROR);
				}
			}
		}
	}

	template<> template<>
	void mass_properties::test<3>()
	{
		// test tetrahedral decomposition of unit tetrahedron centered on origin
		std::vector< LLVector3 > points;
		points.push_back( LLVector3(  0.f, 0.f, 0.f ) );
		points.push_back( LLVector3(  1.f, 0.f, 0.f ) );
		points.push_back( LLVector3( 0.5f, 0.5f * SQRT_THREE, 0.f) );
		points.push_back( LLVector3( 0.5f, SQRT_THREE / 6.f, SQRT_SIX / 3.f) );

		// compute the center
		LLVector3 center;
		for (S32 i = 0; i < (S32)points.size(); ++i)
		{
			center += points[i];
		}
		center *= ( 1.f / F32(points.size()) );

		// shift all points to center of mass frame
		for (S32 i = 0; i < (S32)points.size(); ++i)
		{
			points[i] -= center;
		}

		LLPrimMassProperties tetrahedron;
		tetrahedron.addSignedTetrahedron(1.0, points[0], points[1], points[2], points[3]);
		// we must manually center the inertia tensor here 
		// since addSignedTetrahedron() does not do it automatically
		tetrahedron.centerInertiaTensor();

		F32 density = 1.0f;
		LLVector3 scale(1.f, 1.f, 1.f);
		LLMatrix3 analytic_inertia;
		tetrahedron.getScaledInertiaTensor(analytic_inertia, scale, density);

		// compute the mesh
		std::vector< S32 > triangle_indices;
		triangle_indices.push_back(0);
		triangle_indices.push_back(2);
		triangle_indices.push_back(1);

		triangle_indices.push_back(0);
		triangle_indices.push_back(1);
		triangle_indices.push_back(3);

		triangle_indices.push_back(0);
		triangle_indices.push_back(3);
		triangle_indices.push_back(2);

		triangle_indices.push_back(1);
		triangle_indices.push_back(2);
		triangle_indices.push_back(3);

		// compute the same inertia using a mesh
		{
			LLPrimMassProperties mesh;
			mesh.setUnitMesh(points, triangle_indices);

			// the two properties should agree
			F32 error = ( tetrahedron.getVolume() - mesh.getVolume() ) / tetrahedron.getVolume();
			ensure("tetrahedron and mesh volume should match", error < SMALL_RELATIVE_ERROR);
	
			error = ( tetrahedron.getCenterOfMass() - mesh.getCenterOfMass() ).length();
			ensure("tetrahedron and mesh centers should match", error < SMALL_RELATIVE_ERROR);
	
			LLMatrix3 mesh_inertia;
			mesh.getScaledInertiaTensor(mesh_inertia, scale, density);

			for (S32 i=0; i<3; ++i)
			{
				for (S32 j=0; j<3; ++j)
				{
					// only verify the non-small elements
					if (analytic_inertia.mMatrix[i][j] > SMALL_RELATIVE_ERROR)
					{
						error = fabs(1.f - mesh_inertia.mMatrix[i][j] / analytic_inertia.mMatrix[i][j]);
						ensure("LLPrimMassProperties::setUnitMesh() inertia ", error < SMALL_RELATIVE_ERROR);
					}
				}
			}
		}

		// shift the whole tetrahedron away from the center of mass and recompute the mesh
		{
			LLVector3 shift(11.f, 7.f, 3.f);
			for (S32 i = 0; i < (S32)points.size(); ++i)
			{
				points[i] += shift;
			}
			LLPrimMassProperties mesh;
			mesh.setUnitMesh(points, triangle_indices);
	
			// the two properties should agree
			F32 error = ( tetrahedron.getVolume() - mesh.getVolume() ) / tetrahedron.getVolume();
			ensure("tetrahedron and mesh volume should match", error < SMALL_RELATIVE_ERROR);
	
			LLMatrix3 mesh_inertia;
			mesh.getScaledInertiaTensor(mesh_inertia, scale, density);

			for (S32 i=0; i<3; ++i)
			{
				for (S32 j=0; j<3; ++j)
				{
					// only verify the non-small elements
					if (analytic_inertia.mMatrix[i][j] > SMALL_RELATIVE_ERROR)
					{
						error = fabs(1.f - mesh_inertia.mMatrix[i][j] / analytic_inertia.mMatrix[i][j]);
						ensure("LLPrimMassProperties::setUnitMesh() inertia ", error < SMALL_RELATIVE_ERROR);
					}
				}
			}
		}
	}

	template<> template<>
	void mass_properties::test<4>()
	{
		// test tetrahedron utilities

		// from the paper described here:
		// from the numerical example in this paper:
		// http://www.journaldatabase.org/articles/87064/Explicit_Exact_Formulas_f.html

		// initialize info about the tetrahedron
		std::vector< LLVector3 > points;
		points.push_back( LLVector3(  8.33220f, -11.86875f,   0.93355f) );
		points.push_back( LLVector3(  0.75523f,   5.00000f,  16.37072f) );
		points.push_back( LLVector3( 52.61236f,   5.00000f, - 5.38580f) );
		points.push_back( LLVector3(  2.00000f,   5.00000f,   3.00000f) );

		LLVector3 expected_center( 15.92492f, 0.78281f, 3.732962f);

		LLMatrix3 expected_inertia;
		expected_inertia.mMatrix[0][0] = 43520.33257f;
		expected_inertia.mMatrix[1][1] = 194711.28938f;
		expected_inertia.mMatrix[2][2] = 191168.76173f;

		expected_inertia.mMatrix[0][1] = -11996.20119f;
		expected_inertia.mMatrix[1][0] = -11996.20119f;

		expected_inertia.mMatrix[0][2] =  46343.16662f;
		expected_inertia.mMatrix[2][0] =  46343.16662f;

		expected_inertia.mMatrix[2][1] = -4417.66150f;
		expected_inertia.mMatrix[1][2] = -4417.66150f;

		// measure tetrahedron bounding box max dimension 
		// for relative error estimates
		LLVector3 box_min(FLT_MAX, FLT_MAX, FLT_MAX);
		LLVector3 box_max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		for (S32 point_index = 0; point_index < (S32)points.size(); ++point_index)
		{
			for (S32 i = 0; i < 3; ++i)
			{
				if (points[point_index].mV[i] < box_min.mV[i])
				{
					box_min.mV[i] = points[point_index].mV[i];
				}
				if (points[point_index].mV[i] > box_max.mV[i])
				{
					box_max.mV[i] = points[point_index].mV[i];
				}
			}
		}
		F32 tetrahedron_max_dimension = (box_max - box_min).length();


		// test LLPrimMassProperties::addSignedTetrahedron()
		{
			LLPrimMassProperties tetrahedron;
			tetrahedron.addSignedTetrahedron(1.f, points[0], points[1], points[2], points[3]);
			// we must manually center the inertia tensor here 
			// since addSignedTetrahedron() does not do it automatically
			tetrahedron.centerInertiaTensor();
	
			// check the center of mass
			LLVector3 center = tetrahedron.getCenterOfMass();
			F32 error = (center - expected_center).length() / tetrahedron_max_dimension;
	
			ensure("LLPrimMassProperties::addSignedTetrahedron() center of mass ", error < SMALL_RELATIVE_ERROR);
	
			// check the inertia tensor
			LLMatrix3 computed_inertia;
			LLVector3 scale(1.f, 1.f, 1.f);
			F32 density = 1.f;
			tetrahedron.getScaledInertiaTensor(computed_inertia, scale, density);

			for (S32 i=0; i<3; ++i)
			{
				for (S32 j=0; j<3; ++j)
				{
					error = fabs(1.f - computed_inertia.mMatrix[i][j] / expected_inertia.mMatrix[i][j]);
					ensure("LLPrimMassProperties::addSignedTetrahedron inertia ", error < SMALL_RELATIVE_ERROR);
				}
			}
		}

		// test LLPrimMassProperties::addUnitMesh()
		{
			std::vector< S32 > triangle_indices;
			triangle_indices.push_back(0);
			triangle_indices.push_back(2);
			triangle_indices.push_back(1);
	
			triangle_indices.push_back(1);
			triangle_indices.push_back(3);
			triangle_indices.push_back(0);
	
			triangle_indices.push_back(2);
			triangle_indices.push_back(0);
			triangle_indices.push_back(3);
	
			triangle_indices.push_back(3);
			triangle_indices.push_back(1);
			triangle_indices.push_back(2);
	
			LLPrimMassProperties mesh;
			mesh.setUnitMesh(points, triangle_indices);

			// check the center of mass
			LLVector3 center = mesh.getCenterOfMass();
			F32 error = (center - expected_center).length() / tetrahedron_max_dimension;
	
			ensure("LLPrimMassProperties::setUnitMesh() center of mass ", error < SMALL_RELATIVE_ERROR);
	
			// check the inertia tensor
			LLMatrix3 computed_inertia;
			LLVector3 scale(1.f, 1.f, 1.f);
			F32 density = 1.f;
			mesh.getScaledInertiaTensor(computed_inertia, scale, density);

			for (S32 i=0; i<3; ++i)
			{
				for (S32 j=0; j<3; ++j)
				{
					error = fabs(1.f - computed_inertia.mMatrix[i][j] / expected_inertia.mMatrix[i][j]);
					ensure("LLPrimMassProperties::setUnitMesh() inertia diagonal elements mismatch", error < SMALL_RELATIVE_ERROR);
				}
			}
		}
	}

	template<> template<>
	void mass_properties::test<5>()
	{
		// test LLPrimMassProperties

		// unit shape box
		LLPrimMassProperties box;
		box.setUnitBox();

		// unit shape mesh -- box

		//      
		//          4-----------0
		//  z      /|          /|
		//  |     / |         / |
		//  |    /  |        /  |
		//  |   6-----------2   |
		//  |   |   |       |   |
		//  |   |   5-------|---1
		//  |   |  /        |  /
		//  |   | /         | /
		//  | y |/          |/
		//  |/  7-----------3
		//  +------------------------ x

		std::vector< LLVector3 > points;
		points.push_back( LLVector3( 0.5f,  0.5f,  0.5f) );
		points.push_back( LLVector3( 0.5f,  0.5f, -0.5f) );
		points.push_back( LLVector3( 0.5f, -0.5f,  0.5f) );
		points.push_back( LLVector3( 0.5f, -0.5f, -0.5f) );
		points.push_back( LLVector3(-0.5f,  0.5f,  0.5f) );
		points.push_back( LLVector3(-0.5f,  0.5f, -0.5f) );
		points.push_back( LLVector3(-0.5f, -0.5f,  0.5f) );
		points.push_back( LLVector3(-0.5f, -0.5f, -0.5f) );

		std::vector< S32 > triangle_indices;
		// +x
		triangle_indices.push_back(1);
		triangle_indices.push_back(0);
		triangle_indices.push_back(2);

		triangle_indices.push_back(1);
		triangle_indices.push_back(2);
		triangle_indices.push_back(3);

		// -y
		triangle_indices.push_back(3);
		triangle_indices.push_back(2);
		triangle_indices.push_back(7);

		triangle_indices.push_back(7);
		triangle_indices.push_back(2);
		triangle_indices.push_back(6);

		// -x
		triangle_indices.push_back(7);
		triangle_indices.push_back(6);
		triangle_indices.push_back(4);

		triangle_indices.push_back(7);
		triangle_indices.push_back(4);
		triangle_indices.push_back(5);

		// +y
		triangle_indices.push_back(5);
		triangle_indices.push_back(4);
		triangle_indices.push_back(1);

		triangle_indices.push_back(1);
		triangle_indices.push_back(4);
		triangle_indices.push_back(0);

		// +z
		triangle_indices.push_back(0);
		triangle_indices.push_back(4);
		triangle_indices.push_back(6);

		triangle_indices.push_back(0);
		triangle_indices.push_back(6);
		triangle_indices.push_back(2);

		// -z
		triangle_indices.push_back(7);
		triangle_indices.push_back(5);
		triangle_indices.push_back(3);

		triangle_indices.push_back(3);
		triangle_indices.push_back(5);
		triangle_indices.push_back(1);
				
		LLPrimMassProperties mesh;
		mesh.setUnitMesh(points, triangle_indices);

		// the unit box and unit mesh mass properties should be nearly the same

		// volume should agree
		F32 error = fabs(box.getVolume() - mesh.getVolume()) / box.getVolume();
		ensure("UnitBox and UnitMesh(box) should have same volume", error < SMALL_RELATIVE_ERROR);

		// center of mass should agree
		LLVector3 box_center = box.getCenterOfMass();
		LLVector3 mesh_center = mesh.getCenterOfMass();
		error = fabs( (box_center - mesh_center).length() );
		ensure("UnitBox and UnitMesh(box) centers of mass should agree", error < SMALL_RELATIVE_ERROR );

		LLVector3 scale(1.f, 1.f, 1.f);
		F32 density = 1.f;
		LLMatrix3 box_inertia, mesh_inertia;
		box.getScaledInertiaTensor(box_inertia, scale, density);
		mesh.getScaledInertiaTensor(mesh_inertia, scale, density);

		// mesh eigenvalues should be uniform
		for (S32 i = 0; i < 2; ++i)
		{
			error =  fabs(mesh_inertia.mMatrix[i][i] - mesh_inertia.mMatrix[i+1][i+1]) / mesh_inertia.mMatrix[i][i];
			ensure("UnitMesh(box) should have uniform eigenvalues", error < SMALL_RELATIVE_ERROR);
		}
		// inertias should agree
		for (S32 i = 0; i < 3; ++i)
		{
			for (S32 j = 0; j < 3; ++j)
			{
				error = fabs(box_inertia.mMatrix[i][j] - mesh_inertia.mMatrix[i][j]);
				if (error > 0.f
					&& box_inertia.mMatrix[i][j] != 0.f)
				{
					error /= box_inertia.mMatrix[i][j];
				}
				ensure("UnitBox and UnitMesh(box) should have same inertia", error < SMALL_RELATIVE_ERROR);
			}
		}

		// Here we test the boundary of the LLPrimLinkInfo::canLink() method 
		// between semi-random middle-sized objects.
	}

	template<> template<>
	void mass_properties::test<6>()
	{
		// test LLObjectMassProperties

		// we make a large single-prim box, then a similarly shaped object
		// that is multiple prims, and compare their mass properties

		LLPrimMassProperties box;
		box.setUnitBox();

		F32 density = 3.7f;
		LLVector3 big_scale(1.f, 2.f, 3.f);
		LLObjectMassProperties big_box(box, big_scale, density);

		LLObjectMassProperties multiple_box;
		LLVector3 position;
		LLQuaternion rotation;
		rotation.loadIdentity();

		F32 small_box_size = 0.5f;
		LLVector3 small_scale( small_box_size, small_box_size, small_box_size);
		S32 num_boxes_x = S32(big_scale.mV[VX] / small_box_size);
		S32 num_boxes_y = S32(big_scale.mV[VY] / small_box_size);
		S32 num_boxes_z = S32(big_scale.mV[VZ] / small_box_size);
		LLVector3 start_pos = 0.5f * (small_scale - big_scale);
		for (S32 x = 0; x < num_boxes_x; ++x)
		{
			for (S32 y = 0; y < num_boxes_y; ++y)
			{
				for (S32 z = 0; z < num_boxes_z; ++z)
				{
					position.set( F32(x) * small_box_size, F32(y) * small_box_size, F32(z) * small_box_size );
					position += start_pos;

					multiple_box.add(box, small_scale, density, position, rotation);
				}
			}
		}

		// the mass properties of the two boxes should match

		// mass
		F32 big_mass = big_box.getMass();
		F32 multiple_mass = multiple_box.getMass();
		F32 error = (big_mass - multiple_mass) / big_mass;
		ensure("Big box and equivalent multi-prim box should have same mass", error < SMALL_RELATIVE_ERROR);

		// center of mass
		LLVector3 big_center, multiple_center;
		big_box.getCenterOfMass(big_center);
		multiple_box.getCenterOfMass(multiple_center);
		error = (big_center - multiple_center).length();
		ensure("Big box and equivalent multi-prim box should have same center", error < SMALL_RELATIVE_ERROR);

		// inertia
		LLMatrix3 big_inertia, multiple_inertia;
		big_box.getInertiaLocal(big_inertia);
		multiple_box.getInertiaLocal(multiple_inertia);
		for (S32 i = 0; i < 3; ++i)
		{
			for (S32 j = 0; j < 3; ++j)
			{
				error = fabs(big_inertia.mMatrix[i][j] - multiple_inertia.mMatrix[i][j]);
				if (error > 0.f
					&& big_inertia.mMatrix[i][j] != 0.f)
				{
					error /= big_inertia.mMatrix[i][j];
				}
				ensure("UnitBox and UnitMesh(box) should have same inertia", error < SMALL_RELATIVE_ERROR);
			}
		}
	}

	template<> template<>
	void mass_properties::test<7>()
	{
		// test LLObjectMassProperties with rotations

		// we make a large single-prim box via mesh, then a similarly shaped 
		// object that is multiple prims (implicit boxes), and compare their 
		// mass properties

		//      
		//          4-----------0
		//  z      /|          /|
		//  |     / |         / |
		//  |    /  |        /  |
		//  |   6-----------2   |
		//  |   |   |       |   |
		//  |   |   5-------|---1
		//  |   |  /        |  /
		//  |   | /         | /
		//  | y |/          |/
		//  |/  7-----------3
		//  +------------------------ x

		std::vector< LLVector3 > points;
		points.push_back( LLVector3( 0.5f,  0.5f,  0.5f) );
		points.push_back( LLVector3( 0.5f,  0.5f, -0.5f) );
		points.push_back( LLVector3( 0.5f, -0.5f,  0.5f) );
		points.push_back( LLVector3( 0.5f, -0.5f, -0.5f) );
		points.push_back( LLVector3(-0.5f,  0.5f,  0.5f) );
		points.push_back( LLVector3(-0.5f,  0.5f, -0.5f) );
		points.push_back( LLVector3(-0.5f, -0.5f,  0.5f) );
		points.push_back( LLVector3(-0.5f, -0.5f, -0.5f) );

		std::vector< S32 > triangle_indices;
		// +x
		triangle_indices.push_back(1);
		triangle_indices.push_back(0);
		triangle_indices.push_back(2);

		triangle_indices.push_back(1);
		triangle_indices.push_back(2);
		triangle_indices.push_back(3);

		// -y
		triangle_indices.push_back(3);
		triangle_indices.push_back(2);
		triangle_indices.push_back(7);

		triangle_indices.push_back(7);
		triangle_indices.push_back(2);
		triangle_indices.push_back(6);

		// -x
		triangle_indices.push_back(7);
		triangle_indices.push_back(6);
		triangle_indices.push_back(4);

		triangle_indices.push_back(7);
		triangle_indices.push_back(4);
		triangle_indices.push_back(5);

		// +y
		triangle_indices.push_back(5);
		triangle_indices.push_back(4);
		triangle_indices.push_back(1);

		triangle_indices.push_back(1);
		triangle_indices.push_back(4);
		triangle_indices.push_back(0);

		// +z
		triangle_indices.push_back(0);
		triangle_indices.push_back(4);
		triangle_indices.push_back(6);

		triangle_indices.push_back(0);
		triangle_indices.push_back(6);
		triangle_indices.push_back(2);

		// -z
		triangle_indices.push_back(7);
		triangle_indices.push_back(5);
		triangle_indices.push_back(3);

		triangle_indices.push_back(3);
		triangle_indices.push_back(5);
		triangle_indices.push_back(1);

		F32 angle_step = F_PI / (2.f * 3.f);
		for (F32 angle = 0.f; angle < 0.51f * F_PI; angle += angle_step)
		{
			// scale and rotate mesh points
			LLVector3 axis(0.f, 0.f, angle);
			LLQuaternion mesh_rotation(angle, axis);
			LLVector3 big_scale(3.f, 5.f, 7.f);
			std::vector< LLVector3 > new_points;
			for (S32 p = 0; p < (S32)points.size(); ++p)
			{
				LLVector3 new_point = points[p];
				for (S32 i = 0; i < 3; ++i)
				{
					new_point.mV[i] *= big_scale.mV[i];
				}
				new_points.push_back( new_point * mesh_rotation );
			}
				
			// build the big mesh box
			LLPrimMassProperties mesh_box;
			mesh_box.setUnitMesh(new_points, triangle_indices);
	
			F32 density = 3.7f;
			LLVector3 unit_scale(1.f, 1.f, 1.f);
			LLObjectMassProperties big_box(mesh_box, unit_scale, density);
	
			// build the multiple_box
			LLPrimMassProperties box;
			box.setUnitBox();
	
			LLObjectMassProperties multiple_box;
			LLVector3 position;
	
			F32 small_box_size = 0.5f;
			LLVector3 small_scale( small_box_size, small_box_size, small_box_size);
			S32 num_boxes_x = S32(big_scale.mV[VX] / small_box_size);
			S32 num_boxes_y = S32(big_scale.mV[VY] / small_box_size);
			S32 num_boxes_z = S32(big_scale.mV[VZ] / small_box_size);
			LLVector3 start_pos = (0.5f * (small_scale - big_scale)) * mesh_rotation;
			for (S32 x = 0; x < num_boxes_x; ++x)
			{
				for (S32 y = 0; y < num_boxes_y; ++y)
				{
					for (S32 z = 0; z < num_boxes_z; ++z)
					{
						position.set( F32(x) * small_box_size, F32(y) * small_box_size, F32(z) * small_box_size );
						position *= mesh_rotation;
						position += start_pos;
						multiple_box.add(box, small_scale, density, position, mesh_rotation);
					}
				}
			}

			// the mass properties of the two boxes should match

			// mass
			F32 big_mass = big_box.getMass();
			F32 multiple_mass = multiple_box.getMass();
			F32 error = (big_mass - multiple_mass) / big_mass;
			ensure("Big box and equivalent multi-prim box should have same mass", error < SMALL_RELATIVE_ERROR);

			// center of mass
			LLVector3 big_center, multiple_center;
			big_box.getCenterOfMass(big_center);
			multiple_box.getCenterOfMass(multiple_center);
			error = (big_center - multiple_center).length();
			ensure("Big box and equivalent multi-prim box should have same center", error < SMALL_RELATIVE_ERROR);

			LLMatrix3 big_inertia, multiple_inertia;
			big_box.getInertiaLocal(big_inertia);
			multiple_box.getInertiaLocal(multiple_inertia);

			for (S32 i = 0; i < 3; ++i)
			{
				for (S32 j = 0; j < 3; ++j)
				{
					error = fabs(big_inertia.mMatrix[i][j] - multiple_inertia.mMatrix[i][j]);
					if (error > 0.f
						&& big_inertia.mMatrix[i][j] > SMALL_RELATIVE_ERROR)
					{
						error /= big_inertia.mMatrix[i][j];
					}
					ensure("UnitBox and UnitMesh(box) should have same inertia", error < SMALL_RELATIVE_ERROR);
				}
			}
		}
	}

	template<> template<>
	void mass_properties::test<8>()
	{
		// test LLPhysicsVolumeManager

		// we make a large single-prim box, then a similarly shaped object
		// that is multiple prims, and compare their mass properties

		// first we make the single-prim giant
		//      
		//          4-----------0
		//  z      /|          /|
		//  |     / |         / |
		//  |    /  |        /  |
		//  |   6-----------2   |
		//  |   |   |       |   |
		//  |   |   5-------|---1
		//  |   |  /        |  /
		//  |   | /         | /
		//  | y |/          |/
		//  |/  7-----------3
		//  +------------------------ x

		std::vector< LLVector3 > points;
		points.push_back( LLVector3( 0.5f,  0.5f,  0.5f) );
		points.push_back( LLVector3( 0.5f,  0.5f, -0.5f) );
		points.push_back( LLVector3( 0.5f, -0.5f,  0.5f) );
		points.push_back( LLVector3( 0.5f, -0.5f, -0.5f) );
		points.push_back( LLVector3(-0.5f,  0.5f,  0.5f) );
		points.push_back( LLVector3(-0.5f,  0.5f, -0.5f) );
		points.push_back( LLVector3(-0.5f, -0.5f,  0.5f) );
		points.push_back( LLVector3(-0.5f, -0.5f, -0.5f) );

		std::vector< S32 > triangle_indices;
		// +x
		triangle_indices.push_back(1);
		triangle_indices.push_back(0);
		triangle_indices.push_back(2);

		triangle_indices.push_back(1);
		triangle_indices.push_back(2);
		triangle_indices.push_back(3);

		// -y
		triangle_indices.push_back(3);
		triangle_indices.push_back(2);
		triangle_indices.push_back(7);

		triangle_indices.push_back(7);
		triangle_indices.push_back(2);
		triangle_indices.push_back(6);

		// -x
		triangle_indices.push_back(7);
		triangle_indices.push_back(6);
		triangle_indices.push_back(4);

		triangle_indices.push_back(7);
		triangle_indices.push_back(4);
		triangle_indices.push_back(5);

		// +y
		triangle_indices.push_back(5);
		triangle_indices.push_back(4);
		triangle_indices.push_back(1);

		triangle_indices.push_back(1);
		triangle_indices.push_back(4);
		triangle_indices.push_back(0);

		// +z
		triangle_indices.push_back(0);
		triangle_indices.push_back(4);
		triangle_indices.push_back(6);

		triangle_indices.push_back(0);
		triangle_indices.push_back(6);
		triangle_indices.push_back(2);

		// -z
		triangle_indices.push_back(7);
		triangle_indices.push_back(5);
		triangle_indices.push_back(3);

		triangle_indices.push_back(3);
		triangle_indices.push_back(5);
		triangle_indices.push_back(1);

		// scale the mesh points
		LLVector3 big_scale(1.f, 2.f, 3.f);
		std::vector< LLVector3 > new_points;
		for (S32 p = 0; p < (S32)points.size(); ++p)
		{
			LLVector3 new_point = points[p];
			for (S32 i = 0; i < 3; ++i)
			{
				new_point.mV[i] *= big_scale.mV[i];
			}
			new_points.push_back( new_point );
		}

		// build the big mesh box (primitive)
		LLPrimMassProperties mesh_box;
		mesh_box.setUnitMesh(new_points, triangle_indices);

		F32 density = DEFAULT_OBJECT_DENSITY;
		LLVector3 unit_scale(1.f, 1.f, 1.f);
		LLObjectMassProperties big_box(mesh_box, unit_scale, density);
		
		// build the multi-prim box (object)
		S32 TEST_VOLUME_DETAIL = 1;

		LLVolumeParams volume_params;
		volume_params.setCube();
		
		LLObjectMassProperties multiple_box;
		F32 small_box_size = 0.5f;
		LLVector3 small_scale( small_box_size, small_box_size, small_box_size);
		{
			// hijack the volume manager used by LLPrimitive
			LLPhysicsVolumeManager* volume_manager = new LLPhysicsVolumeManager();
			//volume_manager->setThreadSafe(false);
			LLPrimitive::setVolumeManager(volume_manager);

			std::vector< const LLPrimitive* > prim_list;
	
			F32 angle = 0.f;
			LLVector3 axis(0.f, 0.f, angle);
			LLVector3 position;
			LLQuaternion rotation(angle, axis);
			S32 num_boxes_x = S32(big_scale.mV[VX] / small_box_size);
			S32 num_boxes_y = S32(big_scale.mV[VY] / small_box_size);
			S32 num_boxes_z = S32(big_scale.mV[VZ] / small_box_size);
	
			for (S32 x = 0; x < num_boxes_x; ++x)
			{
				for (S32 y = 0; y < num_boxes_y; ++y)
				{
					for (S32 z = 0; z < num_boxes_z; ++z)
					{
						LLPrimitive* primp = new LLPrimitive();
						primp->setVolume( volume_params, TEST_VOLUME_DETAIL);
	
						position.set( F32(x) * small_box_size, F32(y) * small_box_size, F32(z) * small_box_size );
						position *= rotation;
	
						primp->setPosition(position);
						primp->setRotation(rotation);
						primp->setScale(small_scale);
	
						prim_list.push_back(primp);
					}
				}
			}
	
			volume_manager->getObjectMassProperties(multiple_box, prim_list);
	
			for (S32 i = 0; i < (S32)prim_list.size(); ++i)
			{
				delete prim_list[i];
				prim_list[i] = NULL;
			}
			LLPrimitive::cleanupVolumeManager();
		}

		// mass
		F32 big_mass = big_box.getMass();
		F32 multiple_mass = multiple_box.getMass();
		F32 error = (big_mass - multiple_mass) / big_mass;
		ensure("Big box and equivalent multi-prim box should have same mass", error < SMALL_RELATIVE_ERROR);

		// center of mass
		LLVector3 big_center, multiple_center;
		big_box.getCenterOfMass(big_center);
		multiple_box.getCenterOfMass(multiple_center);
		LLVector3 expected_shift = 0.5f * ( big_scale - small_scale );
		error = ( big_center - (multiple_center - expected_shift) ).length();
		ensure("Big box and equivalent multi-prim box should have same center", error < SMALL_RELATIVE_ERROR);

		// inertia
		LLMatrix3 big_inertia, multiple_inertia;
		big_box.getInertiaLocal(big_inertia);
		multiple_box.getInertiaLocal(multiple_inertia);

		for (S32 i = 0; i < 3; ++i)
		{
			for (S32 j = 0; j < 3; ++j)
			{
				error = fabs(big_inertia.mMatrix[i][j] - multiple_inertia.mMatrix[i][j]);
				if (error > 0.f
					&& big_inertia.mMatrix[i][j] > SMALL_RELATIVE_ERROR)
				{
					error /= big_inertia.mMatrix[i][j];
				}
				bool ok = error < SMALL_RELATIVE_ERROR
						  || (i != j 
							  && error < SMALL_RELATIVE_ERROR);
				ensure("UnitBox and UnitMesh(box) should have same inertia", ok );
			}
		}
	}
}

