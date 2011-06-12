
#include "llviewerinventory.h"

enum export_states {
	EXPORT_INIT,
	EXPORT_STRUCTURE,
	EXPORT_TEXTURES,
	EXPORT_LLSD,
	EXPORT_DONE,
	EXPORT_FAILED
};

class PrimBackup : public LLFloater
{
public:
	virtual ~PrimBackup();

	// Floater stuff
	virtual void show(bool exporting);
	virtual void draw();
	virtual void onClose(bool app_quitting);

	// Static accessor
	static PrimBackup* getInstance();

	// Export idle callback
	static void exportWorker(void *userdata);

	// Import entry point
	void importObject(bool upload=FALSE);

	// Export entry point
	void exportObject();

	// Update map from texture worker
	void updateMap(LLUUID uploaded_asset);

	// Move to next texture upload
	void uploadNextAsset();

	// Folder public geter
	std::string getfolder() { return mFolder; };

	// Prim updated callback
	void primUpdate(LLViewerObject* object);

	// New prim call back
	bool newPrim(LLViewerObject* pobject);

	static const U32 TEXTURE_OK = 0x00;
	static const U32 TEXTURE_BAD_PERM = 0x01;
	static const U32 TEXTURE_MISSING = 0x02;
	static const U32 TEXTURE_BAD_ENCODING = 0x04;
	static const U32 TEXTURE_IS_NULL = 0x08;
	static const U32 TEXTURE_SAVED_FAILED = 0x10;

	// Is ready for next texture?
	bool mNextTextureReady;

	// Export state machine
	enum export_states mExportState;

	// Export result flags for textures.
	U32 mNonExportedTextures;

	static bool validatePerms(const LLPermissions* item_permissions);

private:
	// Static singleton stuff
	PrimBackup();
	static PrimBackup* sInstance;

	// Update the floater with status numbers
	void updateImportNumbers();
	void updateExportNumbers();

	// Permissions stuff.
	LLUUID validateTextureID(LLUUID asset_id);

	// Convert a selection list of objects to LLSD
	LLSD primsToLLSD(LLViewerObject::child_list_t child_list, bool is_attachment);

	// Start the import process
	void importFirstObject();

	// Move to the next import group
	void importNextObject();

	// Export the next texture in list
	void exportNextTexture();

	// Apply LLSD to object
	void xmlToPrim(LLSD prim_llsd, LLViewerObject* pobject);

	// Rez a prim at a given position (note not agent offset X/Y screen for raycast)
	void rezAgentOffset(LLVector3 offset);

	// Get an offset from the agent based on rotation and current pos
	LLVector3 offsetAgent(LLVector3 offset);

	// Are we active flag
	bool mRunning;

	// File and folder name control
	std::string mFileName;
	std::string mFolder;

	// True if we need to rebase the assets
	bool mRetexture;

	// Counts of import and export objects and prims
	U32 mObjects;
	U32 mCurObject;
	U32 mPrims;
	U32 mCurPrim;

	// No prims rezed
	U32 mRezCount;

	// Rebase map
	std::map<LLUUID,LLUUID> mAssetMap;

	// Export texture list
	std::list<LLUUID> mTexturesList;

	// Import object tracking
	std::vector<LLViewerObject*> mToSelect;
	std::vector<LLViewerObject*>::iterator mProcessIter;

	// Working LLSD holders
	LLUUID mCurrentAsset;
	LLSD mLLSD;
	LLSD mThisGroup;
	LLUUID mExpectingUpdate;

	// Working llsd itterators for objects and linksets
	LLSD::map_const_iterator mPrimImportIter;
	LLSD::array_const_iterator mGroupPrimImportIter;

	// Root pos and rotation and central root pos for link set
	LLVector3 mRootPos;
	LLQuaternion mRootRot;
	LLVector3 mRootRootPos;
	LLVector3 mGroupOffset;

	// Agent inital pos and rot when starting import
	LLVector3 mAgentPos;
	LLQuaternion mAgentRot;
};
