/*
 * CDataAcqCtl.h
 *
 *  Created on: 08.08.2017
 *      Author: jan
 */

#ifndef DATAACQCTL_H_
#define DATAACQCTL_H_

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE-1)

// Base data segment
#define ADR_START_BASE     					0x08200000
#define ADR_START_BASE_FRAME_NR_LSB_OFFSET	0x00000002
#define ADR_START_BASE_FRAME_NR_MSB_OFFSET	0x00000000
#define ADR_START_BASE_AVG_NO_OFFSET		0x00000004
#define ADR_START_BASE_PULSE_AVG_VAL_OFFSET	0x00000018
#define ADR_START_BASE_BG_AVG_VAL_OFFSET   	0x0000001A

//Scope data segment
#define ADR_SCOPE_BFR        				0x08400000

// Setup data semgent
#define ADR_SETUP_BASE 						0x08600000
#define ADR_SETUP_BANK_CHANGE_OFFSET   	 	0x00000022
#define ADR_SETUP_PULSE_DELAY_OFFSET       	0x00000062
#define ADR_SETUP_START_PULSE_AVRG_OFFSET 	0x00000082
#define ADR_SETUP_STOP_PULSE_AVRG_OFFSET 	0x000000A2
#define ADR_SETUP_START_BACK_AVRG_OFFSET  	0x000000C2
#define ADR_SETUP_STOP_BACK_AVRG_OFFSET   	0x000000E2
#define ADR_SETUP_END_CNV_CNT_OFFSET       	0x00000102
#define ADR_SETUP_NO_LAMBDA_OFFSET          0x00000122
#define ADR_SETUP_AVRG_NO_OFFSET	        0x00000142

// - ADR SETUP: ---------------------------------------------//
#define AdrStartBuffor        				0x8200000
#define AdrStartDataBuffor    				0x8200018
#define AdrScopeBuffor        				0x8400000
#define AddrSetup             				0x8600000
#define AddrBankChange        				0x8600022
#define AddrPulseDelay        				0x8600062
#define AddrStartPulseAverage 				0x8600082
#define AddrStopPulseAverage 				0x86000A2
#define AddrStartBackAverage  				0x86000C2
#define AddrStopBackAverage   				0x86000E2
#define AddrEndCnvCount       				0x8600102
#define AddrNoLambda          				0x8600122
#define AddrAverageNo         				0x8600142
// - MAX/MIN:  ----------------------------------------------//
#define MAX_PulseDelay          100
#define MIN_PulseDelay          1

#define MAX_StartPulseAverage   100
#define MIN_StartPulseAverage   1

#define MAX_StopPulseAverage    100
#define STEP_StopPulseAverage   1
#define MIN_StopPulseAverage    1

#define MAX_StartBackAverage    100
#define STEP_StartBackAverage   1
#define MIN_StartBackAverage    1

#define MAX_StopBackAverage     100
#define STEP_StopBackAverage    1
#define MIN_StopBackAverage     1

#define MAX_EndConverCount      100
#define STEP_EndConverCount     1
#define MIN_EndConverCount      1

#define MAX_LambdaCount         1000
#define STEP_LambdaCount        1
#define MIN_LambdaCount         1

#define MAX_AverageNo           1000
#define MIN_AverageNo           1

#include "inttypes.h"
#include <memory>
#include <chrono>
#include <mutex>

using namespace std;
using namespace chrono;

/* * */
class DataAcqCtl {

//	private:
//		DataAcqCtl(const DataAcqCtl&) = delete;
//		DataAcqCtl & operator=(const DataAcqCtl&) = delete;
//		static std::unique_ptr<DataAcqCtl> instance;
//		static std::once_flag onceFlag;
	public:
		virtual ~DataAcqCtl();
//		DataAcqCtl():fd(-1),map_base(nullptr), map_scope(nullptr), map_setup(nullptr), state(State::Closed){};
/*		static DataAcqCtl& Instance()
		{
			std::call_once(DataAcqCtl::onceFlag, [](){ instance.reset(new DataAcqCtl); });
			return *(instance.get());
		}	*/
		void openDevice();
		void closeDevice();
		void init();
		void switchBank();

		uint32_t getFrameNumber();
		uint32_t waitFrameValid(uint32_t  timeoutInMillis);
		void configFpgaRegAddrStruct();
		void grabData();

		double wavalengh_No[1024], pulse_data[1024], bg_data[1024];
		double frame_no;

	private:
		struct StructFpgaCtlParameterSet
		{  //Local COPY FPGA - register:s
			unsigned short   bank_control;

			unsigned short   pulse_delay;
			unsigned short   start_pulse_avg;
			unsigned short   stop_pulse_avg;
			unsigned short   start_bg_avg;
			unsigned short   stop_bg_avg;
			unsigned short   end_conv_cnt;
			unsigned short   lmbd_max_cnt;
			unsigned short 	 avg_cnt;

			unsigned short   frame_number_lsb;
			unsigned short   frame_number_msb;

			unsigned short   pulse_avg_value;
			unsigned short   bg_avg_value;
			unsigned short   rd_average_number;
		};

	private:
		bool memoryIsOk();
		enum class State {Open, Initialized, Closed};
		int fd;
		high_resolution_clock::time_point acqStartTime;
		State state;
		struct StructFpgaCtlParameterSet fpgaCtlRegisters;
};

#endif /* DATAACQCTL_H_ */
