/*
The ttaplugins-winamp project.
Copyright (C) 2005-2026 Yamagata Fumihiro

This file is part of enc_tta.

enc_tta is free software: you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation, either
version 3 of the License, or any later version.

enc_tta is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along with enc_tta.
If not, see <https://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

#include "AudioCoderTTA.h"
#include "enc_tta.h"
#include "VersionNo.h"

#include <libtta.h>

// wasabi based services for localisation support
#include <api/service/waServiceFactory.h>
#include <Agave/Language/api_language.h>
#include <Winamp/wa_ipc.h>
#include <mmsystem.h>
#include <Wasabi/bfc/platform/platform.h>

#include <strsafe.h>

HWND winampwnd = 0;
api_service *WASABI_API_SVC = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

const static int MAX_MESSAGE_LENGTH = 1024;

typedef struct
{
	//	configtype cfg;
	char configfile[MAX_PATH];
}
configwndrec;

// {65c17c78-f2d6-43fa-857f-386734fa48e5}
static const GUID EncTTALangGUID =
{ 0x65c17c78, 0xf2d6, 0x43fa, { 0x85, 0x7f, 0x38, 0x67, 0x34, 0xfa, 0x48, 0xe5 } };

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return TRUE;
}

static HINSTANCE GetMyInstance()
{
	MEMORY_BASIC_INFORMATION mbi = { 0 };
	if (VirtualQuery(GetMyInstance, &mbi, sizeof(mbi)))
		return static_cast<HINSTANCE>(mbi.AllocationBase);
	return nullptr;
}

void GetLocalisationApiService(void)
{
	if (!WASABI_API_LNG)
	{
		// loader so that we can get the localisation service api for use
		if (!WASABI_API_SVC)
		{
			WASABI_API_SVC = reinterpret_cast<api_service*>(SendMessage(winampwnd, WM_WA_IPC, 0, IPC_GET_API_SERVICE));
			if (WASABI_API_SVC == reinterpret_cast<api_service*>(1))
			{
				WASABI_API_SVC = nullptr;
				return;
			}
			else
			{
				// Do nothing
			}
		}
		else
		{
			// Do nothing
		}

		if (!WASABI_API_LNG)
		{
			waServiceFactory *sf;
			sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
			if (sf)
			{
				WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());
			}
			else
			{
				// Do nothing
			}
		}
		else
		{
			// Do nothing
		}

		// need to have this initialised before we try to do anything with localisation features
		WASABI_API_START_LANG(GetMyInstance(), EncTTALangGUID);
	}
}

extern "C"
{
	unsigned int __declspec(dllexport) GetAudioTypes3(int idx, char *desc)
	{
		if (idx == 0)
		{
			GetLocalisationApiService();
			StringCchPrintfA(desc, 1024, WASABI_API_LNGSTRING(IDS_ENC_TTA_DESC), ENC_TTA_VERSION_CHAR);
			return mmioFOURCC('T', 'T', 'A', ' ');
		}
		else
		{
			// Do nothing
		}
		return 0;
	}

	AudioCoder __declspec(dllexport) *CreateAudio3(int nch, int srate, int bps, unsigned int srct, unsigned int *outt, char *configfile)
	{
		if (srct == mmioFOURCC('P', 'C', 'M', ' ') && *outt == mmioFOURCC('T', 'T', 'A', ' '))
		{
			//			configtype cfg;
			//			readconfig(configfile, &cfg);
			*outt = mmioFOURCC('T', 'T', 'A', ' ');
			AudioCoderTTA *t = nullptr;
			try
			{
				t = new AudioCoderTTA(nch, srate, bps);
			}
			catch (const tta::tta_exception& e)
			{
				delete t;
				return nullptr;
			}

			return t;
		}
		else
		{
			// Do nothing
		}
		return nullptr;
	}

	void __declspec(dllexport) FinishAudio3(const char *filename, AudioCoder *coder)
	{
		((AudioCoderTTA*)coder)->FinishAudio(filename);
	}

	void __declspec(dllexport) FinishAudio3W(const wchar_t *filename, AudioCoder *coder)
	{
		((AudioCoderTTA*)coder)->FinishAudio(filename);
	}

	void __declspec(dllexport) PrepareToFinish(const char *filename, AudioCoder *coder)
	{
		((AudioCoderTTA*)coder)->PrepareToFinish();
	}

	void __declspec(dllexport) PrepareToFinishW(const wchar_t *filename, AudioCoder *coder)
	{
		((AudioCoderTTA*)coder)->PrepareToFinish();
	}

	BOOL CALLBACK DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		static configwndrec *wr;
		switch (uMsg)
		{
		case WM_INITDIALOG:
			wr = (configwndrec *)lParam;
			//			SendMessage(GetDlgItem(hwndDlg, IDC_COMPRESSIONSLIDER), TBM_SETRANGE, TRUE, MAKELONG(0, 12));
			//			SendMessage(GetDlgItem(hwndDlg, IDC_COMPRESSIONSLIDER), TBM_SETPOS, TRUE, wr->cfg.compression);
			break;

		case WM_NOTIFY:
			//			if (wParam == IDC_COMPRESSIONSLIDER)
			//			{
			//				LPNMHDR l = (LPNMHDR)lParam;
			//				if (l->idFrom == IDC_COMPRESSIONSLIDER)
			//					wr->cfg.compression = SendMessage(GetDlgItem(hwndDlg, IDC_COMPRESSIONSLIDER), TBM_GETPOS, 0, 0);
			//			}
			break;

		case WM_DESTROY:
			//			writeconfig(wr->configfile, &wr->cfg);
			free(wr); wr = nullptr;
			break;
		}
		return 0;
	}

	HWND __declspec(dllexport) ConfigAudio3(HWND hwndParent, HINSTANCE hinst, unsigned int outt, char *configfile)
	{
		if (outt == mmioFOURCC('T', 'T', 'A', ' '))
		{
			//	configwndrec *wr = (configwndrec*)malloc(sizeof(configwndrec));
			//	if (configfile) lstrcpyn(wr->configfile, configfile, MAX_PATH);
			//		else wr->configfile[0] = 0;
			//			readconfig(configfile, &wr->cfg);
			//	GetLocalisationApiService();
			//			return WASABI_API_CREATEDIALOGPARAM(IDD_CONFIG, hwndParent, DlgProc, (LPARAM)wr);
		}
		else
		{
			// Do nothing
		}
		return nullptr;
	}

	int __declspec(dllexport) SetConfigItem(unsigned int outt, char *item, char *data, char *configfile)
	{
		// nothing yet
		return 0;
	}

	int __declspec(dllexport) GetConfigItem(unsigned int outt, char *item, char *data, int len, char *configfile)
	{
		if (outt == mmioFOURCC('T', 'T', 'A', ' '))
		{
			//			configtype cfg;
			//			readconfig(configfile, &cfg);
			//			if (!lstrcmpi(item, "bitrate"))  lstrcpynA(data, "755", len); // FUCKO: this is ment to be an estimate for approximations of output filesize (used by ml_pmp). Improve this.
			//			else if (!lstrcmpi(item, "extension")) lstrcpynA(data, "flac", len);
			return 1;
		}
		else
		{
			// Do nothing
		}
		return 0;
	}

	void __declspec(dllexport) SetWinampHWND(HWND hwnd)
	{
		winampwnd = hwnd;
	}
};