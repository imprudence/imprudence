/** 
 * @file llnamevalue_tut.cpp
 * @author Adroit
 * @date 2007-02
 * @brief LLNameValue unit test
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2008, Linden Research, Inc.
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
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
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
#include "llnamevalue.h"
#include "llsdserialize.h"

namespace tut
{
	struct namevalue_test
	{
		namevalue_test() 
		{			
			mExpectedNameValueReference.string = NULL;
			mExpectedNameValueType = NVT_NULL;
			mCallbackCount = 0;
		}

		~namevalue_test() 
		{
			reset();
		};
		
		void reset() 
		{
			switch(mExpectedNameValueType)
			{
			case NVT_STRING: 
			case NVT_ASSET: 
				delete [] mExpectedNameValueReference.string;
				mExpectedNameValueReference.string = NULL;
				break;
			case NVT_F32:
				delete mExpectedNameValueReference.f32;
				mExpectedNameValueReference.f32 = NULL;
				break;
			case NVT_S32:
				delete mExpectedNameValueReference.s32;
				mExpectedNameValueReference.s32 = NULL;
				break;
			case NVT_U32:
				delete mExpectedNameValueReference.u32;
				mExpectedNameValueReference.u32 = NULL;
				break;
			case NVT_VEC3:
				delete mExpectedNameValueReference.vec3;
				mExpectedNameValueReference.vec3 = NULL;
				break;
			case NVT_U64:
				delete mExpectedNameValueReference.u64;
				mExpectedNameValueReference.u64 = NULL;
			default:
				break;
			}

			mExpectedNameValueType = NVT_NULL;
			mCallbackCount = 0;
		}

		void setExpectedResult(ENameValueType type, void* value)
		{
			reset();
			mExpectedNameValueType = type;
			switch(type)
			{
			case NVT_STRING:
			case NVT_ASSET: 
				mExpectedNameValueReference.string = new char[strlen((const char*) value)+1];
				strcpy(mExpectedNameValueReference.string, (const char*) value);
				break;
			case NVT_F32:
				mExpectedNameValueReference.f32 = new F32(*((F32*) value));
				break;
			case NVT_S32:
				mExpectedNameValueReference.s32 = new S32(*((S32*) value));
				break;
			case NVT_U32:
				mExpectedNameValueReference.u32 = new U32(*((U32*) value));
				break;
			case NVT_VEC3:
				mExpectedNameValueReference.vec3 = new LLVector3(*((LLVector3*) value));
				break;
			case NVT_U64:
				mExpectedNameValueReference.u64 = new U64(*((U64*) value));
			default:
				break;
			}
		}

		void verifyChange(LLNameValue* changed)
		{
			std::string str = "";
			str += "Expected Value of type: ";
			str += NameValueTypeStrings[mExpectedNameValueType];
			str += "not equal";

			switch(mExpectedNameValueType)
			{
			case NVT_STRING:
				ensure_memory_matches(str.c_str(),  changed->getString(), strlen(changed->getString()), mExpectedNameValueReference.string, strlen(mExpectedNameValueReference.string)); 
				break;
			case NVT_ASSET: 
				ensure_memory_matches(str.c_str(),  changed->getAsset(), strlen(changed->getAsset()), mExpectedNameValueReference.string, strlen(mExpectedNameValueReference.string)); 
				break;
			case NVT_F32:
				ensure(str, *(changed->getF32()) == *mExpectedNameValueReference.f32);
				break;
			case NVT_S32:
				ensure(str, *(changed->getS32()) == *mExpectedNameValueReference.s32);
				break;
			case NVT_U32:
				ensure(str, *(changed->getU32()) == *mExpectedNameValueReference.u32);
				break;
			case NVT_VEC3:
				ensure(str, *(changed->getVec3()) == *mExpectedNameValueReference.vec3);
				break;
			case NVT_U64:
				ensure(str, *(changed->getU64()) == *mExpectedNameValueReference.u64);
				break;
			default:
				break;
			}
		}

		void HandleCallback(LLNameValue* changed)
		{
			mCallbackCount++;
			verifyChange(changed);
			ensure("Callback called more than once", mCallbackCount == 1);
		}

		static void NameValueCallbackFunction(LLNameValue* changed, void** data)
		{
			namevalue_test* pNameValue = (namevalue_test*)data;
			pNameValue->HandleCallback(changed);
		}

		ENameValueType		mExpectedNameValueType;
		UNameValueReference	mExpectedNameValueReference;
		int  mCallbackCount;
 	};
	typedef test_group<namevalue_test> namevalue_t;
	typedef namevalue_t::object namevalue_object_t;
	tut::namevalue_t tut_namevalue("namevalue_test");

	
	template<> template<>
	void namevalue_object_t::test<1>()
	{
		// LLNameValue()
		LLNameValue nValue;
		ensure("mName should have been NULL", nValue.mName == NULL);
		ensure("getTypeEnum failed",nValue.getTypeEnum() == NVT_NULL);
		ensure("getClassEnum failed",nValue.getClassEnum() == NVC_NULL);
		ensure("getSendtoEnum failed",nValue.getSendtoEnum() == NVS_NULL);

		LLNameValue nValue1(" SecondLife ASSET RW SIM 232324343");

	}

	// LLNameValue(const char* data);
	// 	LLNameValue(const char* name, const char* data, const char* type, const char* nvclass, const char* nvsendto, 
	//				TNameValueCallback nvcb = NULL, void** user_data = NULL);
	template<> template<>
	void namevalue_object_t::test<2>()
	{
		LLNameValue nValue(" SecondLife ASSET RW S 232324343");
		ensure("mName not set correctly", (0 == strcmp(nValue.mName,"SecondLife")));
		ensure("getTypeEnum failed", nValue.getTypeEnum() == NVT_ASSET);
		ensure("getClassEnum failed", nValue.getClassEnum() == NVC_READ_WRITE);
		ensure("getSendtoEnum failed", nValue.getSendtoEnum() == NVS_SIM);
		ensure("getString failed", (0==strcmp(nValue.getAsset(),"232324343")));
		ensure("sendToData or sendToViewer failed", !nValue.sendToData() && !nValue.sendToViewer());
		
		LLNameValue nValue1("\n\r SecondLife_1 STRING READ_WRITE SIM 232324343");
		ensure("1. mName not set correctly", (0 == strcmp(nValue1.mName,"SecondLife_1")));
		ensure("1. getTypeEnum failed", nValue1.getTypeEnum() == NVT_STRING);
		ensure("1. getClassEnum failed", nValue1.getClassEnum() == NVC_READ_WRITE);
		ensure("1. getSendtoEnum failed", nValue1.getSendtoEnum() == NVS_SIM);
		ensure("1. getString failed", (0==strcmp(nValue1.getString(),"232324343")));
		ensure("1. sendToData or sendToViewer failed", !nValue1.sendToData() && !nValue1.sendToViewer());

		LLNameValue nValue2("SecondLife", "23.5", "F32", "R", "DS");
		ensure("2. getTypeEnum failed", nValue2.getTypeEnum() == NVT_F32);
		ensure("2. getClassEnum failed", nValue2.getClassEnum() == NVC_READ_ONLY);
		ensure("2. getSendtoEnum failed", nValue2.getSendtoEnum() == NVS_DATA_SIM);
		ensure("2. getF32 failed", *nValue2.getF32() == 23.5f);
		ensure("2. sendToData or sendToViewer failed", nValue2.sendToData() && !nValue2.sendToViewer());

		LLNameValue nValue3("SecondLife", "-43456787", "S32", "READ_ONLY", "SIM_SPACE");
		ensure("3. getTypeEnum failed", nValue3.getTypeEnum() == NVT_S32);
		ensure("3. getClassEnum failed", nValue3.getClassEnum() == NVC_READ_ONLY);
		ensure("3. getSendtoEnum failed", nValue3.getSendtoEnum() == NVS_DATA_SIM);
		ensure("3. getS32 failed", *nValue3.getS32() == -43456787);
		ensure("sendToData or sendToViewer failed", nValue3.sendToData() && !nValue3.sendToViewer());

		LLNameValue nValue4("SecondLife", "<1.0, 2.0, 3.0>", "VEC3", "CB", "SV");
		LLVector3 llvec4(1.0, 2.0, 3.0);
		ensure("4. getTypeEnum failed", nValue4.getTypeEnum() == NVT_VEC3);
		ensure("4. getClassEnum failed", nValue4.getClassEnum() == NVC_CALLBACK);
		ensure("4. getSendtoEnum failed", nValue4.getSendtoEnum() == NVS_SIM_VIEWER);
		ensure("4. getVec3 failed", *nValue4.getVec3() == llvec4);
		ensure("4. sendToData or sendToViewer failed", !nValue4.sendToData() && nValue4.sendToViewer());

		LLNameValue nValue5("SecondLife", "-1.0, 2.4, 3", "VEC3", "CALLBACK", "SIM_VIEWER");
		LLVector3 llvec5(-1.0f, 2.4f, 3);
		ensure("5. getTypeEnum failed", nValue5.getTypeEnum() == NVT_VEC3);
		ensure("5. getClassEnum failed", nValue5.getClassEnum() == NVC_CALLBACK);
		ensure("5. getSendtoEnum failed", nValue5.getSendtoEnum() == NVS_SIM_VIEWER);
		ensure("5. getVec3 failed", *nValue5.getVec3() == llvec5);
		ensure("5. sendToData or sendToViewer failed", !nValue5.sendToData() && nValue5.sendToViewer());

		LLNameValue nValue6("SecondLife", "89764323", "U32", "CALLBACK", "DSV");
		ensure("6. getTypeEnum failed", nValue6.getTypeEnum() == NVT_U32);
		ensure("6. getClassEnum failed", nValue6.getClassEnum() == NVC_CALLBACK);
		ensure("6. getSendtoEnum failed", nValue6.getSendtoEnum() == NVS_DATA_SIM_VIEWER);
		ensure("6. getU32 failed", *nValue6.getU32() == 89764323);
		ensure("6. sendToData or sendToViewer failed", nValue6.sendToData() && nValue6.sendToViewer());

		LLNameValue nValue7("SecondLife", "89764323323232", "U64", "CALLBACK", "SIM_SPACE_VIEWER");
		U64 u64_7 = U64L(89764323323232);
		ensure("7. getTypeEnum failed", nValue7.getTypeEnum() == NVT_U64);
		ensure("7. getClassEnum failed", nValue7.getClassEnum() == NVC_CALLBACK);
		ensure("7. getSendtoEnum failed", nValue7.getSendtoEnum() == NVS_DATA_SIM_VIEWER);
		ensure("7. getU32 failed", *nValue7.getU64() == u64_7);
		ensure("7. sendToData or sendToViewer failed", nValue7.sendToData() && nValue7.sendToViewer());
	}

	// LLNameValue(const char* name, const char* data, const char* type, const char* nvclass, 
	//				TNameValueCallback nvcb = NULL, void** user_data = NULL);
	template<> template<>
	void namevalue_object_t::test<3>()
	{
		LLNameValue nValue("SecondLife", "232324343", "ASSET", "READ_WRITE");
		ensure("mName not set correctly", (0 == strcmp(nValue.mName,"SecondLife")));
		ensure("getTypeEnum failed", nValue.getTypeEnum() == NVT_ASSET);
		ensure("getClassEnum failed", nValue.getClassEnum() == NVC_READ_WRITE);
		ensure("getSendtoEnum failed", nValue.getSendtoEnum() == NVS_SIM);
		ensure("getString failed", (0==strcmp(nValue.getAsset(),"232324343")));

		LLNameValue nValue1("SecondLife", "232324343", "STRING", "READ_WRITE");
		ensure("1. mName not set correctly", (0 == strcmp(nValue1.mName,"SecondLife")));
		ensure("1. getTypeEnum failed", nValue1.getTypeEnum() == NVT_STRING);
		ensure("1. getClassEnum failed", nValue1.getClassEnum() == NVC_READ_WRITE);
		ensure("1. getSendtoEnum failed", nValue1.getSendtoEnum() == NVS_SIM);
		ensure("1. getString failed", (0==strcmp(nValue1.getString(),"232324343")));

		LLNameValue nValue2("SecondLife", "23.5", "F32", "R");
		ensure("2. getTypeEnum failed", nValue2.getTypeEnum() == NVT_F32);
		ensure("2. getClassEnum failed", nValue2.getClassEnum() == NVC_READ_ONLY);
		ensure("2. getSendtoEnum failed", nValue2.getSendtoEnum() == NVS_SIM);
		ensure("2. getF32 failed", *nValue2.getF32() == 23.5f);

		LLNameValue nValue3("SecondLife", "-43456787", "S32", "READ_ONLY");
		ensure("3. getTypeEnum failed", nValue3.getTypeEnum() == NVT_S32);
		ensure("3. getClassEnum failed", nValue3.getClassEnum() == NVC_READ_ONLY);
		ensure("3. getSendtoEnum failed", nValue3.getSendtoEnum() == NVS_SIM);
		ensure("3. getS32 failed", *nValue3.getS32() == -43456787);

		LLNameValue nValue4("SecondLife", "<1.0, 2.0, 3.0>", "VEC3", "CB");
		LLVector3 llvec4(1.0, 2.0, 3.0);
		ensure("4. getTypeEnum failed", nValue4.getTypeEnum() == NVT_VEC3);
		ensure("4. getClassEnum failed", nValue4.getClassEnum() == NVC_CALLBACK);
		ensure("4. getSendtoEnum failed", nValue4.getSendtoEnum() == NVS_SIM);
		ensure("4. getVec3 failed", *nValue4.getVec3() == llvec4);

		LLNameValue nValue5("SecondLife", "-1.0, 2.4, 3", "VEC3", "CALLBACK");
		LLVector3 llvec5(-1.0f, 2.4f, 3);
		ensure("5. getTypeEnum failed", nValue5.getTypeEnum() == NVT_VEC3);
		ensure("5. getClassEnum failed", nValue5.getClassEnum() == NVC_CALLBACK);
		ensure("5. getSendtoEnum failed", nValue5.getSendtoEnum() == NVS_SIM);
		ensure("5. getVec3 failed", *nValue5.getVec3() == llvec5);

		LLNameValue nValue6("SecondLife", "89764323", "U32", "CALLBACK");
		ensure("6. getTypeEnum failed", nValue6.getTypeEnum() == NVT_U32);
		ensure("6. getClassEnum failed", nValue6.getClassEnum() == NVC_CALLBACK);
		ensure("6. getSendtoEnum failed", nValue6.getSendtoEnum() == NVS_SIM);
		ensure("6. getU32 failed", *nValue6.getU32() == 89764323);

		LLNameValue nValue7("SecondLife", "89764323323232", "U64", "CALLBACK");
		U64 u64_7 = U64L(89764323323232);
		ensure("7. getTypeEnum failed", nValue7.getTypeEnum() == NVT_U64);
		ensure("7. getClassEnum failed", nValue7.getClassEnum() == NVC_CALLBACK);
		ensure("7. getSendtoEnum failed", nValue7.getSendtoEnum() == NVS_SIM);
		ensure("7. getU32 failed", *nValue7.getU64() == u64_7);
	}

	// LLNameValue(const char* name, const char* type, const char* nvclass, 
	//				TNameValueCallback nvcb = NULL, void** user_data = NULL);
	template<> template<>
	void namevalue_object_t::test<4>()
	{
		LLNameValue nValue("SecondLife",  "STRING", "READ_WRITE");
		ensure("mName not set correctly", (0 == strcmp(nValue.mName,"SecondLife")));
		ensure("getTypeEnum failed", nValue.getTypeEnum() == NVT_STRING);
		ensure("getClassEnum failed", nValue.getClassEnum() == NVC_READ_WRITE);
		ensure("getSendtoEnum failed", nValue.getSendtoEnum() == NVS_SIM);

		LLNameValue nValue1("SecondLife",  "ASSET", "READ_WRITE");
		ensure("1. mName not set correctly", (0 == strcmp(nValue1.mName,"SecondLife")));
		ensure("1. getTypeEnum for RW failed", nValue1.getTypeEnum() == NVT_ASSET);
		ensure("1. getClassEnum for RW failed", nValue1.getClassEnum() == NVC_READ_WRITE);
		ensure("1. getSendtoEnum for RW failed", nValue1.getSendtoEnum() == NVS_SIM);

		LLNameValue nValue2("SecondLife", "F32", "READ_ONLY");
		ensure("2. getTypeEnum failed", nValue2.getTypeEnum() == NVT_F32);
		ensure("2. getClassEnum failed", nValue2.getClassEnum() == NVC_READ_ONLY);
		ensure("2. getSendtoEnum failed", nValue2.getSendtoEnum() == NVS_SIM);

		LLNameValue nValue3("SecondLife", "S32", "READ_ONLY");
		ensure("3. getTypeEnum failed", nValue3.getTypeEnum() == NVT_S32);
		ensure("3. getClassEnum failed", nValue3.getClassEnum() == NVC_READ_ONLY);
		ensure("3. getSendtoEnum failed", nValue3.getSendtoEnum() == NVS_SIM);

		skip_fail("NVC_CALLBACK does not parse.");

		LLNameValue nValue4("SecondLife", "VEC3", "CALLBACK");
		ensure("4. getTypeEnum failed", nValue4.getTypeEnum() == NVT_VEC3);
		ensure("4. getClassEnum failed", nValue4.getClassEnum() == NVC_CALLBACK);
		ensure("4. getSendtoEnum failed", nValue4.getSendtoEnum() == NVS_SIM);

		LLNameValue nValue6("SecondLife", "U32", "CALLBACK");
		ensure("6. getTypeEnum failed", nValue6.getTypeEnum() == NVT_U32);
		ensure("6. getClassEnum failed", nValue6.getClassEnum() == NVC_CALLBACK);
		ensure("6. getSendtoEnum failed", nValue6.getSendtoEnum() == NVS_SIM);

		LLNameValue nValue7("SecondLife", "U64", "CALLBACK");
		ensure("7. getTypeEnum failed", nValue7.getTypeEnum() == NVT_U64);
		ensure("7. getClassEnum failed", nValue7.getClassEnum() == NVC_CALLBACK);
		ensure("7. getSendtoEnum failed", nValue7.getSendtoEnum() == NVS_SIM);
	}

	template<> template<>
	void namevalue_object_t::test<5>()
	{
		skip_fail("callback will be called more than once.");
		LLNameValue nValue("SecondLife", "This is a test", "STRING", "CB", "SIM", NameValueCallbackFunction, (void**) this);

		ensure("getString failed", (0 == strcmp(nValue.getString(),"This is a test")));
		reset();

		setExpectedResult(NVT_STRING, (void*)"New Value");
		nValue.setString("New Value");
		ensure("String nonzero failed", nValue.nonzero() == TRUE);
		reset();
		setExpectedResult(NVT_STRING, (void*)"");
		nValue.setString("");
		ensure("String nonzero failed", nValue.nonzero() == FALSE);
		reset();
	}

	template<> template<>
	void namevalue_object_t::test<6>()
	{
		skip_fail("callback will be called more than once.");
		LLNameValue nValue("SecondLife", "This is a test", "ASSET", "CALLBACK", "S", NameValueCallbackFunction, (void**) this);
		ensure("getAsset failed", (0 == strcmp(nValue.getAsset(),"This is a test")));
		reset();

		setExpectedResult(NVT_ASSET, (void*)"New Value");
		nValue.setAsset("New Value");
		reset();
	}
         	
	template<> template<>
	void namevalue_object_t::test<7>()
	{
		skip_fail("callback will be called more than once.");
		LLNameValue nValue("SecondLife", "555555", "F32", "CB", "SIM", NameValueCallbackFunction, (void**) this);
		
		ensure("getF32 failed",*nValue.getF32() == 555555.f);
		reset();

		F32 fVal = 0.1f;
		setExpectedResult(NVT_F32, &fVal);
		nValue.setF32(fVal);

		fVal = -11111.1f;
		setExpectedResult(NVT_F32, &fVal);
		nValue.setF32(fVal);
		ensure("F32 nonzero failed", nValue.nonzero() == TRUE);
		reset();

		fVal = 0.;
		setExpectedResult(NVT_F32, &fVal);
		nValue.setF32(fVal);
		ensure("F32 nonzero failed", nValue.nonzero() == FALSE);
		reset();
	}

	template<> template<>
	void namevalue_object_t::test<8>()
	{
		skip_fail("callback will be called more than once.");
		LLNameValue nValue("SecondLife", "-5555", "S32", "CB", "SIM", NameValueCallbackFunction, (void**) this);

		ensure("getS32 failed", *nValue.getS32() == -5555);
		reset();

		S32 sVal = 0x7FFFFFFF;
		setExpectedResult(NVT_S32, &sVal);
		nValue.setS32(sVal);

		sVal = -0x7FFFFFFF;
		setExpectedResult(NVT_S32, &sVal);
		nValue.setS32(sVal);
		ensure("S32 nonzero failed", nValue.nonzero() == TRUE);
		reset();

		sVal = 0;
		setExpectedResult(NVT_S32, &sVal);
		nValue.setS32(sVal);
		ensure("S32 nonzero failed", nValue.nonzero() == FALSE);
		reset();
	}
	
	template<> template<>
	void namevalue_object_t::test<9>()
	{
		LLNameValue nValue("SecondLife", "<-3, 2, 1>", "VEC3", "CB", "SIM", NameValueCallbackFunction, (void**) this);
		LLVector3 vecExpected(-3, 2, 1);
		LLVector3 vec;
		nValue.getVec3(vec);
		ensure("getVec3 failed", vec == vecExpected);
		reset();

		vecExpected.setVec(2, -1, 0);
		setExpectedResult(NVT_VEC3, &vecExpected);
		nValue.setVec3(vecExpected);
		ensure("VEC3 nonzero failed", nValue.nonzero() == TRUE);
		reset();

		vecExpected.setVec(0, 0, 0);
		setExpectedResult(NVT_VEC3, &vecExpected);
		nValue.setVec3(vecExpected);
		ensure("VEC3 nonzero failed", nValue.nonzero() == FALSE);
		reset();
	}
	
	template<> template<>
	void namevalue_object_t::test<10>()
	{
		LLNameValue nValue("SecondLife", "12345678", "U32", "CB", "SIM", NameValueCallbackFunction, (void**) this);

		ensure("getU32 failed",*nValue.getU32() == 12345678);

		U32 val = 0xFFFFFFFF;
		setExpectedResult(NVT_U32, &val);
		nValue.setU32(val);
		ensure("U32 nonzero failed", nValue.nonzero() == TRUE);
		reset();

		val = 0;
		setExpectedResult(NVT_U32, &val);
		nValue.setU32(val);		
		ensure("U32 nonzero failed", nValue.nonzero() == FALSE);
		reset();
	}
	
	template<> template<>
	void namevalue_object_t::test<11>()
	{
		skip_fail("incomplete support for U64.");
		LLNameValue nValue("SecondLife", "44444444444", "U64", "CB", "SIM", NameValueCallbackFunction, (void**) this);

		ensure("getU64 failed",*nValue.getU64() == U64L(44444444444));
		ensure("U64 nonzero failed", nValue.nonzero() == TRUE);

		// there is no LLNameValue::setU64()  
	}
	
	template<> template<>
	void namevalue_object_t::test<12>()
	{
		LLNameValue nValue("SecondLife F32 RW SIM -333.337600");	
		F32 val = nValue.magnitude();
		ensure_equals("F32 magnitude failed", val, 333.337600f);
	
		LLNameValue nValue1("SecondLife STRING RW SIM 3300");	
		val = nValue1.magnitude();
		ensure_equals("STRING magnitude failed",val,4.0f);
		
		LLNameValue nValue2("SecondLife S32 RW SIM -3300");	
		val = nValue2.magnitude();
		ensure_equals("S32 magnitude failed", val, 3300.);
		
		LLNameValue nValue3("SecondLife U32 RW SIM 3300");	
		val = nValue3.magnitude();
		ensure_equals("U32 magnitude failed", val, 3300.);
		
		LLNameValue nValue4("SecondLife VEC3 RW SIM <1,2,3>");	
		LLVector3 vec(1,2,3);
		val = nValue4.magnitude();
		ensure_equals("VEC3 magnitude failed", val, vec.magVec());
		
		skip_fail("incomplete support for U64.");
		LLNameValue nValue5("SecondLife U64 RW SIM 12345");	
		val = nValue5.magnitude();
		ensure_equals("U62 magnitude failed", val, 12345);		
	}
	
	template<> template<>
	void namevalue_object_t::test<13>()
	{
		skip_fail("incomplete support for U64.");
		LLNameValue nValue("SecondLife U64 RW DSV 44444444444");	
		std::string ret_str = nValue.printNameValue();
		
		ensure_equals("1:printNameValue failed",ret_str,"SecondLife U64 RW DSV 44444444444");
		
		LLNameValue nValue1(ret_str.c_str());
		ensure_equals("Serialization of printNameValue failed", nValue, nValue1);		
	}

	template<> template<>
	void namevalue_object_t::test<14>()
	{
		LLNameValue nValue("SecondLife STRING RW DSV 44444444444");	
		std::string ret_str = nValue.printData();
		ensure_equals("1:printData failed",ret_str,"44444444444");

		LLNameValue nValue1("SecondLife S32 RW DSV 44444");	
		ret_str = nValue1.printData();
		ensure_equals("2:printData failed",ret_str,"44444");		
	}
	
	template<> template<>
	void namevalue_object_t::test<15>()
	{
		LLNameValue nValue("SecodLife STRING RW SIM 22222");
		std::ostringstream stream1,stream2,stream3, stream4, stream5;
		stream1 << nValue;
		ensure_equals("STRING << failed",stream1.str(),"22222");

		LLNameValue nValue1("SecodLife F32 RW SIM 22222");
		stream2 << nValue1;
		ensure_equals("F32 << failed",stream2.str(),"22222");

		LLNameValue nValue2("SecodLife S32 RW SIM 22222");
		stream3<< nValue2;
		ensure_equals("S32 << failed",stream3.str(),"22222");

		LLNameValue nValue3("SecodLife U32 RW SIM 122222");
		stream4<< nValue3;
		ensure_equals("U32 << failed",stream4.str(),"122222");

		skip_fail("incomplete support for U64.");
		LLNameValue nValue4("SecodLife U64 RW SIM 22222");
		stream5<< nValue4;
		ensure("U64 << failed",0 == strcmp((stream5.str()).c_str(),"22222"));
	}

	template<> template<>
	void namevalue_object_t::test<16>()
	{
		LLNameValue nValue1("SecondLife STRING RW DSV 44444");
		LLNameValue nValue2("SecondLife STRING RW SIM 33333");
		LLNameValue nValue3("SecondLife");
		nValue3 = nValue1 + nValue2;
		ensure("1:operator+ failed",(0==strcmp(nValue3.getString(),"4444433333")));
		
		LLNameValue nValue4("SecondLife F32 R DSV 44444");
		LLNameValue nValue5("SecondLife F32 RW SIM 33333");
		LLNameValue nValue6("SecondLife");
		nValue6 = nValue4 + nValue5;
		ensure_equals("2:operator+ failed",*nValue6.getF32(),77777.0);
		
		LLNameValue nValue7("SecondLife F32 R DSV 44444");
		LLNameValue nValue8("SecondLife S32 RW SIM 33333");
		LLNameValue nValue9("SecondLife F32");
		nValue9 = nValue7 + nValue8;
		ensure_equals("3:operator+ failed",*nValue9.getF32(),77777.0);
		
		LLNameValue nValue10("SecondLife VEC3 RW SIM <4, 4, 4>");
		LLNameValue nValue11("SecondLife VEC3 RW SV <3, 3, 3>");
		LLNameValue nValue12("SecondLife VEC3");
		nValue12 = nValue10 + nValue11;
		LLVector3 vec(7,7,7);
		ensure_equals("4:operator+ failed",*nValue12.getVec3(), vec);	
	}

	template<> template<>
	void namevalue_object_t::test<17>()
	{
		LLNameValue nValue7(" SecondLife S32 RW SIM 22222");
		LLNameValue nValue8(" SecondLife  F32 RW SIM 33333");
		LLNameValue nValue9(" SecondLife F32");
		nValue9 = nValue7 - nValue8;
		ensure_equals("1:operator- failed",*nValue9.getF32(),-11111.f);
		
		LLNameValue nValue10(" SecondLife VEC3 RW SIM <2, 2, 2>");
		LLNameValue nValue11(" SecondLife VEC3 RW SIM <3, 3, 3>");
		LLNameValue nValue12(" SecondLife VEC3");
		LLVector3 vec(-1,-1,-1);
		nValue12 = nValue10 - nValue11;
		ensure_equals("2:operator- failed",*nValue12.getVec3(), vec);
	}

	template<> template<>
	void namevalue_object_t::test<18>()
	{

		LLNameValue nValue1(" SecondLife F32 RW SIM 22222");
		LLNameValue nValue2(" SecondLife F32 RW SIM 33333");
		LLNameValue nValue3(" SecondLife F32");
		nValue3 = nValue1 * nValue2;
		ensure_equals("1:operator* failed",*nValue3.getF32(),740725926.f);
		
		LLNameValue nValue4(" SecondLife S32 RW SIM 22222");
		LLNameValue nValue5(" SecondLife  F32 RW SIM 33333");
		LLNameValue nValue6(" SecondLife F32");
		nValue6 = nValue4 * nValue5;
		ensure_equals("2:operator* failed",*nValue6.getF32(),740725926.f);

		LLNameValue nValue10(" SecondLife VEC3 RW SIM <2, 2, 2>");
		LLNameValue nValue11(" SecondLife VEC3 RW SIM <3, 3, 3>");
		LLNameValue nValue12(" SecondLife F32");
		LLVector3 vec1(2,2,2);
		LLVector3 vec2(3,3,3);
		nValue12 = nValue10 * nValue11;
		ensure_equals("2:operator* failed",*nValue12.getF32(), (vec1 * vec2));
	}

	template<> template<>
	void namevalue_object_t::test<19>()
	{
		LLNameValue nValue1(" SecondLife S32 RW SIM 22222");
		LLNameValue nValue2(" Virtual F32 RW SIM 44444");
		LLNameValue nValue3(" SecondLife F32");
		nValue3 = nValue1 / nValue2;
		ensure_equals("1:operator/ failed",*nValue3.getF32(),0.5);
		
		LLNameValue nValue4(" SecondLife F32 RW SIM 33333");
		LLNameValue nValue5(" SecondLife S32 RW SIM 22222");
		LLNameValue nValue6(" SecondLife F32");
		nValue6 = nValue4 / nValue5;
		ensure_equals("2:operator/ failed",*nValue6.getF32(),1.5);
	}
	
	template<> template<>
	void namevalue_object_t::test<20>()
	{
		LLNameValue nValue1(" SecondLife S32 RW SIM 22222");
		LLNameValue nValue2(" Virtual S32 RW SIM 33333");
		LLNameValue nValue3(" SecondLife S32");
 		nValue3 = nValue1 % nValue2;
		ensure_equals("1:operator% failed",*nValue3.getS32(),22222);

		LLNameValue nValue4(" SecondLife U32 RW SIM 3");
		LLNameValue nValue5(" SecondLife S32 RW SIM 2");
		LLNameValue nValue6(" SecondLife S32");
		nValue6 = nValue4 % nValue5;
		ensure_equals("2:operator% failed",*nValue6.getS32(),1);

		LLNameValue nValue10(" SecondLife VEC3 RW SIM <4, 5, 6>");
		LLNameValue nValue11(" SecondLife VEC3 RW SIM <1, 2, 3>");
		LLNameValue nValue12(" SecondLife VEC3");
		LLVector3 vec1(4,5,6);
		LLVector3 vec2(1,2,3);
		LLVector3 vec3(vec1 % vec2);
		nValue12 = nValue10 % nValue11;
		ensure_equals("5:operator% failed",*nValue12.getVec3(), vec3);
	}

	template<> template<>
	void namevalue_object_t::test<21>()
	{
		LLNameValue nValue1(" SecondLife STRING RW SIM 22222");
		LLNameValue nValue2(" Virtual STRING RW SIM 22222");
		ensure("1:operator== failed", nValue1 == nValue2);

		LLNameValue nValue3(" SecondLife F32 RW SIM 33333");
		LLNameValue nValue4(" Virtual F32 RW SIM 22222");
		ensure("2:operator== failed",!(nValue3 == nValue4));

		LLNameValue nValue5(" SecondLife STRING RW SIM 22222");
		LLNameValue nValue6(" Virtual STRING RW SIM 33333");
		ensure("3:operator== failed",!(nValue5 == nValue6));

		LLNameValue nValue7(" SecondLife VEC3 RW SIM <2, 2, 2>");
		LLNameValue nValue8(" Virtual VEC3 RW SIM <2, 2, 2>");
		ensure("4:operator== failed",(nValue7 == nValue8));		
	}

	template<> template<>
	void namevalue_object_t::test<22>()
	{
		LLNameValue nValue1(" SecondLife STRING RW SIM 22222");
		LLNameValue nValue2(" Virtual STRING RW SIM 33333");
		bool b_ret = (nValue1 <= nValue2) ? 1 : 0;
		ensure("1:operator<= failed",(1==b_ret));	

		LLNameValue nValue3(" SecondLife F32 RW SIM 33333");
		LLNameValue nValue4(" Virtual F32 RW SIM 22222");
		b_ret = (nValue3 <= nValue4) ? 1 : 0;
		ensure("2:operator<= failed",(0==b_ret));
	}

	template<> template<>
	void namevalue_object_t::test<23>()
	{
		LLNameValue nValue1(" SecondLife STRING RW SIM 22222");
		LLNameValue nValue2(" Virtual STRING RW SIM 33333");
		bool b_ret = (nValue1 >= nValue2) ? 1 : 0;
		ensure("operator>= failed",!b_ret);	

		LLNameValue nValue3(" SecondLife F32 RW SIM 33333");
		LLNameValue nValue4(" Virtual F32 RW SIM 22222");
		b_ret = (nValue3 >= nValue4) ? 1 : 0;
		ensure("2:operator<= failed",b_ret);

	}

	template<> template<>
	void namevalue_object_t::test<24>()
	{
		LLNameValue nValue1(" SecondLife STRING RW SIM 33333");
		LLNameValue nValue2(" Virtual STRING RW SIM 33333");
		bool b_ret = (nValue1 < nValue2) ? 1 : 0;
		ensure("operator< failed",!b_ret);	

		LLNameValue nValue3(" SecondLife F32 RW SIM 11111");
		LLNameValue nValue4(" Virtual F32 RW SIM 22222");
		b_ret = (nValue3 < nValue4) ? 1 : 0;
		ensure("2:operator< failed",b_ret);

	}

	template<> template<>
	void namevalue_object_t::test<25>()
	{
		LLNameValue nValue1(" SecondLife STRING RW SIM 33333");
		LLNameValue nValue2(" Virtual STRING RW SIM 33333");
		bool b_ret = (nValue1 > nValue2) ? 1 : 0;
		ensure("1:operator> failed",!b_ret);	

		LLNameValue nValue3(" SecondLife F32 RW SIM 11111");
		LLNameValue nValue4(" Virtual F32 RW SIM 22222");
		b_ret = (nValue3 > nValue4) ? 1 : 0;
		ensure("2:operator> failed",!b_ret);

		LLNameValue nValue5(" SecondLife S32 RW SIM 22222");
		LLNameValue nValue6(" Virtual F32 RW SIM 11111");
		b_ret = (nValue5 > nValue6) ? 1 : 0;
		ensure("3:operator> failed",b_ret);
	}

	template<> template<>
	void namevalue_object_t::test<26>()
	{
		LLNameValue nValue1(" SecondLife STRING RW SIM 33333");
		LLNameValue nValue2(" Virtual STRING RW SIM 33333");
		bool b_ret = (nValue1 != nValue2) ? 1 : 0;
		ensure("1:operator!= failed",!b_ret);	

		LLNameValue nValue3(" SecondLife F32 RW SIM 11111");
		LLNameValue nValue4(" Virtual F32 RW SIM 22222");
		b_ret = (nValue3 != nValue4) ? 1 : 0;
		ensure("2:operator!= failed",b_ret);

	}
	

	template<> template<>
	void namevalue_object_t::test<27>()
	{
		LLNameValue nValue1(" SecondLife F32 RW SIM 33333");
		LLNameValue nValue2("Virtual");
		nValue2 = -nValue1;
		ensure_equals("1:operator unary- failed",*nValue2.getF32(), -33333.f);

		LLNameValue nValue3(" SecondLife U32 RW SIM 11111");
		LLNameValue nValue4("Virtual S32");
		nValue4 = -nValue3;
		ensure_equals("2:operator unary- failed",*nValue4.getS32(), -11111);

		LLNameValue nValue5(" SecondLife VEC3 RW SIM <1, 1, 1>");
		LLNameValue nValue6("Virtual VEC3");
		LLVector3 vec(-1, -1, -1);
		nValue6 = -nValue5;
		ensure_equals("3:operator unary- failed",*nValue6.getVec3(), vec);
	}	

	template<> template<>
	void namevalue_object_t::test<28>()
	{
		LLNameValue nValue("SecondLife", "This is a test", "ASSET", "R", "S", NameValueCallbackFunction, (void**) this);

		ensure("getAsset failed", (0 == strcmp(nValue.getAsset(),"This is a test")));
		// this should not have updated as it is read only.
		nValue.setAsset("New Value should not be updated");
		ensure("setAsset on ReadOnly failed", (0 == strcmp(nValue.getAsset(),"This is a test")));

		LLNameValue nValue1("SecondLife", "1234", "U32", "R", "S", NameValueCallbackFunction, (void**) this);
		// this should not have updated as it is read only.
		nValue1.setU32(4567);
		ensure("setU32 on ReadOnly failed", *nValue1.getU32() == 1234);

		LLNameValue nValue2("SecondLife", "1234", "S32", "R", "S", NameValueCallbackFunction, (void**) this);
		// this should not have updated as it is read only.
		nValue2.setS32(4567);
		ensure("setS32 on ReadOnly failed", *nValue2.getS32() == 1234);

		LLNameValue nValue3("SecondLife", "1234", "F32", "R", "S", NameValueCallbackFunction, (void**) this);
		// this should not have updated as it is read only.
		nValue3.setF32(4567);
		ensure("setF32 on ReadOnly failed", *nValue3.getF32() == 1234);
		nValue3 = nValue3 * 2;
		ensure("setF32 on ReadOnly failed", *nValue3.getF32() == 1234);

		LLNameValue nValue4("SecondLife", "<1,2,3>", "VEC3", "R", "S", NameValueCallbackFunction, (void**) this);
		// this should not have updated as it is read only.
		LLVector3 vec(4,5,6);
		nValue3.setVec3(vec);
		LLVector3 vec1(1,2,3);
		ensure("setVec3 on ReadOnly failed", *nValue4.getVec3() == vec1);

		// cant test for U64 as no set64 exists nor any operators support U64 type
	}         
}
