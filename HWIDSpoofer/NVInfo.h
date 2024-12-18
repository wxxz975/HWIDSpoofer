#pragma once

typedef unsigned char      NvU8;
typedef NvU8 NvBool;

typedef unsigned long      NvU32;
typedef unsigned short     NvU16;


typedef enum
{
    GPU_GC6_STATE_POWERED_ON = 0,
    GPU_GC6_STATE_EXITED = GPU_GC6_STATE_POWERED_ON,
    GPU_GC6_STATE_ENTERING,
    GPU_GC6_STATE_ENTERING_FAILED,
    GPU_GC6_STATE_ENTERED,
    GPU_GC6_STATE_EXITING,
    GPU_GC6_STATE_EXITING_FAILED,
} GPU_GC6_STATE;

typedef enum NV2080_CTRL_GC6_FLAVOR_ID {
    NV2080_CTRL_GC6_FLAVOR_ID_MSHYBRID = 0,
    NV2080_CTRL_GC6_FLAVOR_ID_OPTIMUS = 1,


    NV2080_CTRL_GC6_FLAVOR_ID_MAX = 4,
} NV2080_CTRL_GC6_FLAVOR_ID;
typedef struct
{
    NvU32               refCount;
    NvU16               GC6PerstDelay;          // waiting time for Upstream Port of GPU,
    // before asserting perst# signal,
    // during RTD3/GC6 Entry.
    NvU16               GC6TotalBoardPower;     // Power required by GPU to sustain RTD3/GC6.
    GPU_GC6_STATE       currentState;
    NvU32               executedStepMask;       // step mask executed during entry sequence
    NvU32               stepMask[NV2080_CTRL_GC6_FLAVOR_ID_MAX];  // step mask cache
} _GPU_GC6_STATE;

struct NVOC_CHILD_TREE
{
    struct Object* pChild;
    struct Object* pSibling;
};
struct Object {

    // Metadata
    const struct NVOC_RTTI* __nvoc_rtti;

    // Ancestor object pointers for `staticCast` feature
    struct Object* __nvoc_pbase_Object;    // obj

    // Data members
    struct Object* pParent;
    struct NVOC_CHILD_TREE childTree;
    NvU32 ipVersion;
    NvU32 createFlags;
};
struct ChipHal {
    unsigned short __nvoc_HalVarIdx;
};

struct RmVariantHal {
    unsigned short __nvoc_HalVarIdx;
};

struct TegraChipHal {
    unsigned short __nvoc_HalVarIdx;
};

struct DispIpHal {
    unsigned short __nvoc_HalVarIdx;
};
struct RmHalspecOwner {

    // Metadata
    const struct NVOC_RTTI* __nvoc_rtti;

    // Ancestor object pointers for `staticCast` feature
    struct RmHalspecOwner* __nvoc_pbase_RmHalspecOwner;    // rmhalspecowner

    // Data members
    struct ChipHal chipHal;
    struct RmVariantHal rmVariantHal;
    struct TegraChipHal tegraChipHal;
    struct DispIpHal dispIpHal;
};


struct OBJTRACEABLE {

    // Metadata
    const struct NVOC_RTTI* __nvoc_rtti;

    // Ancestor object pointers for `staticCast` feature
    struct OBJTRACEABLE* __nvoc_pbase_OBJTRACEABLE;    // traceable
};


#define MAKE_VECTOR(vectTypeName, dataType)                  \
    typedef union vectTypeName##Iter                         \
    {                                                        \
        dataType      *pValue;                               \
        VectorIterBase iter;                                 \
    } vectTypeName##Iter;                                    \
    typedef union vectTypeName                               \
    {                                                        \
        VectorWrapper real;                                  \
        CONT_TAG_TYPE(Vector, dataType, vectTypeName##Iter); \
        CONT_TAG_NON_INTRUSIVE(dataType);                    \
    } vectTypeName

typedef enum NV2080_CTRL_GPU_RECOVERY_ACTION {
    NV2080_CTRL_GPU_RECOVERY_ACTION_NONE = 0,
    NV2080_CTRL_GPU_RECOVERY_ACTION_GPU_RESET = 1,
    NV2080_CTRL_GPU_RECOVERY_ACTION_NODE_REBOOT = 2,
    NV2080_CTRL_GPU_RECOVERY_ACTION_DRAIN_P2P = 3,
} NV2080_CTRL_GPU_RECOVERY_ACTION;

typedef   signed int       NvS32;
#define RM_SHA1_GID_SIZE         16
typedef struct
{
    NvBool              isInitialized;
    NvU8                uuid[RM_SHA1_GID_SIZE];
} _GPU_UUID;

struct OBJGPU {

    // Metadata
    const struct NVOC_RTTI* __nvoc_rtti;

    // Parent (i.e. superclass or base class) object pointers
    struct Object __nvoc_base_Object;
    struct RmHalspecOwner __nvoc_base_RmHalspecOwner;
    struct OBJTRACEABLE __nvoc_base_OBJTRACEABLE;

    // Ancestor object pointers for `staticCast` feature
    struct Object* __nvoc_pbase_Object;    // obj super
    struct RmHalspecOwner* __nvoc_pbase_RmHalspecOwner;    // rmhalspecowner super
    struct OBJTRACEABLE* __nvoc_pbase_OBJTRACEABLE;    // traceable super
    struct OBJGPU* __nvoc_pbase_OBJGPU;    // gpu

    // Vtable with 73 per-object function pointers
    void* vtable[73];
    //NV_STATUS(*__gpuGetDeviceInfoTableSectionInfos__)(struct OBJGPU* /*this*/, DeviceInfoTableSectionVec*);  // halified (2 hals) body
    //NV_STATUS(*__gpuParseDeviceInfoTableSection__)(struct OBJGPU* /*this*/, DeviceInfoEntryVec*, const DeviceInfoTableSection*, const NvU8*);  // halified (3 hals) body
    //NV_STATUS(*__gpuConstructDeviceInfoTable__)(struct OBJGPU* /*this*/);  // halified (3 hals) body
    //NV_STATUS(*__gpuGetNameString__)(struct OBJGPU* /*this*/, NvU32, void*);  // halified (2 hals)
    //NV_STATUS(*__gpuGetShortNameString__)(struct OBJGPU* /*this*/, NvU8*);  // halified (2 hals)
    //NV_STATUS(*__gpuInitBranding__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //void (*__gpuGetRtd3GC6Data__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NV_STATUS(*__gpuSetResetScratchBit__)(struct OBJGPU* /*this*/, NvBool);  // halified (2 hals) body
    //NV_STATUS(*__gpuGetResetScratchBit__)(struct OBJGPU* /*this*/, NvBool*);  // halified (2 hals) body
    //NV_STATUS(*__gpuResetRequiredStateChanged__)(struct OBJGPU* /*this*/, NvBool);  // halified (2 hals) body
    //NV_STATUS(*__gpuMarkDeviceForReset__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NV_STATUS(*__gpuUnmarkDeviceForReset__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NV_STATUS(*__gpuIsDeviceMarkedForReset__)(struct OBJGPU* /*this*/, NvBool*);  // halified (2 hals) body
    //NV_STATUS(*__gpuSetDrainAndResetScratchBit__)(struct OBJGPU* /*this*/, NvBool);  // halified (3 hals) body
    //NV_STATUS(*__gpuGetDrainAndResetScratchBit__)(struct OBJGPU* /*this*/, NvBool*);  // halified (3 hals) body
    //NV_STATUS(*__gpuMarkDeviceForDrainAndReset__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NV_STATUS(*__gpuUnmarkDeviceForDrainAndReset__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NV_STATUS(*__gpuIsDeviceMarkedForDrainAndReset__)(struct OBJGPU* /*this*/, NvBool*);  // halified (2 hals) body
    //NV_STATUS(*__gpuPowerOff__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NV_STATUS(*__gpuWriteBusConfigReg__)(struct OBJGPU* /*this*/, NvU32, NvU32);  // halified (2 hals) body
    //NV_STATUS(*__gpuReadBusConfigReg__)(struct OBJGPU* /*this*/, NvU32, NvU32*);  // halified (2 hals) body
    //NV_STATUS(*__gpuReadBusConfigRegEx__)(struct OBJGPU* /*this*/, NvU32, NvU32*, THREAD_STATE_NODE*);  // halified (2 hals) body
    //NV_STATUS(*__gpuReadFunctionConfigReg__)(struct OBJGPU* /*this*/, NvU32, NvU32, NvU32*);  // halified (2 hals) body
    //NV_STATUS(*__gpuWriteFunctionConfigReg__)(struct OBJGPU* /*this*/, NvU32, NvU32, NvU32);  // halified (2 hals) body
    //NV_STATUS(*__gpuWriteFunctionConfigRegEx__)(struct OBJGPU* /*this*/, NvU32, NvU32, NvU32, THREAD_STATE_NODE*);  // halified (2 hals) body
    //NV_STATUS(*__gpuReadPassThruConfigReg__)(struct OBJGPU* /*this*/, NvU32, NvU32*);  // halified (3 hals) body
    //NV_STATUS(*__gpuConfigAccessSanityCheck__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NV_STATUS(*__gpuReadBusConfigCycle__)(struct OBJGPU* /*this*/, NvU32, NvU32*);  // halified (2 hals)
    //NV_STATUS(*__gpuWriteBusConfigCycle__)(struct OBJGPU* /*this*/, NvU32, NvU32);  // halified (2 hals)
    //void (*__gpuGetIdInfo__)(struct OBJGPU* /*this*/);  // halified (3 hals) body
    //NV_STATUS(*__gpuGenGidData__)(struct OBJGPU* /*this*/, NvU8*, NvU32, NvU32);  // halified (2 hals) body
    //NvU8(*__gpuGetChipSubRev__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NV_STATUS(*__gpuGetSkuInfo__)(struct OBJGPU* /*this*/, NV2080_CTRL_BIOS_GET_SKU_INFO_PARAMS*);  // halified (2 hals) body
    //NV_STATUS(*__gpuGetRegBaseOffset__)(struct OBJGPU* /*this*/, NvU32, NvU32*);  // halified (2 hals) body
    //void (*__gpuHandleSanityCheckRegReadError__)(struct OBJGPU* /*this*/, NvU32, NvU32);  // halified (2 hals) body
    //void (*__gpuHandleSecFault__)(struct OBJGPU* /*this*/);  // halified (3 hals) body
    //NV_STATUS(*__gpuSanityCheckVirtRegAccess__)(struct OBJGPU* /*this*/, NvU32);  // halified (3 hals) body
    //const GPUCHILDPRESENT* (*__gpuGetChildrenPresent__)(struct OBJGPU* /*this*/, NvU32*);  // halified (9 hals)
    //const CLASSDESCRIPTOR* (*__gpuGetClassDescriptorList__)(struct OBJGPU* /*this*/, NvU32*);  // halified (10 hals)
    //NvU32(*__gpuGetPhysAddrWidth__)(struct OBJGPU* /*this*/, NV_ADDRESS_SPACE);  // halified (2 hals)
    //NV_STATUS(*__gpuInitSriov__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NV_STATUS(*__gpuDeinitSriov__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NV_STATUS(*__gpuCreateDefaultClientShare__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //void (*__gpuDestroyDefaultClientShare__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NvBool(*__gpuFuseSupportsDisplay__)(struct OBJGPU* /*this*/);  // halified (3 hals) body
    //NvU64(*__gpuGetActiveFBIOs__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NvBool(*__gpuIsGspToBootInInstInSysMode__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NvBool(*__gpuCheckPageRetirementSupport__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NvBool(*__gpuIsInternalSku__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NV_STATUS(*__gpuClearFbhubPoisonIntrForBug2924523__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NvBool(*__gpuCheckIfFbhubPoisonIntrPending__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NV_STATUS(*__gpuGetSriovCaps__)(struct OBJGPU* /*this*/, NV0080_CTRL_GPU_GET_SRIOV_CAPS_PARAMS*);  // halified (2 hals) body
    //NvBool(*__gpuCheckIsP2PAllocated__)(struct OBJGPU* /*this*/);  // halified (3 hals) body
    //NV_STATUS(*__gpuPrePowerOff__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NV_STATUS(*__gpuVerifyExistence__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NvU64(*__gpuGetFlaVasSize__)(struct OBJGPU* /*this*/, NvBool);  // halified (3 hals) body
    //NvBool(*__gpuIsAtsSupportedWithSmcMemPartitioning__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NvBool(*__gpuIsGlobalPoisonFuseEnabled__)(struct OBJGPU* /*this*/);  // halified (2 hals)
    //void (*__gpuDetermineSelfHostedMode__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NvU32(*__gpuDetermineSelfHostedSocType__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //void (*__gpuDetermineMIGSupport__)(struct OBJGPU* /*this*/);  // halified (3 hals) body
    //NV_STATUS(*__gpuInitOptimusSettings__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NV_STATUS(*__gpuDeinitOptimusSettings__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NvBool(*__gpuIsSliCapableWithoutDisplay__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NvBool(*__gpuIsCCEnabledInHw__)(struct OBJGPU* /*this*/);  // halified (4 hals) body
    //NvBool(*__gpuIsDevModeEnabledInHw__)(struct OBJGPU* /*this*/);  // halified (4 hals) body
    //NvBool(*__gpuIsProtectedPcieEnabledInHw__)(struct OBJGPU* /*this*/);  // halified (3 hals) body
    //NvBool(*__gpuIsCtxBufAllocInPmaSupported__)(struct OBJGPU* /*this*/);  // halified (2 hals) body
    //NV_STATUS(*__gpuUpdateErrorContainmentState__)(struct OBJGPU* /*this*/, NV_ERROR_CONT_ERR_ID, NV_ERROR_CONT_LOCATION, NvU32*);  // halified (2 hals) body
    //NV_STATUS(*__gpuSetPartitionErrorAttribution__)(struct OBJGPU* /*this*/, NV_ERROR_CONT_ERR_ID, NV_ERROR_CONT_LOCATION, NvU32);  // halified (2 hals) body
    //NV_STATUS(*__gpuWaitForGfwBootComplete__)(struct OBJGPU* /*this*/);  // halified (3 hals) body
    //NvBool(*__gpuIsFlcnRiscvParityError__)(struct OBJGPU* /*this*/, NvU32);  // halified (2 hals) body
    //NvBool(*__gpuGetIsCmpSku__)(struct OBJGPU* /*this*/);  // halified (2 hals) body


    // 103 PDB properties
    NvBool PDB_PROP_GPU_HIGH_SPEED_BRIDGE_CONNECTED;
    NvBool PDB_PROP_GPU_IN_STANDBY;
    NvBool PDB_PROP_GPU_IN_HIBERNATE;
    NvBool PDB_PROP_GPU_IN_PM_CODEPATH;
    NvBool PDB_PROP_GPU_IN_PM_RESUME_CODEPATH;
    NvBool PDB_PROP_GPU_STATE_INITIALIZED;
    NvBool PDB_PROP_GPU_EMULATION;
    NvBool PDB_PROP_GPU_PRIMARY_DEVICE;
    NvBool PDB_PROP_GPU_HYBRID_MGPU;
    NvBool PDB_PROP_GPU_ALTERNATE_TREE_ENABLED;
    NvBool PDB_PROP_GPU_ALTERNATE_TREE_HANDLE_LOCKLESS;
    NvBool PDB_PROP_GPU_3D_CONTROLLER;
    NvBool PDB_PROP_GPU_IS_ALL_INST_IN_SYSMEM;
    NvBool PDB_PROP_GPU_IS_CONNECTED;
    NvBool PDB_PROP_GPU_BROKEN_FB;
    NvBool PDB_PROP_GPU_IN_FULLCHIP_RESET;
    NvBool PDB_PROP_GPU_IN_SECONDARY_BUS_RESET;
    NvBool PDB_PROP_GPU_IN_GC6_RESET;
    NvBool PDB_PROP_GPU_IS_GEMINI;
    NvBool PDB_PROP_GPU_PERSISTENT_SW_STATE;
    NvBool PDB_PROP_GPU_COHERENT_CPU_MAPPING;
    NvBool PDB_PROP_GPU_IS_LOST;
    NvBool PDB_PROP_GPU_IN_TIMEOUT_RECOVERY;
    NvBool PDB_PROP_GPU_ALLOW_PAGE_RETIREMENT;
    NvBool PDB_PROP_GPU_TEGRA_SOC_NVDISPLAY;
    NvBool PDB_PROP_GPU_TEGRA_SOC_IGPU;
    NvBool PDB_PROP_GPU_ATS_SUPPORTED;
    NvBool PDB_PROP_GPU_SECONDARY_BUS_RESET_PENDING;
    NvBool PDB_PROP_GPU_IN_BUGCHECK_CALLBACK_ROUTINE;
    NvBool PDB_PROP_GPU_BUG_3007008_EMULATE_VF_MMU_TLB_INVALIDATE;
    NvBool PDB_PROP_GPU_IS_UEFI;
    NvBool PDB_PROP_GPU_IS_EFI_INIT;
    NvBool PDB_PROP_GPU_ZERO_FB;
    NvBool PDB_PROP_GPU_CAN_OPTIMIZE_COMPUTE_USE_CASE;
    NvBool PDB_PROP_GPU_MIG_SUPPORTED;
    NvBool PDB_PROP_GPU_VC_CAPABILITY_SUPPORTED;
    NvBool PDB_PROP_GPU_RESETLESS_MIG_SUPPORTED;
    NvBool PDB_PROP_GPU_IS_COT_ENABLED;
    NvBool PDB_PROP_GPU_SRIOV_SYSMEM_DIRTY_PAGE_TRACKING_ENABLED;
    NvBool PDB_PROP_GPU_VGPU_OFFLOAD_CAPABLE;
    NvBool PDB_PROP_GPU_SWRL_GRANULAR_LOCKING;
    NvBool PDB_PROP_GPU_IN_SLI_LINK_CODEPATH;
    NvBool PDB_PROP_GPU_IS_PLX_PRESENT;
    NvBool PDB_PROP_GPU_IS_BR03_PRESENT;
    NvBool PDB_PROP_GPU_IS_BR04_PRESENT;
    NvBool PDB_PROP_GPU_BEHIND_BRIDGE;
    NvBool PDB_PROP_GPU_BEHIND_BR03;
    NvBool PDB_PROP_GPU_BEHIND_BR04;
    NvBool PDB_PROP_GPU_UPSTREAM_PORT_L0S_UNSUPPORTED;
    NvBool PDB_PROP_GPU_UPSTREAM_PORT_L1_UNSUPPORTED;
    NvBool PDB_PROP_GPU_UPSTREAM_PORT_L1_POR_SUPPORTED;
    NvBool PDB_PROP_GPU_UPSTREAM_PORT_L1_POR_MOBILE_ONLY;
    NvBool PDB_PROP_GPU_RM_UNLINKED_SLI;
    NvBool PDB_PROP_GPU_SLI_LINK_ACTIVE;
    NvBool PDB_PROP_GPU_ENABLE_REG_ACCESS_IN_LOW_POWER_FOR_SIM_SRTEST;
    NvBool PDB_PROP_GPU_DO_NOT_CHECK_REG_ACCESS_IN_PM_CODEPATH;
    NvBool PDB_PROP_GPU_EXTERNAL_HEAP_CONTROL;
    NvBool PDB_PROP_GPU_IS_MOBILE;
    NvBool PDB_PROP_GPU_IS_EXTERNAL_GPU;
    NvBool PDB_PROP_GPU_RTD3_GC6_SUPPORTED;
    NvBool PDB_PROP_GPU_RTD3_GC6_ACTIVE;
    NvBool PDB_PROP_GPU_FAST_GC6_ACTIVE;
    NvBool PDB_PROP_GPU_UNIX_DYNAMIC_POWER_SUPPORTED;
    NvBool PDB_PROP_GPU_MOVE_CTX_BUFFERS_TO_PMA;
    NvBool PDB_PROP_GPU_LEGACY_GCOFF_SUPPORTED;
    NvBool PDB_PROP_GPU_RTD3_GCOFF_SUPPORTED;
    NvBool PDB_PROP_GPU_GCOFF_STATE_ENTERING;
    NvBool PDB_PROP_GPU_GCOFF_STATE_ENTERED;
    NvBool PDB_PROP_GPU_ACCOUNTING_ON;
    NvBool PDB_PROP_GPU_INACCESSIBLE;
    NvBool PDB_PROP_GPU_NVLINK_SYSMEM;
    NvBool PDB_PROP_GPU_SKIP_CE_MAPPINGS_NO_NVLINK;
    NvBool PDB_PROP_GPU_C2C_SYSMEM;
    NvBool PDB_PROP_GPU_IN_TCC_MODE;
    NvBool PDB_PROP_GPU_SUPPORTS_TDR_EVENT;
    NvBool PDB_PROP_GPU_MSHYBRID_GC6_ACTIVE;
    NvBool PDB_PROP_GPU_VGPU_BIG_PAGE_SIZE_64K;
    NvBool PDB_PROP_GPU_OPTIMIZE_SPARSE_TEXTURE_BY_DEFAULT;
    NvBool PDB_PROP_GPU_ENABLE_IOMMU_SUPPORT;
    NvBool PDB_PROP_GPU_IGNORE_REPLAYABLE_FAULTS;
    NvBool PDB_PROP_GPU_IS_VIRTUALIZATION_MODE_HOST_VGPU;
    NvBool PDB_PROP_GPU_IS_VIRTUALIZATION_MODE_HOST_VSGA;
    NvBool PDB_PROP_GPU_IS_VGPU_HETEROGENEOUS_MODE;
    NvBool PDB_PROP_GPU_NVLINK_P2P_LOOPBACK_DISABLED;
    NvBool PDB_PROP_GPU_NV_USERMODE_ENABLED;
    NvBool PDB_PROP_GPU_IN_FATAL_ERROR;
    NvBool PDB_PROP_GPU_OPTIMUS_GOLD_CFG_SPACE_RESTORE;
    NvBool PDB_PROP_GPU_VGA_ENABLED;
    NvBool PDB_PROP_GPU_IS_MXM_3X;
    NvBool PDB_PROP_GPU_GSYNC_III_ATTACHED;
    NvBool PDB_PROP_GPU_QSYNC_II_ATTACHED;
    NvBool PDB_PROP_GPU_CC_FEATURE_CAPABLE;
    NvBool PDB_PROP_GPU_APM_FEATURE_CAPABLE;
    NvBool PDB_PROP_GPU_EXTENDED_GSP_RM_INITIALIZATION_TIMEOUT_FOR_VGX;
    NvBool PDB_PROP_GPU_SKIP_TABLE_CE_MAP;
    NvBool PDB_PROP_GPU_CHIP_SUPPORTS_RTD3_DEF;
    NvBool PDB_PROP_GPU_IS_SOC_SDM;
    NvBool PDB_PROP_GPU_DISP_PB_REQUIRES_SMMU_BYPASS;
    NvBool PDB_PROP_GPU_FORCE_PERF_BIOS_LEVEL;
    NvBool PDB_PROP_GPU_FASTPATH_SEQ_ENABLED;
    NvBool PDB_PROP_GPU_PREPARING_FULLCHIP_RESET;
    NvBool PDB_PROP_GPU_RECOVERY_DRAIN_P2P_REQUIRED;
    NvBool PDB_PROP_GPU_RUSD_POLLING_SUPPORT_MONOLITHIC;

    // Data members
    NvBool bVideoLinkDisabled;
    void* pGpuFabricProbeInfoKernel;
    NvU32 moduleId;
    NvU8 nvswitchSupport;
    NvBool bIsFlexibleFlaSupported;
    NV2080_CTRL_GPU_RECOVERY_ACTION currentRecoveryAction;
    void* pOsGpuInfo;
    void* pOsRmCaps;
    NvU32 halImpl;
    void* hPci;
    void* engineNonstallIntrEventNotifications[84];
    NvBool bIsSOC;
    NvU32 gpuInstance;
    NvU32 gpuDisabled;
    NvU32 gpuId;
    NvU32 boardId;
    NvU32 deviceInstance;
    NvU32 subdeviceInstance;
    NvS32 numaNodeId;
    NvS32 cpuNumaNodeId;
    _GPU_UUID gpuUuid;
};