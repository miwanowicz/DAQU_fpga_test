/*
 * dataacqctl.cpp
 *
  */
#include "dataacqctl.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <thread>
#include <unistd.h>

#define MEMPATH "/dev/mem"

using namespace std;

//std::unique_ptr<DataAcqCtl> DataAcqCtl::instance;
//std::once_flag DataAcqCtl::onceFlag;

DataAcqCtl::~DataAcqCtl()  {
	closeDevice();
}

void DataAcqCtl::openDevice() {
	if((fd = open(MEMPATH, O_RDWR | O_SYNC )) == -1)
		throw runtime_error("could not access device memory!");
	state = State::Open;
}

void DataAcqCtl::init() {

	if(state != State::Open)
		throw runtime_error("device not open or has already been initialized!");

	if (!memoryIsOk())
		throw runtime_error("Error mapping data memory");
	acqStartTime = chrono::high_resolution_clock::now();
	configFpgaRegAddrStruct();
	state = State::Initialized;
}

bool DataAcqCtl::memoryIsOk()
{
	return 1;
}

void DataAcqCtl::closeDevice() {
	close(fd);
	state = State::Closed;
}
/***/
uint32_t DataAcqCtl::getFrameNumber()	{
	  void *map_base,*virt_addr1,*virt_addr2;
	  unsigned long read_result1, read_result2;
	  off_t target = AdrStartBuffor;
//---------------------------------------------------------------------------//
/* Map one page */
	  map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
	  if(map_base == (void *) -1)
	     printf("Houston, We've Got a Problem");
// --------------------------------------------------------------------------//
	  virt_addr1   = map_base + (target & MAP_MASK);
	  virt_addr2   = map_base + (target & MAP_MASK) + 2;
	  read_result1 = *((unsigned short *) virt_addr1);
	  read_result2 = *((unsigned short *) virt_addr2);
//--- printf(" Frame number at address 0x%X (%p): 0x%X / 0x%X\n ", target, virt_addr1, read_result1 , read_result2);
	  frame_no  = read_result1 ;
      frame_no *= 0xFFFF;
      frame_no += read_result2;

      if (munmap(map_base, MAP_SIZE) == -1)
           printf("mmap close - fatal error.\n");

      return frame_no;
}
/***/
void DataAcqCtl::grabData()		{
	 void *map_base,*virt_addr1,*virt_addr2;
	 int  C=0, A=1000;
	 unsigned long read_result1, read_result2,temp1, temp2;
	 off_t target = AdrStartBuffor;
     /* Map one page */
	 map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
	 if(map_base == (void *) -1)
		 printf("Houston, We've Got a Problem");
// -------------------------------------------------------------------------- //
     virt_addr1 	= map_base + (target & MAP_MASK);
	 virt_addr2 	= map_base + (target & MAP_MASK) + 2;
	 read_result1 	= *((unsigned short *) virt_addr1);
	 read_result2 	= *((unsigned short *) virt_addr2);
//	 printf("Frame number at address 0x%X (%p): 0x%X / 0x%X\n ", target, virt_addr1, read_result1 , read_result2);
	 frame_no  = read_result1;
	 frame_no *= 0xFFFF;
	 frame_no += read_result2;
	 target    = AdrStartDataBuffor;
// ----------------------------------------------------------------------------//
	 for (int i=0; i<1024*4; i=i+4)	 {
	     if (i<((A*4)-8))   		 {
	         wavalengh_No [C] = C;
	         virt_addr1		= map_base + (target & MAP_MASK) + i;
	         virt_addr2 	= map_base + (target & MAP_MASK) + i + 2;
	         temp1 			= *((unsigned short *) virt_addr1);
	         temp2 			= *((unsigned short *) virt_addr2);
             pulse_data[C] 	=  temp1;
	         bg_data   [C] 	=  temp2;
 //          printf("Value at address A 0x%X (%p): 0x%X : 0x%X\n ", target + i, virt_addr1, temp1 , temp2);
	     }
	     else    					 {
	         wavalengh_No [C] = 0;
	         pulse_data   [C] = 0;
	         bg_data      [C] = 0;
	      }
	    C++;
	 }

	 if (munmap(map_base, MAP_SIZE) == -1)
	      printf("mmap close - fatal error.\n");
}
/**
 * Blocks program execution until new frame id is observed. Returns new frame id.
 */
uint32_t DataAcqCtl::waitFrameValid(uint32_t  timeoutInMillis) {
	uint32_t Id,curFrameId,newFrameId;
//	const auto absoluteTimeout = steady_clock::now() + milliseconds(timeoutInMillis);
// ---------------------------------------------------------------------------------------
	for( Id=0; Id<5000; Id++)  {
		newFrameId = getFrameNumber();
		this_thread::sleep_for (chrono::microseconds(timeoutInMillis));
		printf("Frame number %d: 0x%X / 0x%X div= %d \n",Id,curFrameId,newFrameId, newFrameId-curFrameId);
	//	grabData();

		switchBank();
		curFrameId=newFrameId;
	}
	return newFrameId;
}
/***/
void DataAcqCtl::switchBank()									{
    void *ymap_base, *yvirt_addr;
    off_t ytarget = AddrBankChange;
    unsigned long read_result,temp;
    unsigned short data;
//--- Map one page  --------------------------------
    ymap_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, ytarget & ~MAP_MASK);
    if(ymap_base == (void *) -1)
    	printf("Houston, We've Got a Problem");

    yvirt_addr = ymap_base + (ytarget & MAP_MASK);
    read_result  = *((unsigned short *) yvirt_addr);
    temp = read_result & 0x01;
//--------------------------------------------------//
     if (temp == 1)  	{
         data = read_result & 0xFFF0;
        *((unsigned short *) yvirt_addr)  = data;   // init default value
     }
     else		    	{
        data =  read_result | 0x0001;
        *((unsigned short *) yvirt_addr)  = data;   // init default value

     }
     if(munmap(ymap_base, MAP_SIZE) == -1)
          printf("mmap close - fatal error.\n");
}
/***/
void DataAcqCtl::configFpgaRegAddrStruct()	{
	   	 off_t xtarget=ADR_SETUP_BASE, target;
	   	 int Val;
	   	 void *xmap_base,*xvirt_addr;
//-------------------------------------------------------------
	   	 xmap_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, xtarget & ~MAP_MASK);
	   	 if(xmap_base == (void *) -1)
	   		 printf("Houston, We've Got a Problem");
// init default value:
	   	 xvirt_addr = xmap_base + (xtarget & MAP_MASK);
	   	 Val = *((unsigned short *) xvirt_addr);  // init default value
// Pulse Delay: ------------------------------------------------
	   	 target = AddrPulseDelay; // Addr Start Pulse Average
	   	 xvirt_addr = xmap_base + (target & MAP_MASK) ;
	     fpgaCtlRegisters.pulse_delay = *((unsigned short *) xvirt_addr);
	     printf("ValPulseDelay - %d.\n", fpgaCtlRegisters.pulse_delay);
// Start Pulse Average: ------------------------------------------------
	     target = AddrStartPulseAverage; // Addr Start Pulse Average
	     xvirt_addr = xmap_base + (target & MAP_MASK) ;
	     fpgaCtlRegisters.start_pulse_avg = *((unsigned short *) xvirt_addr);
	     printf("StartPulseAverage - %d.\n", fpgaCtlRegisters.start_pulse_avg);
// Stop Pulse Average: --------------------------------------------------
	     target = AddrStopPulseAverage; // Addr Stop Pulse Average
	     xvirt_addr = xmap_base + (target & MAP_MASK) ;
	     fpgaCtlRegisters.stop_pulse_avg =  *((unsigned short *) xvirt_addr);
	     printf("StopPulseAveragee - %d.\n",  fpgaCtlRegisters.stop_pulse_avg);
// Start Background Average: ----------------------------------------------
	     target = AddrStartBackAverage ; // Addr Start Back Average
	     xvirt_addr = xmap_base + (target & MAP_MASK) ;
	     fpgaCtlRegisters.start_bg_avg  = *((unsigned short *) xvirt_addr);
	     printf("StartBackAverage - %d.\n", fpgaCtlRegisters.start_bg_avg);
// Stop Backbround Average: ----------------------------------------------
	     target = AddrStopBackAverage; // Addr ValStopBackeAverage
	     xvirt_addr = xmap_base + (target & MAP_MASK);
	     fpgaCtlRegisters.stop_bg_avg  = *((unsigned short *) xvirt_addr);
	     printf("ValStopPulseAverage - %d.\n",fpgaCtlRegisters.stop_bg_avg);
// End Cnv Count: ------------------------------------------------------
	     target = AddrEndCnvCount; // Addr ValEndCnvCount
	     xvirt_addr = xmap_base + (target & MAP_MASK);
	     fpgaCtlRegisters.end_conv_cnt = *((unsigned short *) xvirt_addr);
	     printf("ValEndCnvCount - %d.\n", fpgaCtlRegisters.end_conv_cnt);
// NoLambda ------------------------------------------------------------
	     target = AddrNoLambda; // Addr ValEndCnvCount
	     xvirt_addr = xmap_base + (target & MAP_MASK) ;
	     fpgaCtlRegisters.lmbd_max_cnt = *((unsigned short *) xvirt_addr);
	     fpgaCtlRegisters.lmbd_max_cnt -= 2;
	     printf("NoLambda - %d.\n",fpgaCtlRegisters.lmbd_max_cnt);
// AverageNo ------------------------------------------------------------
	     target = AddrAverageNo; // Addr ValEndCnvCount
	     xvirt_addr = xmap_base + (target & MAP_MASK) ;
	     fpgaCtlRegisters.avg_cnt  = *((unsigned short *) xvirt_addr);
	     printf("NoLambda - %d.\n", fpgaCtlRegisters.avg_cnt );
//---------------------------------------------------------------------
	     if(munmap(xmap_base, MAP_SIZE) == -1)
	        printf("/dev/mem - mem map close - fatal error.\n");
}
/***/
