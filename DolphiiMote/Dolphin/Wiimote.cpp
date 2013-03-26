
#include "Common.h"
#include <mutex>

#include "Wiimote.h"
#include "WiimoteReal.h"

namespace Wiimote
{

void Shutdown()
{

}

// if plugin isn't initialized, init and load config
void Initialize(void* const hwnd)
{
	WiimoteReal::Initialize();
}

// __________________________________________________________________________________________________
// Function: Wiimote_Output
// Purpose:  An L2CAP packet is passed from the Core to the Wiimote,
//           on the HID CONTROL channel.
// input:    Da pakket.
// output:   none
//
void ControlChannel(int _number, u16 _channelID, const void* _pData, u32 _Size)
{
	if (WIIMOTE_SRC_REAL & g_wiimote_sources[_number])
		WiimoteReal::ControlChannel(_number, _channelID, _pData, _Size);
}

// __________________________________________________________________________________________________
// Function: Wiimote_InterruptChannel
// Purpose:  An L2CAP packet is passed from the Core to the Wiimote,
//           on the HID INTERRUPT channel.
// input:    Da pakket.
// output:   none
//
void InterruptChannel(int _number, u16 _channelID, const void* _pData, u32 _Size)
{
		WiimoteReal::InterruptChannel(_number, _channelID, _pData, _Size);
}

// __________________________________________________________________________________________________
// Function: Wiimote_Update
// Purpose:  This function is called periodically by the Core.
// input:    none
// output:   none
//
void Update(int _number)
{
	//PanicAlert( "Wiimote_Update" );

	// TODO: change this to a try_to_lock, and make it give empty input on failure
		WiimoteReal::Update(_number);
}

// __________________________________________________________________________________________________
// Function: PAD_GetAttachedPads
// Purpose:  Get mask of attached pads (eg: controller 1 & 4 -> 0x9)
// input:	 none
// output:   number of pads
//
unsigned int GetAttached()
{
	unsigned int attached = 0;
	for (unsigned int i=0; i<4; ++i)
		if (g_wiimote_sources[i])
			attached |= (1 << i);
	return attached;
}

// ___________________________________________________________________________
// Function: DoState
// Purpose:  Saves/load state
// input/output: ptr
// input: mode
//
void DoState(unsigned char **ptr, int mode)
{
	// TODO:
}

// ___________________________________________________________________________
// Function: EmuStateChange
// Purpose: Notifies the plugin of a change in emulation state
// input:    newState
// output:   none
//
void EmuStateChange(EMUSTATE_CHANGE newState)
{
	// TODO
	WiimoteReal::StateChange(newState);
}

}
