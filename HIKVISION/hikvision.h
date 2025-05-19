#pragma once

#define XWF_ITEM_INFO_ORIG_ID 1
#define XWF_ITEM_INFO_ATTR 2
#define XWF_ITEM_INFO_FLAGS 3
#define XWF_ITEM_INFO_DELETION 4
#define XWF_ITEM_INFO_CLASSIFICATION 5
#define XWF_ITEM_INFO_LINKCOUNT 6
#define XWF_ITEM_INFO_COLORANALYSIS 7
#define XWF_ITEM_INFO_PIXELINDEX 8
#define XWF_ITEM_INFO_FILECOUNT 11
#define XWF_ITEM_INFO_EMBEDDEDOFFSET 16
#define XWF_ITEM_INFO_CREATIONTIME 32
#define XWF_ITEM_INFO_MODIFICATIONTIME 33
#define XWF_ITEM_INFO_LASTACCESSTIME 34
#define XWF_ITEM_INFO_ENTRYMODIFICATIONTIME 35
#define XWF_ITEM_INFO_DELETIONTIME 36
#define XWF_ITEM_INFO_INTERNALCREATIONTIME 37

#define XT_PREPARE_CALLPI 0x01
#define XT_PREPARE_CALLPILATE 0x02
#define XT_PREPARE_EXPECTMOREITEMS 0x04
#define XT_PREPARE_DONTOMIT 0x08
#define XT_PREPARE_TARGETDIRS 0x10
#define XT_PREPARE_TARGETZEROBYTEFILES 0x20

#define XT_INIT_NOT_THREAD_SAFE 1

#define XT_INIT_THREAD_SAFE 2

#define XWF_HASHTYPE_MD5	7
#define XWF_HASHTYPE_SHA1	8
#define XWF_HASHTYPE_SHA256 9

#define WINDOWS_TICK 10000000
#define SEC_TO_UNIX_EPOCH 11644473600LL


struct HikHeader {
	uint64_t hddCap;
	uint64_t sysLogOffset;
	uint64_t sysLogSize;
	uint64_t videoDataAreaOffset;
	uint64_t dataBlockSize;
	uint32_t dataBlockTotal;
	uint64_t hikBtree1Offset;
	uint64_t hikBtree2Offset;
	uint32_t hikBtree1Size;
	uint32_t hikBtree2Size;
	uint64_t initTime;
	uint64_t signature_high;
	uint64_t signature_low;
};

struct HikBTree {
	uint64_t signature;
	uint32_t createdTime;
	uint64_t footerOffset;
	uint64_t pageListOffset;
	uint64_t pageOneOffset;
};

struct HikDataBlockEntry {
	uint64_t unused;
	uint64_t fileExists;
	uint64_t channel;
	uint32_t startTime;
	uint32_t endTime;
	uint64_t dataOffset;
};

struct HikPageEntry {
	uint64_t pageOffset;
	uint16_t channel;
	uint32_t startTime;
	uint32_t endTime;
	uint64_t dataOffset;
	std::vector<HikDataBlockEntry> dataBlockEntries;
};

struct HikPageList {
	std::vector<HikPageEntry> pageList;
};

std::unique_ptr<BYTE[]> readBytes(uint64_t size, HANDLE hItem);

void setPosition(uint64_t newPosition);

void skipBytes(uint64_t bytes);

DWORD readHeader(HANDLE hItem, HikHeader &hikHeader, HikBTree &hikBTree);

DWORD readHikBTree(HANDLE hItem, HikHeader& hikHeader, HikBTree &hikBTree);

DWORD readPageList(HANDLE hItem, HikBTree &hikBTree, HikPageList &hikPageList);

DWORD readPageEntries(HANDLE hItem, HikPageList& hikPageList);

DWORD createVSItems(HANDLE hItem, HikHeader& hikHeader, HikPageList& hikPageList);

INT64 unixtimeToFiletime(uint64_t unixtime);

const wchar_t* unixtimeToWString(int unixtime);
