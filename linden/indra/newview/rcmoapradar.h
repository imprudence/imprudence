

class LLFloaterMOAPRadar : public LLFloater
{

private:
	LLFloaterMOAPRadar();
public:
	~LLFloaterMOAPRadar();

	enum AVATARS_COLUMN_ORDER
	{
		LIST_URL,
		LIST_FACE,
		LIST_DISTANCE,
		LIST_POSITION,
		LIST_ALTITUDE
	};

	/*virtual*/ void onClose(bool app_quitting);
	/*virtual*/ void onOpen();
	/*virtual*/ BOOL postBuild();
	/*virtual*/ void draw();

	static void toggle(void*);

	static void showInstance();

	static void callbackIdle(void *userdata);

	void updateMOAPList();

	static void onClickOpen(void* userdata);
	static void onClickCopy(void* userdata);
	static void onClickTrack(void* userdata);
	static void onSelectMOAP(LLUICtrl*, void* userdata);


private:
	static LLFloaterMOAPRadar* sInstance;
	LLScrollListCtrl*			mMOAPList;
	LLButton *				mTrackBtn;

	U32 mUpdateRate;

	LLUUID mSelectedObjID;
	U8 mSelectedFace;
	bool mTrackingRunning;

	LLUUID mTrackedID;
	U8 mTrackedFace;

	void updatetrackbtn(); 

public:
	static LLFloaterMOAPRadar* getInstance() { return sInstance; }

};