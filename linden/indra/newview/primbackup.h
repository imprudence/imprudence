
#include "llviewerinventory.h"

#define LL_GRID_PERMISSIONS 1

//enum export_states {EXPORT_INIT,EXPORT_STRUCTURE,EXPORT_TEXTURES,EXPORT_LLSD,EXPORT_DONE};

class primbackup : 	public LLFloater

{
	public:
	//Export state machine
	enum export_states export_state; 
	
	//Export idle callback
	//static void exportworker(void *userdata);

	//Static accessor
	static primbackup* getInstance();

  static bool check_perms( LLSelectNode* node );

	virtual ~primbackup();
	
	//Floater stuff
	virtual void show();
	virtual void draw();
	virtual void onClose( bool app_quitting );

	//Import entry point
	void import_object(bool upload=FALSE);
	
	//Export entry point
	void pre_export_object();

	//Update map from texture worker
	void update_map(LLUUID uploaded_asset);

	//Move to next texture upload
	void upload_next_asset();

	// is ready for next texture?
	bool m_nexttextureready;

	//Folder public geter
	std::string getfolder() {return folder;};

	//Prim updated callback
	void prim_update(LLViewerObject* object);

	//New prim call back
	bool newprim(LLViewerObject * pobject);

private:

	//Static singleton stuff
	primbackup();	
	static primbackup* sInstance;

	// are we active flag
	bool running;

	//file and folder name control 
	std::string file_name;
	std::string folder;

	// True if we need to rebase the assets
	bool m_retexture;

	//Counts of import and export objects and textures and prims
	int m_objects,m_curobject;
	int m_prims,m_curprim;
	int m_textures,m_curtexture;

	// No prims rezed
	int rezcount;

	// Update the floater with status numbers
	void updateimportnumbers();
	void updateexportnumbers();

	//Convert a selection list of objects to HPA
	LLXMLNode *prims_to_xml(LLViewerObject::child_list_t child_list);
	//Convert a selection list of objects to LLSD
	LLSD prims_to_llsd(LLViewerObject::child_list_t child_list);
		
	// Start the import process
	void import_object1a();

	//Export the next texture in list
	void export_next_texture();

	//apply LLSD to object
	void xmltoprim(LLSD prim_llsd,LLViewerObject * pobject);
	

	//rez a prim at a given position (note not agent offset X/Y screen for raycast)
	void rez_agent_offset(LLVector3 offset);	
	
	//Move to the next import group
	void import_next_object();
	
	//Get an offset from the agent based on rotation and current pos
	LLVector3 offset_agent(LLVector3 offset);

	// Rebase map
	std::map<LLUUID,LLUUID> assetmap;
	
	//Export texture list
	std::list<LLUUID> textures;

	//Import object tracking
	std::vector<LLViewerObject *> toselect;
	std::vector<LLViewerObject *>::iterator process_iter;

	//Working LLSD holders
	LLUUID current_asset;
	LLSD llsd;
	LLSD this_group;
	LLUUID expecting_update;

	LLXMLNodePtr xml;

	//working llsd itterators for objects and linksets
	LLSD::map_const_iterator prim_import_iter;
	LLSD::array_const_iterator group_prim_import_iter;

	// Root pos and central root pos for link set
	LLVector3 root_pos;
	LLVector3 root_root_pos;
	LLVector3 group_offset;

	//Agent inital pos and rot when starting import
	LLQuaternion root_rot;
	LLQuaternion agent_rot;

};
