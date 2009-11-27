/** 
 * @file importtracker.h
 * @brief A utility for importing linksets from XML.
 * Discrete wuz here
 */

#ifndef IMPORTTRACKER_H
#define IMPORTTRACKER_H

#include "llviewerobject.h"

class LLSpinCtrl;

class ImportTrackerFloater : public LLFloater
{
public:
	void draw();
	static ImportTrackerFloater* getInstance();
	virtual ~ImportTrackerFloater();
	//close me
	static void close();
	void show();
	ImportTrackerFloater();	
	static ImportTrackerFloater* sInstance;

	BOOL handleMouseDown(S32 x,S32 y,MASK mask);
	BOOL handleMouseUp(S32 x,S32 y,MASK mask);
	BOOL handleHover(S32 x,S32 y,MASK mask);

	static void 	onCommitPosition(LLUICtrl* ctrl, void* userdata);

	//Reset button
	static void onClickReset(void* data);

	//Import button
	static void onClickImport(void* data);
	
	//Close button
	static void onClickClose(void* data);

	LLSpinCtrl*		mCtrlPosX;
	LLSpinCtrl*		mCtrlPosY;
	LLSpinCtrl*		mCtrlPosZ;

	static int		total_objects;
	static int		objects_imported;
	static int		total_linksets;
	static int		linksets_imported;
	static int		textures_imported;
	static int		total_assets;
	static int		assets_imported;
	static int		assets_uploaded;

protected:
	void			sendPosition();
};

class ImportTracker
{
	public:
		enum ImportState { IDLE, WAND, BUILDING, LINKING, POSITIONING };			
		
		ImportTracker()
		: numberExpected(0),
		state(IDLE),
		last(0),
		groupcounter(0),
		updated(0)
		{ }
		ImportTracker(LLSD &data) { state = IDLE; linkset = data; numberExpected=0;}
		~ImportTracker() { localids.clear(); linkset.clear(); }
	
		//Chalice - support import of linkset groups
		void loadhpa(std::string file);
		void importer(std::string file, void (*callback)(LLViewerObject*));
		void cleargroups();
		void import(LLSD &ls_data);
		void expectRez();
		void clear();
		void finish();
		void cleanUp();
		void get_update(S32 newid, BOOL justCreated = false, BOOL createSelected = false);
		
		const int getState() { return state; }

		U32 total_linksets;
		U32 objects;
		U32 textures;
		LLSD linksets;
		U32 asset_insertions;
		LLVector3 importposition;
		LLVector3 size;
		LLVector3 importoffset;
		LLVector3 currentimportoffset;

		//Working LLSD holders
		LLUUID current_asset;
		
		// Rebase map
		std::map<LLUUID,LLUUID> assetmap;
		
		//Export texture list
		std::list<LLUUID> uploadtextures;

		//Update map from texture worker
		void update_map(LLUUID uploaded_asset);

		//Move to next texture upload
		void upload_next_asset();
		
	protected:		
		void send_inventory(LLSD &prim);
		void send_properties(LLSD &prim, int counter);
		void send_vectors(LLSD &prim, int counter);
		void send_shape(LLSD &prim);
		void send_image(LLSD &prim);
		void send_extras(LLSD &prim);
		void send_namedesc(LLSD &prim);
		void link();
		void wear(LLSD &prim);
		void position(LLSD &prim);
		void plywood_above_head();
	
	private:
		int				numberExpected;
	public:
		int				state;
	private:	
		S32				last;
		LLVector3			root;
		LLQuaternion		rootrot;
		std::list<S32>			localids;
	public:
		LLSD				linksetgroups;
		int					groupcounter;
	private:	
		int					updated;
		LLVector3			linksetoffset;
		LLVector3			initialPos;
		LLSD				linkset;

		std::string filepath;
		std::string asset_dir;
		void	(*mDownCallback)(LLViewerObject*);

		U32 lastrootid;
};

extern ImportTracker gImportTracker;

//extern LLAgent gAgent;

#endif
