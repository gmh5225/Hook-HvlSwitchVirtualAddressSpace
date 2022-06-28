#include "HideMemory.h"
#include "Memory.h"

#define MAKE_TYPE_ADD(type,point,add)((type)((ULONG64)(point) + ((ULONG64)(add))))

NTKERNELAPI CHAR* PsGetProcessImageFileName(PEPROCESS);

static ULONG64 g_ProcetCr3 = 0;

static ULONG64 g_FakerCr3  = 0;

static ULONG64 Fution[3][2] = { 0,0 };

NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(__in PEPROCESS Process);

//0xc bytes (sizeof)
typedef struct _RUNTIME_FUNCTION
{
	ULONG BeginAddress;                                                     //0x0
	ULONG EndAddress;                                                       //0x4
	union
	{
		ULONG UnwindInfoAddress;                                            //0x8
		ULONG UnwindData;                                                   //0x8
	};
}RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

PRUNTIME_FUNCTION RtlLookupFunctionEntry(
	DWORD64  ControlPc,
	PDWORD64 ImageBase,
	PVOID	 HistoryTable
);

PVOID KeLookupFunctionInfo(PVOID Address, PVOID* BeginAddress, PVOID* EndAddress, SIZE_T* BodySize)
{
	PVOID ImageBase = NULL;
	PRUNTIME_FUNCTION EntryTable;
	if (EntryTable =  RtlLookupFunctionEntry((DWORD64)Address, (PDWORD64)&ImageBase, NULL))
	{
		while (TRUE) {
			if (FALSE == MmIsAddressValid(ImageBase) || FALSE ==  MmIsAddressValid(EntryTable) || FALSE ==  MmIsAddressValid(&EntryTable->UnwindData))
				break;
			PVOID BegiAddress = MAKE_TYPE_ADD(PVOID, ImageBase, EntryTable->BeginAddress);
			PVOID EndpAddress = MAKE_TYPE_ADD(PVOID, ImageBase, EntryTable->EndAddress);
			if (FALSE == MmIsAddressValid(BegiAddress) || FALSE ==  MmIsAddressValid(EndpAddress))
				break;
			if (Address < BegiAddress || Address > EndpAddress)
				break;
			if (BeginAddress)
				*BeginAddress = BegiAddress;
			if (EndAddress)
				*EndAddress = EndpAddress;
			if (BodySize)
				*BodySize = EntryTable->EndAddress - EntryTable->BeginAddress;
			return EndpAddress;
		}
	}
	return BeginAddress;
}
 
 
void HvPreCall(ULONG64 HvType, ULONG64 NewCr3, ULONG64 UnKnow_1, PETHREAD OldThread)
{
	ULONG64 Ret[12] = {0};

	BOOLEAN  IsReadProcess = FALSE;
 
	RtlWalkFrameChain((PVOID*)Ret,12,0);

	if (HvType == HV_TYPE_SWAP_PT && NewCr3)
	{
		if (FALSE == BitTest64((LONG64*)&NewCr3, 1))
		{
			__writecr4(__readcr4() ^ 0x80ull);
			__writecr4(__readcr4() ^ 0x80ull);
		}
 

		for (ULONG64 Index = 0; Index < 12; Index++)
		{
			if (Ret[Index] > Fution[0][0] && Ret[Index] < Fution[0][1])
			{
				IsReadProcess = TRUE;
				break;
			}

			if (Ret[Index] > Fution[1][0] && Ret[Index] < Fution[1][1])
			{
				IsReadProcess = TRUE;
				break;
			}
		}
 
		if (IsReadProcess = FALSE)
		{
			__writecr3(NewCr3);
			goto $EXIT;
		}


		if (g_ProcetCr3 == NewCr3)
		{
			CHAR* Name = PsGetProcessImageFileName(PsGetThreadProcess(PsGetCurrentThread()));
			if (strcmp(Name, "Hacking.exe") == 0)
			{
				//DbgPrintEx(77, 0, "Read\n");
 
				__writecr3(g_FakerCr3);
				goto $EXIT; 
			 
			}
		}

 
		__writecr3(NewCr3);
	}
 

$EXIT:
	return;
}

 
PVOID GetProcAddress(PCWSTR name)
{
	UNICODE_STRING Uname;
	RtlInitUnicodeString(&Uname, name);
	return MmGetSystemRoutineAddress(&Uname);
}
 

NTSTATUS DriverEntry(PDRIVER_OBJECT ObjDrv, PUNICODE_STRING PathDrv)
{

	PEPROCESS Eprocess = GetProcessEprocess(ProcessNameGetProcressId(L"*DWM.EXE"), FALSE);

	g_ProcetCr3 = *(ULONG64 *)((ULONG64)Eprocess + 0X28);

	KeAttachProcess(Eprocess);
	g_FakerCr3 = RebuildPageDirectory(__readcr3());
 
	ClreaAddressPTE(g_FakerCr3, (PVOID)PsGetProcessSectionBaseAddress(Eprocess));
	KeDetachProcess();
 




	//HvRegisterPageMonitoring(HvPreCall);

	return STATUS_SUCCESS;
}