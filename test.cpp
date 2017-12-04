#include <iostream>
#include <thread>
#include <vector>
#include "dataacqctl.h"

using std::endl;

int main()
{
	DataAcqCtl CDAQU;
	uint32_t curFrameId = 0,new_FrameId=0;

	CDAQU.openDevice();
	CDAQU.configFpgaRegAddrStruct();
	CDAQU.grabData();
	CDAQU.waitFrameValid(50000);
	CDAQU.switchBank();
// ---------------------------------------------------
	curFrameId = CDAQU.getFrameNumber();
	CDAQU.switchBank();
	new_FrameId = CDAQU.getFrameNumber();
// ---------------------------------------------------
	CDAQU.waitFrameValid (500);


	vector<uint32_t> fids(1000);


//	for (uint i=0; i<fids.size(); i++)
//	{
	//	curFrameId = ctl.waitFrameValid(curFrameId, 1000);
	//	fids[i] = curFrameId;
	//	ctl.grabData();
//	}

	cout << endl;
	CDAQU.closeDevice();
	return 0;
}




