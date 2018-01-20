struct Toaster_record
{
	int cmd;
	int previewSource, transitionBank, transitionSpeed;
	int transitionCol, transitionRow, transitionOwn;

	int from;
	TEXT saveFrameStore[SIZE_FULLPATH];
	int FS_Number;

	TEXT loadFrameStore[SIZE_FULLPATH];
	int Into_Number;

	TEXT frameStorePath[SIZE_FULLPATH];
};

/******** E O F ********/
