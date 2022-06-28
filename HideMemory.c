#include "HideMemory.h"
#include "search.h"
#include "Memory.h"
NTKERNELAPI void HvlInvokeHypercall();

NTKERNELAPI void KeFlushEntireTb();

ULONG64 g_FuncPointer = 0;

void __asm_set_hv_handler(PVOID);

void __asm_hv_dispatcher_handler();

ULONG64(*Ori_HvlSwapGuestVirtualSpace)(ULONG64, ULONG64, ULONG64) = NULL;
 
static HV_CALL_BACK CallBack = NULL;

PVOID RtlGetHypervHvcallCodeVa()
{
	PVOID pHvCallpCodeVaStub = NULL;

	PVOID KernelBase = GetKernelBase(NULL);
 
	unsigned char *tmp = Scan_Pattem_Image(KernelBase, "0F 10 6A 60 0F 10 62  50 0F 10 5A 40 0F 10 52  30 0F 10 4A 20 0F 10 42 10 4C 8B 42 08 48 8B 12", "*");
	if (!tmp)
	{
		KeBugCheck(0x1942B);
	}

	while (tmp++)
	{
		if (*(USHORT*)tmp == 0x15FF)
		{
			pHvCallpCodeVaStub = RELOC(tmp, 2);
			break;
		}
		if (tmp[0] == 0x48 && tmp[1] == 0x8B && tmp[2] == 0x05)
		{
			pHvCallpCodeVaStub =  RELOC(tmp, 3);
			break;
		}
	}
 
//	UCHAR* ControlPC = (UCHAR*)((ULONG64)HvlInvokeHypercall + 0x07);
//	return RVA_VA(ControlPC);

	return pHvCallpCodeVaStub;
}

PVOID RtlGetHypervlEnlightenments()
{ 
 
	PVOID KernelBase = GetKernelBase(NULL);

	unsigned char *tmp = Scan_Pattem_Image(KernelBase, "F7 05 ? ? ? ? 01 00 00 00 74 07 E8 ? ? ? ? EB ? 0F 22 D9", "*");
	DbgPrintEx(77, 0, "%p\n", tmp);
	UCHAR *Enlightenments = RELOC(tmp, 2);
	DbgPrintEx(77, 0, "%p\n", Enlightenments);
	return (PVOID)((ULONG64)Enlightenments + 4);

}


ULONG64  HvlSwapGuestVirtualSpaceCallBack(ULONG64 HvType, ULONG64 NewCr3, ULONG64 UnKnow_1, PETHREAD OldThread)
{
 
	CallBack(HvType, NewCr3, UnKnow_1, OldThread);
	return  2;
}

 __int64 WPOFFx64() {
	__int64 irql = -1;
	if (KeGetCurrentIrql() < DISPATCH_LEVEL)
	{
		irql = KeRaiseIrqlToDpcLevel();
	}
	UINT64 cr0 = __readcr0();
	cr0 &= 0xfffffffffffeffff;
	__writecr0(cr0);
	_disable();
	return irql;
}
 void WPONx64(__int64 irql) {
	UINT64 cr0 = __readcr0();
	cr0 |= 0x10000;
	_enable();
	__writecr0(cr0);
	if (irql >= 0 && KeGetCurrentIrql() > irql)
	{
		KeLowerIrql((KIRQL)irql);
	}
}
BOOLEAN HvRegisterPageMonitoring(HV_CALL_BACK PreCall)
{
	PHYSICAL_ADDRESS PA;
	PA.QuadPart = 0xFFFFFFFFFFFFFFFF;
	PVOID HookCode = MmAllocateContiguousMemory(0x1000, PA);
 
	PVOID HvcallCode  =  *(PVOID *)RtlGetHypervHvcallCodeVa();

	PVOID HvlEnlightenments =  RtlGetHypervlEnlightenments();

	RtlZeroMemory(HookCode, 0);

	RtlCopyMemory(HookCode, __asm_hv_dispatcher_handler, 0x100);

	for (ULONG Index = 0; Index < 0x100; Index++)
	{
		ULONG64 Tag = ((ULONG64)HookCode + Index);
		if (*(ULONG64 *)Tag == 0x112233344556677)
		{
			*(ULONG64 *)Tag = (ULONG64)HvlSwapGuestVirtualSpaceCallBack;
			break;
		}
	}

	CallBack = PreCall;

	//Ori_HvlSwapGuestVirtualSpace =  *HvcallCode;

	//*HvcallCode = __asm_hv_dispatcher_handler;

	  
	__asm_set_hv_handler(HvlSwapGuestVirtualSpaceCallBack);
 
	ULONG64 PTE = SetGetAddressPTE(__readcr3(), HookCode, 0);

	SetGetAddressPTE(__readcr3(),HvcallCode, PTE);


	 *(ULONG32*)HvlEnlightenments = TRUE;

	return TRUE;
}



