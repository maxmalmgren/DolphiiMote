// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#include <queue>
#include <algorithm>
#include <stdlib.h>

#include "Common.h"
#include "StringUtil.h"
#include "Timer.h"

#include "WiimoteReal.h"

#include "WiimoteHid.h"

unsigned int	g_wiimote_sources[MAX_WIIMOTES];

namespace WiimoteReal
{

void HandleFoundWiimotes(const std::vector<Wiimote*>&);
void TryToConnectWiimote(Wiimote*);
void HandleWiimoteDisconnect(int index);
void DoneWithWiimote(int index);

bool g_real_wiimotes_initialized = false;

std::recursive_mutex g_refresh_lock;

Wiimote* g_wiimotes[MAX_WIIMOTES];

WiimoteScanner g_wiimote_scanner;

listener_collection<std::weak_ptr<wiimote_listener>> listeners;

u8 FoundWiimotesFlag()
{
  std::lock_guard<std::recursive_mutex> lock(g_refresh_lock);

  int found = 0;
  for(int i = 0, it = 1; i < MAX_WIIMOTES; i++, it *= 2)
    if(g_wiimotes[i])
      found |= it;

  return found;
}

Wiimote::Wiimote()
	: index()
#ifdef __APPLE__
	, btd(), ichan(), cchan(), inputlen(), m_connected()
#elif defined(__linux__) && HAVE_BLUEZ
	, cmd_sock(-1), int_sock(-1)
#elif defined(_WIN32)
	, dev_handle(0), stack(MSBT_STACK_UNKNOWN)
#endif
	, m_last_data_report(Report((u8 *)NULL, 0))
	, m_channel(0), m_run_thread(false)
{
#if defined(__linux__) && HAVE_BLUEZ
	bdaddr = (bdaddr_t){{0, 0, 0, 0, 0, 0}};
#endif
}

Wiimote::~Wiimote()
{
	StopThread();

	if (IsConnected())
		Disconnect();
	
	ClearReadQueue();

	// clear write queue
	Report rpt;
	while (m_write_reports.Pop(rpt))
		delete[] rpt.first;
}

// to be called from CPU thread
void Wiimote::QueueReport(u8 rpt_id, const void* _data, unsigned int size)
{
	auto const data = static_cast<const u8*>(_data);
	
	Report rpt;
	rpt.second = size + 2;
	rpt.first = new u8[rpt.second];
	rpt.first[0] = WM_SET_REPORT | WM_BT_OUTPUT;
	rpt.first[1] = rpt_id;
	std::copy(data, data + size, rpt.first + 2);
	m_write_reports.Push(rpt);
}

void Wiimote::DisableDataReporting()
{
	wm_report_mode rpt = {};
	rpt.mode = WM_REPORT_CORE;
	rpt.all_the_time = 0;
	rpt.continuous = 0;
	rpt.rumble = 0;
	QueueReport(WM_REPORT_MODE, &rpt, sizeof(rpt));
}

void Wiimote::ClearReadQueue()
{
	Report rpt;

	if (m_last_data_report.first)
	{
		delete[] m_last_data_report.first;
		m_last_data_report.first = NULL;
	}

	while (m_read_reports.Pop(rpt))
		delete[] rpt.first;
}

void Wiimote::ControlChannel(const u16 channel, const void* const data, const u32 size)
{
	// Check for custom communication
	if (99 == channel)
		EmuStop();
	else
	{
		InterruptChannel(channel, data, size);
		const hid_packet* const hidp = (hid_packet*)data;
		if (hidp->type == HID_TYPE_SET_REPORT)
		{
			u8 handshake_ok = HID_HANDSHAKE_SUCCESS;
			
      listeners.notify(&wiimote_listener::data_received, index, channel, &handshake_ok, sizeof(handshake_ok));
		}
	}
}

void Wiimote::InterruptChannel(const u16 channel, const void* const _data, const u32 size)
{
	// first interrupt/control channel sent
	if (channel != m_channel)
	{
		m_channel = channel;
		
		ClearReadQueue();

		EmuStart();
	}
	
	auto const data = static_cast<const u8*>(_data);

	Report rpt;
	rpt.first = new u8[size];
	rpt.second = (u8)size;
	std::copy(data, data + size, rpt.first);

	// Convert output DATA packets to SET_REPORT packets.
	// Nintendo Wiimotes work without this translation, but 3rd
	// party ones don't.
 	if (rpt.first[0] == 0xa2)
	{
		rpt.first[0] = WM_SET_REPORT | WM_BT_OUTPUT;
 	}
 	
 	// Disallow games from turning off all of the LEDs.
 	// It makes wiimote connection status confusing.
 	/*if (rpt.first[1] == WM_LEDS)
	{
		auto& leds_rpt = *reinterpret_cast<wm_leds*>(&rpt.first[2]);
		if (0 == leds_rpt.leds)
		{
			// Turn on ALL of the LEDs.
			leds_rpt.leds = 0xf;
		}
	}*/
	/*if (rpt.first[1] == WM_WRITE_SPEAKER_DATA)
	{
		// Translate speaker data reports into rumble reports.
		rpt.first[1] = WM_CMD_RUMBLE;
		// Keep only the rumble bit.
		rpt.first[2] &= 0x1;
		rpt.second = 3;
	}*/

	m_write_reports.Push(rpt);
}

bool Wiimote::Read()
{
	Report rpt;
	
	rpt.first = new unsigned char[MAX_PAYLOAD];
	rpt.second = IORead(rpt.first);

	if (0 == rpt.second)
	{
		WARN_LOG(WIIMOTE, "Wiimote::IORead failed. Disconnecting wiimote %d.", index + 1);
		Disconnect();
	}

	if (rpt.second > 0 && m_channel > 0)
	{
		// Add it to queue
		m_read_reports.Push(rpt);
		return true;
	}

	delete[] rpt.first;
	return false;
}
void Wiimote::WriteImmediately(const u16 channel, const u8* const data, const u32 size) {
	IOWrite(data, size);
}
bool Wiimote::Write()
{
	if (!m_write_reports.Empty())
	{
		Report const& rpt = m_write_reports.Front();
		
		bool const is_speaker_data = rpt.first[1] == WM_WRITE_SPEAKER_DATA;
		if (!is_speaker_data || m_last_audio_report.GetTimeDifference() > 5)
		{
			IOWrite(rpt.first, rpt.second);
			
			if (is_speaker_data)
				m_last_audio_report.Update();
			
			delete[] rpt.first;
			m_write_reports.Pop();
			return true;
		}
	}
	
	return false;
}

// Returns the next report that should be sent
Report Wiimote::ProcessReadQueue()
{
	// Pop through the queued reports
	Report rpt = m_last_data_report;
	while (m_read_reports.Pop(rpt))
	{
		if (rpt.first[1] >= WM_REPORT_CORE)
			// A data report
			m_last_data_report = rpt;
		else
			// Some other kind of report
			return rpt;
	}

	// The queue was empty, or there were only data reports
	return rpt;
}

void Wiimote::Update()
{
	if (!IsConnected())
	{
		HandleWiimoteDisconnect(index);
		return;
	}

	// Pop through the queued reports
	Report const rpt = ProcessReadQueue();

	// Send the report
	if (rpt.first != NULL && m_channel > 0)
    listeners.notify(&wiimote_listener::data_received, index, m_channel, (const void*)rpt.first, (const u32)rpt.second);

	// Delete the data if it isn't also the last data rpt
	if (rpt != m_last_data_report)
		delete[] rpt.first;
}

bool Wiimote::Prepare(int _index)
{
	index = _index;

	// core buttons, no continuous reporting
	u8 const mode_report[] = {WM_SET_REPORT | WM_BT_OUTPUT, WM_CMD_REPORT_TYPE, 0, 0x30};
	
	// Set the active LEDs and turn on rumble.
	u8 const led_report[] = {WM_SET_REPORT | WM_BT_OUTPUT, WM_CMD_LED, u8(WIIMOTE_LED_1 << index | 0x1)};

	// Turn off rumble
	u8 rumble_report[] = {WM_SET_REPORT | WM_BT_OUTPUT, WM_CMD_RUMBLE, 0};
	
	// Request status report
	u8 const req_status_report[] = {WM_SET_REPORT | WM_BT_OUTPUT, WM_REQUEST_STATUS, 0};
	// TODO: check for sane response?

	return (IOWrite(mode_report, sizeof(mode_report))
		&& IOWrite(led_report, sizeof(led_report))
		&& (SLEEP(200), IOWrite(rumble_report, sizeof(rumble_report)))
		&& IOWrite(req_status_report, sizeof(req_status_report)));
}

void Wiimote::EmuStart()
{
	DisableDataReporting();
}

void Wiimote::EmuStop()
{
	m_channel = 0;

	DisableDataReporting();

	NOTICE_LOG(WIIMOTE, "Stopping wiimote data reporting");
}

unsigned int CalculateWantedWiimotes()
{
	// Figure out how many real wiimotes are required
	unsigned int wanted_wiimotes = 0;
	for (unsigned int i = 0; i < MAX_WIIMOTES; ++i)
		if (WIIMOTE_SRC_REAL & g_wiimote_sources[i] && !g_wiimotes[i])
			++wanted_wiimotes;

	return wanted_wiimotes;
}

void WiimoteScanner::WantWiimotes(bool do_want)
{
	m_want_wiimotes = do_want;
}

void WiimoteScanner::StartScanning()
{
	if (!m_run_thread && IsReady())
	{
		m_run_thread = true;
		m_scan_thread = std::thread(std::mem_fun(&WiimoteScanner::ThreadFunc), this);
	}
}

void WiimoteScanner::StopScanning()
{
	m_run_thread = false;
	if (m_scan_thread.joinable())
	{
		m_scan_thread.join();
	}
}

void CheckForDisconnectedWiimotes()
{
	std::lock_guard<std::recursive_mutex> lk(g_refresh_lock);

	for (unsigned int i = 0; i != MAX_WIIMOTES; ++i)
		if (g_wiimotes[i] && !g_wiimotes[i]->IsConnected())
			HandleWiimoteDisconnect(i);
}

void WiimoteScanner::ThreadFunc()
{
	NOTICE_LOG(WIIMOTE, "Wiimote scanning has started");

	while (m_run_thread)
	{
		std::vector<Wiimote*> found_wiimotes;

		//NOTICE_LOG(WIIMOTE, "in loop");

		if (m_want_wiimotes)
			found_wiimotes = FindWiimotes();
		else
		{
			// Does stuff needed to detect disconnects on Windows
			Update();
		}

		//NOTICE_LOG(WIIMOTE, "after update");

		// TODO: this is a fairly lame place for this
		CheckForDisconnectedWiimotes();

		HandleFoundWiimotes(found_wiimotes);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	
	NOTICE_LOG(WIIMOTE, "Wiimote scanning has stopped");
}

void Wiimote::StartThread()
{
	m_run_thread = true;
	m_wiimote_thread = std::thread(std::mem_fun(&Wiimote::ThreadFunc), this);
}

void Wiimote::StopThread()
{
	m_run_thread = false;
	if (m_wiimote_thread.joinable())
		m_wiimote_thread.join();
}

void Wiimote::ThreadFunc()
{
	// main loop
	while (m_run_thread && IsConnected())
	{
#ifdef __APPLE__
		// Reading happens elsewhere on OSX
		bool const did_something = Write();
#else
		bool const did_something = Write() || Read();
#endif
		if (!did_something)
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void LoadSettings()
{
	for (unsigned int i=0; i < MAX_WIIMOTES; ++i)
		g_wiimote_sources[i] = WIIMOTE_SRC_REAL;
}

// config dialog calls this when some settings change
void Initialize()
{
	g_wiimote_scanner.StartScanning();
	
	std::lock_guard<std::recursive_mutex> lk(g_refresh_lock);

	g_wiimote_scanner.WantWiimotes(0 != CalculateWantedWiimotes());
	
	if (g_real_wiimotes_initialized)
		return;

	NOTICE_LOG(WIIMOTE, "WiimoteReal::Initialize");

	g_real_wiimotes_initialized = true;
}

void Shutdown(void)
{	
	g_wiimote_scanner.StopScanning();

	std::lock_guard<std::recursive_mutex> lk(g_refresh_lock);

	if (!g_real_wiimotes_initialized)
		return;

	NOTICE_LOG(WIIMOTE, "WiimoteReal::Shutdown");

	g_real_wiimotes_initialized = false;

	for (unsigned int i = 0; i < MAX_WIIMOTES; ++i)
		HandleWiimoteDisconnect(i);
}

void ChangeWiimoteSource(unsigned int index, int source)
{
	{
	std::lock_guard<std::recursive_mutex> lk(g_refresh_lock);
	
	g_wiimote_sources[index] = source;
	g_wiimote_scanner.WantWiimotes(0 != CalculateWantedWiimotes());
	
	// kill real connection (or swap to different slot)
	DoneWithWiimote(index);
	}

	// reconnect to emu
  listeners.notify(&wiimote_listener::wiimote_connection_changed, index, false);

	if (WIIMOTE_SRC_EMU & source)
  {
		listeners.notify(&wiimote_listener::wiimote_connection_changed, index, true);
  }
}

void TryToConnectWiimote(Wiimote* wm)
{
	std::unique_lock<std::recursive_mutex> lk(g_refresh_lock);

	for (unsigned int i = 0; i != MAX_WIIMOTES; ++i)
	{
		if (WIIMOTE_SRC_REAL & g_wiimote_sources[i]
			&& !g_wiimotes[i])
		{
			if (wm->Connect() && wm->Prepare(i))
			{
				NOTICE_LOG(WIIMOTE, "Connected to wiimote %i.", i + 1);
				
				std::swap(g_wiimotes[i], wm);
				g_wiimotes[i]->StartThread();
				
				listeners.notify(&wiimote_listener::wiimote_connection_changed, i, true);
			}
			break;
		}
	}

	g_wiimote_scanner.WantWiimotes(0 != CalculateWantedWiimotes());
	
	lk.unlock();
	
	delete wm;
}

void DoneWithWiimote(int index)
{
	std::lock_guard<std::recursive_mutex> lk(g_refresh_lock);
	
	if (g_wiimotes[index])
	{
		g_wiimotes[index]->StopThread();
		
		// First see if we can use this real wiimote in another slot.
		for (unsigned int i = 0; i != MAX_WIIMOTES; ++i)
		{
			if (WIIMOTE_SRC_REAL & g_wiimote_sources[i]
				&& !g_wiimotes[i])
			{
				if (g_wiimotes[index]->Prepare(i))
				{
					std::swap(g_wiimotes[i], g_wiimotes[index]);
					g_wiimotes[i]->StartThread();
					
					listeners.notify(&wiimote_listener::wiimote_connection_changed, i, true);
				}
				break;
			}
		}
	}
	
	// else, just disconnect the wiimote
	HandleWiimoteDisconnect(index);
}

void HandleWiimoteDisconnect(int index)
{
	Wiimote* wm = NULL;
	
	{
	std::lock_guard<std::recursive_mutex> lk(g_refresh_lock);
	std::swap(wm, g_wiimotes[index]);
	g_wiimote_scanner.WantWiimotes(0 != CalculateWantedWiimotes());
	}

	if (wm)
	{
		delete wm;
		NOTICE_LOG(WIIMOTE, "Disconnected wiimote %i.", index + 1);
	}
}

void HandleFoundWiimotes(const std::vector<Wiimote*>& wiimotes)
{
	std::for_each(wiimotes.begin(), wiimotes.end(), TryToConnectWiimote);
}

// This is called from the GUI thread
void Refresh()
{
	g_wiimote_scanner.StopScanning();
	
	{
	std::unique_lock<std::recursive_mutex> lk(g_refresh_lock);
	std::vector<Wiimote*> found_wiimotes;
	
	if (0 != CalculateWantedWiimotes())
	{
		// Don't hang dolphin when searching
		lk.unlock();
		found_wiimotes = g_wiimote_scanner.FindWiimotes();
		lk.lock();
	}

	CheckForDisconnectedWiimotes();

	// Brief rumble for already connected wiimotes.
	for (int i = 0; i != MAX_WIIMOTES; ++i)
	{
		if (g_wiimotes[i])
		{
			g_wiimotes[i]->StopThread();
			g_wiimotes[i]->Prepare(i);
			g_wiimotes[i]->StartThread();
		}
	}

	HandleFoundWiimotes(found_wiimotes);
	}
	
	Initialize();
}

void WriteImmediately(int _WiimoteNumber, u16 _channelID, const u8* _pData, u32 _Size)
{
	std::lock_guard<std::recursive_mutex> lk(g_refresh_lock);

	if (g_wiimotes[_WiimoteNumber])
		g_wiimotes[_WiimoteNumber]->WriteImmediately(_channelID, _pData, _Size);
}
void InterruptChannel(int _WiimoteNumber, u16 _channelID, const void* _pData, u32 _Size)
{
	std::lock_guard<std::recursive_mutex> lk(g_refresh_lock);

	if (g_wiimotes[_WiimoteNumber])
		g_wiimotes[_WiimoteNumber]->InterruptChannel(_channelID, _pData, _Size);
}

void ControlChannel(int _WiimoteNumber, u16 _channelID, const void* _pData, u32 _Size)
{
	std::lock_guard<std::recursive_mutex> lk(g_refresh_lock);

	if (g_wiimotes[_WiimoteNumber])
		g_wiimotes[_WiimoteNumber]->ControlChannel(_channelID, _pData, _Size);
}


// Read the Wiimote once
void Update(int _WiimoteNumber)
{
	std::lock_guard<std::recursive_mutex> lk(g_refresh_lock);

	if (g_wiimotes[_WiimoteNumber])
		g_wiimotes[_WiimoteNumber]->Update();

	// Wiimote::Update() may remove the wiimote if it was disconnected.
	if (!g_wiimotes[_WiimoteNumber])
	{
		listeners.notify(&wiimote_listener::wiimote_connection_changed, _WiimoteNumber, false);
	}
}

void StateChange(EMUSTATE_CHANGE newState)
{
	//std::lock_guard<std::recursive_mutex> lk(g_refresh_lock);

	// TODO: disable/enable auto reporting, maybe
}

bool IsValidBluetoothName(const std::string& name)
{
	return
		"Nintendo RVL-CNT-01" == name ||
		"Nintendo RVL-CNT-01-TR" == name ||
		"Nintendo RVL-WBC-01" == name;
}

}; // end of namespace
