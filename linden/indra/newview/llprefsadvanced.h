/**
* @file llprefsadvanced.h
* @brief Advanced preferences options for Imprudence
*
* $LicenseInfo:firstyear=2009&license=viewergpl$
*
* Copyright (c) 2010, McCabe Maxsted
*
* Imprudence Viewer Source Code
* The source code in this file ("Source Code") is provided to you
* under the terms of the GNU General Public License, version 2.0
* ("GPL"). Terms of the GPL can be found in doc/GPL-license.txt in
* this distribution, or online at
* http://secondlifegrid.net/programs/open_source/licensing/gplv2
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
* ALL SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
* WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
* COMPLETENESS OR PERFORMANCE.
* $/LicenseInfo$
*/

#ifndef LLPREFSADVANCED_H
#define LLPREFSADVANCED_H

#include "llpanel.h"
#include "llviewerinventory.h"
class LLColor4;

class LLPrefsAdvanced : public LLPanel
{
public:
	LLPrefsAdvanced();
	~LLPrefsAdvanced();

	BOOL postBuild();

	void apply();
	void cancel();
	void refresh();
	void draw();
private:
	static LLPrefsAdvanced* sInstance;

	static void onCommitCheckBox(LLUICtrl* ctrl, void* user_data);
	static void onResetThisCloudButton(void * data);
	static void onSaveThisCloudButton(void * data);
	static void onSaveAnyoneCloudButton(void * data);

protected:
	void setParticleControls(bool is_logged_in);

private:
	bool mWasLoggedIn;
	LLColor4 mCloudStartColor;
	LLColor4 mCloudEndColor;
	LLUUID mCloudTextureID;
	BOOL mWLControl;
};

#endif // LLPREFSADVANCED_H
