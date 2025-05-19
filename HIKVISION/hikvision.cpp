#include "pch.h"
#include "hikvision.h"

static uint64_t currentPosition = 0;

#pragma pack(2)
struct DriveInfo {
	DWORD nSize;
	LONG nDrive;
	LONG nParentDrive;
	DWORD nBytesPerSector;
	INT64 nSectorCount;
	INT64 nParentSectorCount;
	INT64 nStartSectorOnParent;
	LPVOID lpPrivate;
};

LONG __stdcall XT_Init(CallerInfo info, DWORD nFlags, HANDLE hMainWnd, struct LicenseInfo* pLicInfo)
{
	XT_RetrieveFunctionPointers();
	XWF_OutputMessage(L"Starting HIKVISION X-Tension", 0);

	if (nFlags == XT_INIT_QUICKCHECK || nFlags == XT_INIT_ABOUTONLY) {

		return XT_INIT_THREAD_SAFE;

	}
	return XT_INIT_NOT_THREAD_SAFE;
}

LONG __stdcall XT_Done(void* lpReserved)
{
	XWF_OutputMessage(L"HIKVISION X-Tension done.", 0);
	return 0;
}

LONG __stdcall XT_About(HANDLE hParentWnd, void* lpReserved)
{
	XWF_OutputMessage(L"X-Ways X-Tension to extract video data from HIKVISION filesystem \'HIK.2011.03.08\'. NO WARRANTY. SOFTWARE IS PROVIDED \' AS IS\'", 0);
	return 0;
}

LONG __stdcall XT_Prepare(HANDLE hVolume, HANDLE hEvidence, DWORD nOpType, void* lpReserved)
{
	return XT_PREPARE_CALLPI;
}

LONG __stdcall XT_Finalize(HANDLE hVolume, HANDLE hEvidence, DWORD nOpType, void* lpReserved)
{
	XWF_OutputMessage(L"Finished.", 0);
	return 0;
}

LONG XT_ProcessItemEx(LONG nItemID, HANDLE hItem, PVOID lpReserved)
{
	// XWF_OutputMessage(L"XT_ProcessItemEx", 0);
	// XWF_OutputMessage(L"Read HIKVISION Header", 0);

	HikHeader hikHeader;
	HikBTree hikBTree;
	HikPageList hikPageList;

	if (readHeader(hItem, hikHeader, hikBTree) != 0) 
	{
		XWF_OutputMessage(L"Signature mismatch, returning", 0);
		return 0;
	}

	readHikBTree(hItem, hikHeader, hikBTree);

	readPageList(hItem, hikBTree, hikPageList);

	readPageEntries(hItem, hikPageList);
		
	createVSItems(hItem, hikHeader, hikPageList);

	return 0;
}

std::unique_ptr<BYTE[]> readBytes(uint64_t size, HANDLE hItem)
{
	std::unique_ptr<BYTE[]> buffer(new BYTE[size]);
	ZeroMemory(buffer.get(), size);
	XWF_Read(hItem, currentPosition, buffer.get(), size);
	currentPosition += size;
	return buffer;
}

void skipBytes(uint64_t bytes)
{
	currentPosition += bytes;
}

void setPosition(uint64_t newPosition)
{
	currentPosition = newPosition;
}

DWORD readHeader(HANDLE hItem, HikHeader &hikHeader, HikBTree &hikBTree)
{
	skipBytes(528);

	memcpy(&hikHeader.signature_high, readBytes(8, hItem).get(), 8);

	memcpy(&hikHeader.signature_low, readBytes(8, hItem).get(), 8);

	if (_byteswap_uint64(hikHeader.signature_high) != 0x48494B564953494F && _byteswap_uint64(hikHeader.signature_low) != 0x4E4048414E475A48)
	{
		XWF_OutputMessage(L"Signature mismatch! Returning.", 0);
		return 0;
	}

	skipBytes(40);

	memcpy(&hikHeader.hddCap, readBytes(8, hItem).get(), 8);

	skipBytes(16);

	memcpy(&hikHeader.sysLogOffset, readBytes(8, hItem).get(), 8);

	memcpy(&hikHeader.sysLogSize, readBytes(8, hItem).get(), 8);

	skipBytes(8);

	memcpy(&hikHeader.videoDataAreaOffset, readBytes(8, hItem).get(), 8);

	skipBytes(8);

	memcpy(&hikHeader.dataBlockSize, readBytes(8, hItem).get(), 8);

	memcpy(&hikHeader.dataBlockTotal, readBytes(4, hItem).get(), 4);

	skipBytes(4);

	memcpy(&hikHeader.hikBtree1Offset, readBytes(8, hItem).get(), 8);

	memcpy(&hikHeader.hikBtree1Size, readBytes(4, hItem).get(), 4);

	skipBytes(4);

	memcpy(&hikHeader.hikBtree2Offset, readBytes(8, hItem).get(), 8);

	memcpy(&hikHeader.hikBtree2Size, readBytes(4, hItem).get(), 4);

	skipBytes(60);

	memcpy(&hikHeader.initTime, readBytes(4, hItem).get(), 4);

	return 0;
}

DWORD readHikBTree(HANDLE hItem, HikHeader& hikHeader, HikBTree &hikBTree)
{
	// XWF_OutputMessage(L"Read HIKVISION HikBTree", 0);

	setPosition(hikHeader.hikBtree1Offset);

	skipBytes(16);

	memcpy(&hikBTree.signature, readBytes(8, hItem).get(), 8);
	skipBytes(36);

	memcpy(&hikBTree.createdTime, readBytes(4, hItem).get(), 4);
	memcpy(&hikBTree.footerOffset, readBytes(8, hItem).get(), 8);

	skipBytes(8);

	memcpy(&hikBTree.pageListOffset, readBytes(8, hItem).get(), 8);

	memcpy(&hikBTree.pageOneOffset, readBytes(8, hItem).get(), 8);

	return 0;
}

DWORD readPageList(HANDLE hItem, HikBTree& hikBTree, HikPageList& hikPageList)
{
	// XWF_OutputMessage(L"Read HIKVISION HikPageList", 0);

	uint64_t offset = 0;
	uint64_t firstPageOffset = 0;
	uint64_t pageOffset = 0;

	setPosition(hikBTree.pageListOffset);

	skipBytes(24);

	memcpy(&firstPageOffset, readBytes(8, hItem).get(), 8);

	skipBytes(64);

	memcpy(&pageOffset, readBytes(8, hItem).get(), 8);

	HikPageEntry hpe;

	hpe.pageOffset = hikBTree.pageOneOffset;
	hikPageList.pageList.push_back(hpe);

	while (pageOffset != 0)
	{
		hpe.pageOffset = pageOffset;
		skipBytes(8);

		memcpy(&hpe.channel, readBytes(2, hItem).get(), 2);
		hpe.channel = _byteswap_ushort(hpe.channel);

		skipBytes(6);

		memcpy(&hpe.startTime, readBytes(4, hItem).get(), 4);

		memcpy(&hpe.endTime, readBytes(4, hItem).get(), 4);

		memcpy(&hpe.dataOffset, readBytes(8, hItem).get(), 8);

		skipBytes(8);

		hikPageList.pageList.push_back(hpe);

		memcpy(&pageOffset, readBytes(8, hItem).get(), 8);
	}
	return 0;
}

DWORD readPageEntries(HANDLE hItem, HikPageList& hikPageList)
{
	for (HikPageEntry &hpe : hikPageList.pageList)
	{
		setPosition(hpe.pageOffset);
		skipBytes(96);
		uint64_t unusedBytes = 0;
		memcpy(&unusedBytes, readBytes(8, hItem).get(), 8);

		while (unusedBytes == 0xFFFFFFFFFFFFFFFF)
		{
			HikDataBlockEntry hdbe;
			memcpy(&hdbe.fileExists, readBytes(8, hItem).get(), 8);
			memcpy(&hdbe.channel, readBytes(2, hItem).get(), 2);
			hdbe.channel = _byteswap_ushort(hdbe.channel);
			skipBytes(6);
			memcpy(&hdbe.startTime, readBytes(4, hItem).get(), 4);
			memcpy(&hdbe.endTime, readBytes(4, hItem).get(), 4);
			memcpy(&hdbe.dataOffset, readBytes(8, hItem).get(), 8);
			skipBytes(8);
			memcpy(&unusedBytes, readBytes(8, hItem).get(), 8);
			hpe.dataBlockEntries.push_back(hdbe);
		}
	}
	return 0;
}

DWORD createVSItems(HANDLE hItem, HikHeader& hikHeader ,HikPageList& hikPageList)
{
	const std::wstring itemType = std::wstring(L"mp4\0");

	std::unordered_map<uint64_t, std::vector<HikDataBlockEntry>> fileStructure;

	for (HikPageEntry hpe : hikPageList.pageList)
	{
		for (HikDataBlockEntry &hdbe : hpe.dataBlockEntries)
		{
			fileStructure[hdbe.channel].push_back(hdbe);
		}
	}

	for (const std::pair<uint64_t, std::vector<HikDataBlockEntry>>& pair : fileStructure)
	{
		const uint64_t channel = pair.first;
		const std::vector<HikDataBlockEntry> fileList = pair.second;

		std::wstring folderName = std::wstring(L"Channel ") + std::to_wstring(channel);
		int parentId = XWF_CreateItem(const_cast<LPWSTR>(folderName.c_str()), 0x00000001);
		XWF_SetItemInformation(parentId, XWF_ITEM_INFO_FLAGS, 0x00000001);
		XWF_SetItemParent(parentId, 0);

		int fileCounter = 0;
		for (const HikDataBlockEntry &hdbe : fileList)
		{
			std::wstring fileName = const_cast<LPWSTR>(unixtimeToWString(hdbe.startTime));
			int childId = XWF_CreateItem(const_cast<LPWSTR>(fileName.c_str()), 0x00000001);
			XWF_SetItemInformation(childId, XWF_ITEM_INFO_CREATIONTIME, unixtimeToFiletime(hdbe.startTime));
			XWF_SetItemInformation(childId, XWF_ITEM_INFO_MODIFICATIONTIME, unixtimeToFiletime(hdbe.endTime));
			XWF_SetItemSize(childId, hikHeader.dataBlockSize);
			XWF_SetItemType(childId, const_cast <LPWSTR>(itemType.c_str()), 3);
			XWF_SetItemParent(childId, parentId);
			XWF_SetItemOfs(childId, (-1) * hdbe.dataOffset, hdbe.dataOffset / 512);
			fileCounter++;
		}
		XWF_SetItemInformation(parentId, XWF_ITEM_INFO_FILECOUNT, fileCounter);

		uint64_t folderSize = fileCounter * hikHeader.dataBlockSize;
		XWF_SetItemSize(parentId, folderSize);
	}

	return 0;
}

INT64 unixtimeToFiletime(uint64_t unixtime)
{
	const uint64_t EPOCH_DIFFERENCE = 116444736000000000ULL;
	uint64_t fileTimeValue = unixtime * 10000000ULL + EPOCH_DIFFERENCE;

	FILETIME ft;
	ULARGE_INTEGER ull;
	ft.dwLowDateTime = static_cast<DWORD>(fileTimeValue & 0xFFFFFFFF);
	ft.dwHighDateTime = static_cast<DWORD>(fileTimeValue >> 32);

	ull.LowPart = ft.dwLowDateTime;
	ull.HighPart = ft.dwHighDateTime;

	INT64 ret = static_cast<INT64>(ull.QuadPart);

	return ret;
}

const wchar_t* unixtimeToWString(int unixtime)
{
	static wchar_t buffer[100];
	time_t rawTime = static_cast<time_t>(unixtime);
	tm timeInfo;

	if (localtime_s(&timeInfo, &rawTime) != 0)
	{
		return L"Timeerror";
	}

	wcsftime(buffer, sizeof(buffer) / sizeof(wchar_t), L"%d.%m.%Y %H:%M:%S", &timeInfo);

	return buffer;
}
