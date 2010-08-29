/* Copyright (C) 2010 LordGregGreg Back

   This is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.
 
   This is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.
 
   You should have received a copy of the GNU Lesser General Public
   License along with the viewer; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */


#ifndef LGG_AUTO_CORRECT
#define LGG_AUTO_CORRECT
class LGGAutoCorrect
{
	LGGAutoCorrect();
	~LGGAutoCorrect();
	static LGGAutoCorrect* sInstance;
public:
	static LGGAutoCorrect* getInstance();
	BOOL addCorrectionList(LLSD newList);
	BOOL removeCorrectionList(std::string listName);
	BOOL setListEnabled(std::string listName, BOOL enabled);
	BOOL setListAnnounceeState(std::string listName, BOOL announce);
	BOOL setListPriority(std::string listName, int priority);
	BOOL setListStyle(std::string listName, BOOL announce);
	std::string replaceWords(std::string words);
	std::string replaceWord(std::string currentWord);
	BOOL addEntryToList(std::string wrong, std::string right, std::string listName);
	BOOL removeEntryFromList(std::string wrong, std::string listName);
	BOOL saveListToDisk(std::string listName, std::string fileName);
	LLSD exportList(std::string listName);
	void runTest();
	LLSD getAutoCorrects();
	LLSD getAutoCorrectEntries(std::string listName);
	void save();

	void loadFromDisk();

private:
	void saveToDisk(LLSD newSettings);
	LLSD getExampleLLSD();	
	std::string getFileName();
	std::string getDefaultFileName();

	LLSD mAutoCorrects;

};



#endif 
