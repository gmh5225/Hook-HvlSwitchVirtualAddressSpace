#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>
 
#include <intrin.h>
#include <ntstrsafe.h>

#define HV_TYPE_SWAP_PT 0x10001ull

typedef void (*HV_CALL_BACK)(ULONG64 HvType, ULONG64 NewCr3, ULONG64 UnKnow_1, PETHREAD OldThread);

#define RVA_VA(p) ((PVOID)((PCHAR)(ULONG_PTR)(p) + *(PLONG)(ULONG_PTR)(p) + sizeof(LONG)))

PVOID  RtlGetHypervHvcallCodeVa();

PVOID  RtlGetHypervlEnlightenments();

BOOLEAN  HvRegisterPageMonitoring(HV_CALL_BACK PreCall);