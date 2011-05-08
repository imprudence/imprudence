// <edit>
#include "llviewerprecompiledheaders.h"
#include "llfloatermessagebuilder.h"
#include "lluictrlfactory.h"
#include "llmessagetemplate.h"
#include "llagent.h"
#include "llchat.h"
#include "llfloaterchat.h"
#include "llviewerregion.h" // getHandle
#include "llcombobox.h"
#include "llselectmgr.h" // fill in stuff about selected object
#include "llparcel.h"
#include "llviewerparcelmgr.h" // same for parcel
#include "llscrolllistctrl.h"
#include "llworld.h"
#include "lltemplatemessagebuilder.h"

////////////////////////////////
// LLNetListItem
////////////////////////////////
LLNetListItem::LLNetListItem(LLUUID id)
:	mID(id),
	mAutoName(TRUE),
	mName("No name"),
	mPreviousRegionName(""),
	mCircuitData(NULL)
{
}

////////////////////////////////
// LLFloaterMessageBuilder
////////////////////////////////
std::list<LLNetListItem*> LLFloaterMessageBuilder::sNetListItems;

LLFloaterMessageBuilder::LLFloaterMessageBuilder(std::string initial_text)
:	LLFloater(),
	LLEventTimer(1.0f),
	mNetInfoMode(NI_NET),
	mInitialText(initial_text)
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_message_builder.xml");
}
LLFloaterMessageBuilder::~LLFloaterMessageBuilder()
{
}
void LLFloaterMessageBuilder::show(std::string initial_text)
{
	(new LLFloaterMessageBuilder(initial_text))->open();
}
BOOL LLFloaterMessageBuilder::tick()
{
	refreshNetList();
	return FALSE;
}
LLNetListItem* LLFloaterMessageBuilder::findNetListItem(LLHost host)
{
	std::list<LLNetListItem*>::iterator end = sNetListItems.end();
	for(std::list<LLNetListItem*>::iterator iter = sNetListItems.begin(); iter != end; ++iter)
		if((*iter)->mCircuitData && (*iter)->mCircuitData->getHost() == host)
			return (*iter);
	return NULL;
}
LLNetListItem* LLFloaterMessageBuilder::findNetListItem(LLUUID id)
{
	std::list<LLNetListItem*>::iterator end = sNetListItems.end();
	for(std::list<LLNetListItem*>::iterator iter = sNetListItems.begin(); iter != end; ++iter)
		if((*iter)->mID == id)
			return (*iter);
	return NULL;
}
void LLFloaterMessageBuilder::refreshNetList()
{
	LLScrollListCtrl* scrollp = getChild<LLScrollListCtrl>("net_list");
	// Update circuit data of net list items
	std::vector<LLCircuitData*> circuits = gMessageSystem->getCircuit()->getCircuitDataList();
	std::vector<LLCircuitData*>::iterator circuits_end = circuits.end();
	for(std::vector<LLCircuitData*>::iterator iter = circuits.begin(); iter != circuits_end; ++iter)
	{
		LLNetListItem* itemp = findNetListItem((*iter)->getHost());
		if(!itemp)
		{
			LLUUID id; id.generate();
			itemp = new LLNetListItem(id);
			sNetListItems.push_back(itemp);
		}
		itemp->mCircuitData = (*iter);
	}
	// Clear circuit data of items whose circuits are gone
	std::list<LLNetListItem*>::iterator items_end = sNetListItems.end();
	for(std::list<LLNetListItem*>::iterator iter = sNetListItems.begin(); iter != items_end; ++iter)
	{
		if(std::find(circuits.begin(), circuits.end(), (*iter)->mCircuitData) == circuits.end())
			(*iter)->mCircuitData = NULL;
	}
	// Remove net list items that are totally useless now
	for(std::list<LLNetListItem*>::iterator iter = sNetListItems.begin(); iter != sNetListItems.end();)
	{
		if((*iter)->mCircuitData == NULL)
			iter = sNetListItems.erase(iter);
		else ++iter;
	}
	// Update names of net list items
	items_end = sNetListItems.end();
	for(std::list<LLNetListItem*>::iterator iter = sNetListItems.begin(); iter != items_end; ++iter)
	{
		LLNetListItem* itemp = (*iter);
		if(itemp->mAutoName)
		{
			if(itemp->mCircuitData)
			{
				LLViewerRegion* regionp = LLWorld::getInstance()->getRegion(itemp->mCircuitData->getHost());
				if(regionp)
				{
					std::string name = regionp->getName();
					if(name == "") name = llformat("%s (awaiting region name)", itemp->mCircuitData->getHost().getString().c_str());
					itemp->mName = name;
					itemp->mPreviousRegionName = name;
				}
				else
				{
					itemp->mName = itemp->mCircuitData->getHost().getString();
					if(itemp->mPreviousRegionName != "")
						itemp->mName.append(llformat(" (was %s)", itemp->mPreviousRegionName.c_str()));
				}
			}
			else
			{
				// an item just for an event queue, not handled yet
				itemp->mName = "Something else";
			}
		}
	}
	// Rebuild scroll list from scratch
	LLUUID selected_id = scrollp->getFirstSelected() ? scrollp->getFirstSelected()->getUUID() : LLUUID::null;
	S32 scroll_pos = scrollp->getScrollPos();
	scrollp->clearRows();
	for(std::list<LLNetListItem*>::iterator iter = sNetListItems.begin(); iter != items_end; ++iter)
	{
		LLNetListItem* itemp = (*iter);
		LLSD element;
		element["id"] = itemp->mID;
		LLSD& text_column = element["columns"][0];
		text_column["column"] = "text";
		text_column["value"] = itemp->mName + (itemp->mCircuitData->getHost() == gAgent.getRegionHost() ? " (main)" : "");

		LLSD& state_column = element["columns"][ 1];
		state_column["column"] = "state";
		state_column["value"] = "";

		LLScrollListItem* scroll_itemp = scrollp->addElement(element);
		BOOL has_live_circuit = itemp->mCircuitData && itemp->mCircuitData->isAlive();

		LLScrollListText* state = (LLScrollListText*)scroll_itemp->getColumn(1);

		if(has_live_circuit)
			state->setText(std::string("Alive"));
		else
			state->setText(std::string("Alive"));
	}
	if(selected_id.notNull()) scrollp->selectByID(selected_id);
	if(scroll_pos < scrollp->getItemCount()) scrollp->setScrollPos(scroll_pos);
}
BOOL LLFloaterMessageBuilder::postBuild()
{
	childSetText("message_edit", mInitialText);
	childSetAction("send_btn", onClickSend, this);
	std::vector<std::string> names;
	LLComboBox* combo;
	LLMessageSystem::message_template_name_map_t::iterator temp_end = gMessageSystem->mMessageTemplates.end();
	LLMessageSystem::message_template_name_map_t::iterator temp_iter;
	std::vector<std::string>::iterator names_end;
	std::vector<std::string>::iterator names_iter;
	for(temp_iter = gMessageSystem->mMessageTemplates.begin(); temp_iter != temp_end; ++temp_iter)
		if((*temp_iter).second->getTrust() == MT_NOTRUST)
			names.push_back((*temp_iter).second->mName);
	std::sort(names.begin(), names.end());
	combo = getChild<LLComboBox>("untrusted_message_combo");
	names_end = names.end();
	for(names_iter = names.begin(); names_iter != names_end; ++names_iter)
		combo->add((*names_iter));
	names.clear();
	for(temp_iter = gMessageSystem->mMessageTemplates.begin(); temp_iter != temp_end; ++temp_iter)
		if((*temp_iter).second->getTrust() == MT_TRUST)
			names.push_back((*temp_iter).second->mName);
	std::sort(names.begin(), names.end());
	combo = getChild<LLComboBox>("trusted_message_combo");
	names_end = names.end();
	for(names_iter = names.begin(); names_iter != names_end; ++names_iter)
		combo->add((*names_iter));
	childSetCommitCallback("untrusted_message_combo", onCommitPacketCombo, this);
	childSetCommitCallback("trusted_message_combo", onCommitPacketCombo, this);
	return TRUE;
}
inline std::vector<std::string> split(std::string input, std::string separator)
{
	S32 size = input.length();
	char* buffer = new char[size + 1];
	strncpy(buffer, input.c_str(), size);
	buffer[size] = '\0';
	std::vector<std::string> lines;
	char* result = strtok(buffer, separator.c_str());
	while(result)
	{
		lines.push_back(result);
		result = strtok(NULL, separator.c_str());
	}
	delete[] buffer;
	return lines;
}
std::string mvtstr(e_message_variable_type var_type)
{
	switch(var_type)
	{
	case MVT_U8:
		return "U8";
		break;
	case MVT_U16:
		return "U16";
		break;
	case MVT_U32:
		return "U32";
		break;
	case MVT_U64:
		return "U64";
		break;
	case MVT_S8:
		return "S8";
		break;
	case MVT_S16:
		return "S16";
		break;
	case MVT_S32:
		return "S32";
		break;
	case MVT_S64:
		return "S64";
		break;
	case MVT_F32:
		return "F32";
		break;
	case MVT_F64:
		return "F64";
		break;
	case MVT_LLVector3:
		return "LLVector3";
		break;
	case MVT_LLVector3d:
		return "LLVector3d";
		break;
	case MVT_LLVector4:
		return "LLVector4";
		break;
	case MVT_LLQuaternion:
		return "LLQuaternion";
		break;
	case MVT_LLUUID:
		return "LLUUID";
		break;
	case MVT_BOOL:
		return "BOOL";
		break;
	case MVT_IP_ADDR:
		return "IPADDR";
		break;
	case MVT_IP_PORT:
		return "IPPORT";
		break;
	case MVT_VARIABLE:
		return "Variable";
		break;
	case MVT_FIXED:
		return "Fixed";
		break;
	default:
		return "Missingno.";
		break;
	}
}
// static
BOOL LLFloaterMessageBuilder::addField(e_message_variable_type var_type, const char* var_name, std::string input, BOOL hex)
{
	LLStringUtil::trim(input);
	if(input.length() < 1 && var_type != MVT_VARIABLE)
		return FALSE;
	U8 valueU8;
	U16 valueU16;
	U32 valueU32;
	U64 valueU64;
	S8 valueS8;
	S16 valueS16;
	S32 valueS32;
	// S64 valueS64;
	F32 valueF32;
	F64 valueF64;
	LLVector3 valueVector3;
	LLVector3d valueVector3d;
	LLVector4 valueVector4;
	LLQuaternion valueQuaternion;
	LLUUID valueLLUUID;
	BOOL valueBOOL;
	std::string input_lower = input;
	LLStringUtil::toLower(input_lower);
	if(input_lower == "$agentid")
		input = gAgent.getID().asString();
	else if(input_lower == "$sessionid")
		input = gAgent.getSessionID().asString();
	else if(input_lower == "$uuid")
	{
		LLUUID id;
		id.generate();
		input = id.asString();
	}
	else if(input_lower == "$circuitcode")
	{
		std::stringstream temp_stream;
		temp_stream << gMessageSystem->mOurCircuitCode;
		input = temp_stream.str();
	}
	else if(input_lower == "$regionhandle")
	{
		std::stringstream temp_stream;
		temp_stream << (gAgent.getRegion() ? gAgent.getRegion()->getHandle() : 0);
		input = temp_stream.str();
	}
	else if(input_lower == "$position" || input_lower == "$pos")
	{
		std::stringstream temp_stream;
		valueVector3 = gAgent.getPositionAgent();
		temp_stream << "<" << valueVector3[0] << ", " << valueVector3[1] << ", " << valueVector3[2] << ">";
		input = temp_stream.str();
	}
	if(hex)
	{
		if(var_type != MVT_VARIABLE && var_type != MVT_FIXED)
			return FALSE;
		int len = input_lower.length();
		const char* cstr = input_lower.c_str();
		std::string new_input("");
		BOOL nibble = FALSE;
		char byte = 0;
		for(int i = 0; i < len; i++)
		{
			char c = cstr[i];
			if(c >= 0x30 && c <= 0x39)
				c -= 0x30;
			else if(c >= 0x61 && c <= 0x66)
				c -= 0x57;
			else if(c != 0x20)
				return FALSE;
			else
				continue;
			if(!nibble)
				byte = c << 4;
			else
				new_input.push_back(byte | c);
			nibble = !nibble;
		}
		if(nibble)
			return FALSE;
		input = new_input;
	}
	std::stringstream stream(input);
	std::vector<std::string> tokens;
	switch(var_type)
	{
	case MVT_U8:
		if(input.substr(0, 1) == "-")
			return FALSE;
		if((stream >> valueU32).fail())
			return FALSE;
		valueU8 = (U8)valueU32;
		gMessageSystem->addU8(var_name, valueU8);
		return TRUE;
		break;
	case MVT_U16:
		if(input.substr(0, 1) == "-")
			return FALSE;
		if((stream >> valueU16).fail())
			return FALSE;
		gMessageSystem->addU16(var_name, valueU16);
		return TRUE;
		break;
	case MVT_U32:
		if(input.substr(0, 1) == "-")
			return FALSE;
		if((stream >> valueU32).fail())
			return FALSE;
		gMessageSystem->addU32(var_name, valueU32);
		return TRUE;
		break;
	case MVT_U64:
		if(input.substr(0, 1) == "-")
			return FALSE;
		if((stream >> valueU64).fail())
			return FALSE;
		gMessageSystem->addU64(var_name, valueU64);
		return TRUE;
		break;
	case MVT_S8:
		if((stream >> valueS8).fail())
			return FALSE;
		gMessageSystem->addS8(var_name, valueS8);
		return TRUE;
		break;
	case MVT_S16:
		if((stream >> valueS16).fail())
			return FALSE;
		gMessageSystem->addS16(var_name, valueS16);
		return TRUE;
		break;
	case MVT_S32:
		if((stream >> valueS32).fail())
			return FALSE;
		gMessageSystem->addS32(var_name, valueS32);
		return TRUE;
		break;
	/*
	case MVT_S64:
		if((stream >> valueS64).fail())
			return FALSE;
		gMessageSystem->addS64(var_name, valueS64);
		return TRUE;
		break;
	*/
	case MVT_F32:
		if((stream >> valueF32).fail())
			return FALSE;
		gMessageSystem->addF32(var_name, valueF32);
		return TRUE;
		break;
	case MVT_F64:
		if((stream >> valueF64).fail())
			return FALSE;
		gMessageSystem->addF64(var_name, valueF64);
		return TRUE;
		break;
	case MVT_LLVector3:
		LLStringUtil::trim(input);
		if(input.substr(0, 1) != "<" || input.substr(input.length() - 1, 1) != ">")
			return FALSE;
		tokens = split(input.substr(1, input.length() - 2), ",");
		if(tokens.size() != 3)
			return FALSE;
		for(int i = 0; i < 3; i++)
		{
			stream.clear();
			stream.str(tokens[i]);
			if((stream >> valueF32).fail())
				return FALSE;
			valueVector3.mV[i] = valueF32;
		}
		gMessageSystem->addVector3(var_name, valueVector3);
		return TRUE;
		break;
	case MVT_LLVector3d:
		LLStringUtil::trim(input);
		if(input.substr(0, 1) != "<" || input.substr(input.length() - 1, 1) != ">")
			return FALSE;
		tokens = split(input.substr(1, input.length() - 2), ",");
		if(tokens.size() != 3)
			return FALSE;
		for(int i = 0; i < 3; i++)
		{
			stream.clear();
			stream.str(tokens[i]);
			if((stream >> valueF64).fail())
				return FALSE;
			valueVector3d.mdV[i] = valueF64;
		}
		gMessageSystem->addVector3d(var_name, valueVector3d);
		return TRUE;
		break;
	case MVT_LLVector4:
		LLStringUtil::trim(input);
		if(input.substr(0, 1) != "<" || input.substr(input.length() - 1, 1) != ">")
			return FALSE;
		tokens = split(input.substr(1, input.length() - 2), ",");
		if(tokens.size() != 4)
			return FALSE;
		for(int i = 0; i < 4; i++)
		{
			stream.clear();
			stream.str(tokens[i]);
			if((stream >> valueF32).fail())
				return FALSE;
			valueVector4.mV[i] = valueF32;
		}
		gMessageSystem->addVector4(var_name, valueVector4);
		return TRUE;
		break;
	case MVT_LLQuaternion:
		LLStringUtil::trim(input);
		if(input.substr(0, 1) != "<" || input.substr(input.length() - 1, 1) != ">")
			return FALSE;
		tokens = split(input.substr(1, input.length() - 2), ",");
		if(tokens.size() != 3)
			return FALSE;
		for(int i = 0; i < 3; i++)
		{
			stream.clear();
			stream.str(tokens[i]);
			if((stream >> valueF32).fail())
				return FALSE;
			valueVector3.mV[i] = valueF32;
		}
		valueQuaternion.unpackFromVector3(valueVector3);
		gMessageSystem->addQuat(var_name, valueQuaternion);
		return TRUE;
		break;
	case MVT_LLUUID:
		if((stream >> valueLLUUID).fail())
			return FALSE;
		gMessageSystem->addUUID(var_name, valueLLUUID);
		return TRUE;
		break;
	case MVT_BOOL:
		if(input_lower == "true")
			valueBOOL = TRUE;
		else if(input_lower == "false")
			valueBOOL = FALSE;
		else if((stream >> valueBOOL).fail())
			return FALSE;
		//gMessageSystem->addBOOL(var_name, valueBOOL);
		gMessageSystem->addU8(var_name, (U8)valueBOOL);
		return TRUE;
		break;
	case MVT_IP_ADDR:
		if((stream >> valueU32).fail())
			return FALSE;
		gMessageSystem->addIPAddr(var_name, valueU32);
		return TRUE;
		break;
	case MVT_IP_PORT:
		if((stream >> valueU16).fail())
			return FALSE;
		gMessageSystem->addIPPort(var_name, valueU16);
		return TRUE;
		break;
	case MVT_VARIABLE:
		if(!hex)
		{
			char* buffer = new char[input.size() + 1];
			strncpy(buffer, input.c_str(), input.size());
			buffer[input.size()] = '\0';
			gMessageSystem->addBinaryData(var_name, buffer, input.size() + 1);
			delete[] buffer;
		}
		else
			gMessageSystem->addBinaryData(var_name, input.c_str(), input.size());
		return TRUE;
		break;
	case MVT_FIXED:
		if(!hex)
		{
			char* buffer = new char[input.size() + 1];
			strncpy(buffer, input.c_str(), input.size());
			buffer[input.size()] = '\0';
			gMessageSystem->addBinaryData(var_name, buffer, input.size());
			delete[] buffer;
		}
		else
			gMessageSystem->addBinaryData(var_name, input.c_str(), input.size());
		return TRUE;
		break;
	default:
		break;
	}
	return FALSE;
}
// static
void LLFloaterMessageBuilder::onCommitPacketCombo(LLUICtrl* ctrl, void* user_data)
{
	LLFloaterMessageBuilder* floaterp = (LLFloaterMessageBuilder*)user_data;
	LLViewerObject* selected_objectp = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
	LLParcel* agent_parcelp = LLViewerParcelMgr::getInstance()->getAgentParcel();
	std::string message = ctrl->getValue();
	std::map<const char *, LLMessageTemplate*>::iterator template_iter;
	template_iter = gMessageSystem->mMessageTemplates.find( LLMessageStringTable::getInstance()->getString(message.c_str()) );
	if(template_iter == gMessageSystem->mMessageTemplates.end())
	{
		floaterp->childSetText("message_edit", std::string(""));
		return;
	}
	std::string text(llformat((*template_iter).second->getTrust() == MT_NOTRUST ? "out %s\n" : "in %s\n", message.c_str()));
	LLMessageTemplate* temp = (*template_iter).second;
	LLMessageTemplate::message_block_map_t::iterator blocks_end = temp->mMemberBlocks.end();
	for (LLMessageTemplate::message_block_map_t::iterator blocks_iter = temp->mMemberBlocks.begin();
		 blocks_iter != blocks_end; ++blocks_iter)
	{
		LLMessageBlock* block = (*blocks_iter);
		const char* block_name = block->mName;
		std::string block_name_string = std::string(block_name);
		S32 num_blocks = 1;
		if(block->mType == MBT_MULTIPLE)
			num_blocks = block->mNumber;
		else if(("ObjectLink" == message && "ObjectData" == block_name_string))
			num_blocks = 2;
		for(S32 i = 0; i < num_blocks; i++)
		{
			text.append(llformat("[%s]\n", block_name));
			LLMessageBlock::message_variable_map_t::iterator var_end = block->mMemberVariables.end();
			for (LLMessageBlock::message_variable_map_t::iterator var_iter = block->mMemberVariables.begin();
				 var_iter != var_end; ++var_iter)
			{
				LLMessageVariable* variable = (*var_iter);
				const char* var_name = variable->getName();
				std::string var_name_string = std::string(var_name);
				text.append(llformat("%s = ", var_name));
				std::string value("");
				S32 size = variable->getSize();
				switch(variable->getType())
				{
				case MVT_U8:
				case MVT_U16:
				case MVT_U32:
				case MVT_U64:
				case MVT_S8:
				case MVT_S16:
				case MVT_S32:
				case MVT_IP_ADDR:
				case MVT_IP_PORT:
					if("RegionHandle" == var_name_string || "Handle" == var_name_string)
						value = "$RegionHandle";
					else if("CircuitCode" == var_name_string || "ViewerCircuitCode" == var_name_string
						|| ("Code" == var_name_string && "CircuitCode" == block_name_string) )
					{
						value = "$CircuitCode";
					}
					else if(selected_objectp && 
							(
								   "ObjectLocalID" == var_name_string
								|| "TaskLocalID" == var_name_string
								|| ("LocalID" == var_name_string && 
										(
											   "ObjectData" == block_name_string
											|| "UpdateData" == block_name_string
											|| "InventoryData" == block_name_string
										)
									)
							)
						)
					{
						std::stringstream temp_stream;
						temp_stream << selected_objectp->getLocalID();
						value = temp_stream.str();
					}
					else if( agent_parcelp &&
						"LocalID" == var_name_string &&
						(
							"ParcelData" == block_name_string
							|| message.find("Parcel") != message.npos
						)
					)
					{
						std::stringstream temp_stream;
						temp_stream << agent_parcelp->getLocalID();
						value = temp_stream.str();
					}
					else if("PCode" == var_name_string)
						value = "9";
					else if("PathCurve" == var_name_string)
						value = "16";
					else if("ProfileCurve" == var_name_string)
						value = "1";
					else if("PathScaleX" == var_name_string || "PathScaleY" == var_name_string)
						value = "100";
					else if("BypassRaycast" == var_name_string)
						value = "1";
					else
						value = "0";
					break;
				case MVT_F32:
				case MVT_F64:
					value = "0.0";
					break;
				case MVT_LLVector3:
				case MVT_LLVector3d:
				case MVT_LLQuaternion:
					if("Position" == var_name_string || "RayStart" == var_name_string || "RayEnd" == var_name_string)
						value = "$Position";
					else if("Scale" == var_name_string)
						value = "<0.5, 0.5, 0.5>";
					else
						value = "<0, 0, 0>";
					break;
				case MVT_LLVector4:
					value = "<0, 0, 0, 0>";
					break;
				case MVT_LLUUID:
					if("AgentID" == var_name_string)
						value = "$AgentID";
					else if("SessionID" == var_name_string)
						value = "$SessionID";
					else if("ObjectID" == var_name_string && selected_objectp)
						value = selected_objectp->getID().asString();
					else if("ParcelID" == var_name_string && agent_parcelp)
						value = agent_parcelp->getID().asString();
					else
						value = "00000000-0000-0000-0000-000000000000";
					break;
				case MVT_BOOL:
					value = "false";
					break;
				case MVT_VARIABLE:
					value = "Hello, world!";
					break;
				case MVT_FIXED:
					for(S32 si = 0; si < size; si++)
						//value.append(std::string("0123456789abcdef").substr(si & 0xf, 1));
						value.append("a");
					break;
				default:
					value = "";
					break;
				}
				text.append(llformat("%s\n", value.c_str()));
			}
		}
	}
	text = text.substr(0, text.length() - 1);
	floaterp->childSetText("message_edit", text);
}
// static
void LLFloaterMessageBuilder::onClickSend(void* user_data)
{
	LLFloaterMessageBuilder* floaterp = (LLFloaterMessageBuilder*)user_data;
	std::vector<std::string> lines = split(floaterp->childGetText("message_edit"), "\n");
	if(!lines.size())
	{
		LLFloaterChat::addChat(LLChat("Not enough information :O"));
		return;
	}
	std::vector<std::string> tokens = split(lines[0], " ");
	if(!tokens.size())
	{
		LLFloaterChat::addChat(LLChat("Not enough information :O"));
		return;
	}
	std::string dir_str = tokens[0];
	LLStringUtil::toLower(dir_str);
	// Direction
	BOOL outgoing;
	if(dir_str == "out")
		outgoing = TRUE;
	else if(dir_str == "in")
		outgoing = FALSE;
	else
	{
		LLFloaterChat::addChat(LLChat("Expected direction 'in' or 'out'"));
		return;
	}
	// Message
	std::string message = "Invalid";
	if(tokens.size() > 1)
	{
		if(tokens.size() > 2)
		{
			LLFloaterChat::addChat(LLChat("Unexpected extra stuff at the top"));
			return;
		}
		message = tokens[1];
		LLStringUtil::trim(message);
	}
	// Body
	std::vector<parts_block> parts;
	if(lines.size() > 1)
	{
		std::vector<std::string>::iterator line_end = lines.end();
		std::vector<std::string>::iterator line_iter = lines.begin();
		++line_iter;
		std::string current_block("");
		int current_block_index = -1;
		for( ; line_iter != line_end; ++line_iter)
		{
			std::string line = (*line_iter);
			LLStringUtil::trim(line);
			if(!line.length())
				continue;
			if(line.substr(0, 1) == "[" && line.substr(line.size() - 1, 1) == "]")
			{
				current_block = line.substr(1, line.length() - 2);
				LLStringUtil::trim(current_block);
				++current_block_index;
				parts_block pb;
				pb.name = current_block;
				parts.push_back(pb);
			}
			else
			{
				if(current_block.empty())
				{
					LLFloaterChat::addChat(LLChat("Unexpected field when no block yet"));
					return;
				}
				int eqpos = line.find("=");
				if(eqpos == line.npos)
				{
					LLFloaterChat::addChat(LLChat("Missing an equal sign"));
					return;
				}
				std::string field = line.substr(0, eqpos);
				LLStringUtil::trim(field);
				if(!field.length())
				{
					LLFloaterChat::addChat(LLChat("Missing name of field"));
					return;
				}
				std::string value = line.substr(eqpos + 1);
				LLStringUtil::trim(value);
				parts_var pv;
				if(value.substr(0, 1) == "|")
				{
					pv.hex = TRUE;
					value = value.substr(1);
					LLStringUtil::trim(value);
				}
				else
					pv.hex = FALSE;
				pv.name = field;
				pv.value = value;
				parts[current_block_index].vars.push_back(pv);
			}
		}
	}
	// Verification
	std::map<const char *, LLMessageTemplate*>::iterator template_iter;
	template_iter = gMessageSystem->mMessageTemplates.find( LLMessageStringTable::getInstance()->getString(message.c_str()) );
	if(template_iter == gMessageSystem->mMessageTemplates.end())
	{
		LLFloaterChat::addChat(LLChat(llformat("Don't know how to build a '%s' message", message.c_str())));
		return;
	}
	LLMessageTemplate* temp = (*template_iter).second;
	std::vector<parts_block>::iterator parts_end = parts.end();
	std::vector<parts_block>::iterator parts_iter = parts.begin();
	LLMessageTemplate::message_block_map_t::iterator blocks_end = temp->mMemberBlocks.end();
	for (LLMessageTemplate::message_block_map_t::iterator blocks_iter = temp->mMemberBlocks.begin();
		 blocks_iter != blocks_end; )
	{
		LLMessageBlock* block = (*blocks_iter);
		const char* block_name = block->mName;
		if(parts_iter == parts_end)
		{
			if(block->mType != MBT_VARIABLE)
				LLFloaterChat::addChat(LLChat(llformat("Expected '%s' block", block_name)));
			else
			{
				++blocks_iter;
				continue;
			}
			return;
		}
		else if((*parts_iter).name != block_name)
		{
			if(block->mType != MBT_VARIABLE)
				LLFloaterChat::addChat(LLChat(llformat("Expected '%s' block", block_name)));
			else
			{
				++blocks_iter;
				continue;
			}
			return;
		}
		std::vector<parts_var>::iterator part_var_end = (*parts_iter).vars.end();
		std::vector<parts_var>::iterator part_var_iter = (*parts_iter).vars.begin();
		LLMessageBlock::message_variable_map_t::iterator var_end = block->mMemberVariables.end();
		for (LLMessageBlock::message_variable_map_t::iterator var_iter = block->mMemberVariables.begin();
			 var_iter != var_end; ++var_iter)
		{
			LLMessageVariable* variable = (*var_iter);
			const char* var_name = variable->getName();
			if(part_var_iter == part_var_end)
			{
				LLFloaterChat::addChat(LLChat(llformat("Expected '%s' field under '%s' block", var_name, block_name)));
				return;
			}
			else if((*part_var_iter).name != var_name)
			{
				LLFloaterChat::addChat(LLChat(llformat("Expected '%s' field under '%s' block", var_name, block_name)));
				return;
			}
			(*part_var_iter).var_type = variable->getType();
			++part_var_iter;
		}
		if(part_var_iter != part_var_end)
		{
			LLFloaterChat::addChat(LLChat(llformat("Unexpected field(s) at end of '%s' block", block_name)));
			return;
		}
		++parts_iter;
		// test
		if((block->mType != MBT_SINGLE) && (parts_iter != parts_end) && ((*parts_iter).name == block_name))
		{
			// block will repeat
		}
		else ++blocks_iter;
	}
	if(parts_iter != parts_end)
	{
		LLFloaterChat::addChat(LLChat("Unexpected block(s) at end"));
		return;
	}
	// Build and send
	gMessageSystem->newMessage( message.c_str() );
	for(parts_iter = parts.begin(); parts_iter != parts_end; ++parts_iter)
	{
		const char* block_name = (*parts_iter).name.c_str();
		gMessageSystem->nextBlock(block_name);
		std::vector<parts_var>::iterator part_var_end = (*parts_iter).vars.end();
		for(std::vector<parts_var>::iterator part_var_iter = (*parts_iter).vars.begin();
			part_var_iter != part_var_end; ++part_var_iter)
		{
			parts_var pv = (*part_var_iter);
			if(!addField(pv.var_type, pv.name.c_str(), pv.value, pv.hex))
			{
				LLFloaterChat::addChat(LLChat(llformat("Error adding the provided data for %s '%s' to '%s' block", mvtstr(pv.var_type).c_str(), pv.name.c_str(), block_name)));
				gMessageSystem->clearMessage();
				return;
			}
		}
	}

	LLScrollListCtrl* scrollp = floaterp->getChild<LLScrollListCtrl>("net_list");
	LLScrollListItem* selected_itemp = scrollp->getFirstSelected();

	//if a specific circuit is selected, send it to that, otherwise send it to the current sim
	if(selected_itemp)
	{
		LLNetListItem* itemp = findNetListItem(selected_itemp->getUUID());
		LLScrollListText* textColumn = (LLScrollListText*)selected_itemp->getColumn(1);

		//why would you send data through a dead circuit?
		if(textColumn->getValue().asString() == "Dead")
		{
			LLFloaterChat::addChat(LLChat("No sending messages through dead circuits!"));
			return;
		}
		if(outgoing)
		{
			gMessageSystem->sendMessage(itemp->mCircuitData->getHost());
		} else {
			U8 builtMessageBuffer[MAX_BUFFER_SIZE];

			S32 message_size = gMessageSystem->mTemplateMessageBuilder->buildMessage(builtMessageBuffer, MAX_BUFFER_SIZE, 0);
			gMessageSystem->clearMessage();
			gMessageSystem->checkMessages(0, true, builtMessageBuffer, itemp->mCircuitData->getHost(), message_size);

		}
	} else {
		if(outgoing)
		{
			gMessageSystem->sendMessage(gAgent.getRegionHost());
		} else {
			U8 builtMessageBuffer[MAX_BUFFER_SIZE];

			S32 message_size = gMessageSystem->mTemplateMessageBuilder->buildMessage(builtMessageBuffer, MAX_BUFFER_SIZE, 0);
			gMessageSystem->clearMessage();
			gMessageSystem->checkMessages(0, true, builtMessageBuffer, gAgent.getRegionHost(), message_size);

		}
	}
}

BOOL LLFloaterMessageBuilder::handleKeyHere(KEY key, MASK mask)
{
	if(key == KEY_RETURN && (mask & MASK_CONTROL))
	{
		onClickSend(this);
		return TRUE;
	}
	if(key == KEY_ESCAPE)
	{
		releaseFocus();
		return TRUE;
	}
	return FALSE;
}
// </edit>
