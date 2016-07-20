/*
  ==============================================================================
   This file is part of the Springbeats Virtual MIDI Driver (sbvmidi).
   Copyright (c) 2016 - Springbeats SARL.
   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3
   Details of these licenses can be found at: www.gnu.org/licenses
   Sbvmidi is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
   ------------------------------------------------------------------------------
   To release a closed-source product which uses sbvmidi, commercial licenses are
   available: visit www.springbeats.com for more information.
  ==============================================================================
*/
#include "miniport.h"
#include "sbvmidi.h"
#include "log.h"

#define kUseDMusicMiniport 1

#define NUM_CHANNEL_REFERENCE_NAMES 8

static PWSTR PortReferenceNames[NUM_CHANNEL_REFERENCE_NAMES] =
{
    L"AutoTheory1",
    L"AutoTheory2",
    L"AutoTheory3",
    L"AutoTheory4",
    L"AutoTheory5",
    L"AutoTheory6",
    L"AutoTheory7",
    L"AutoTheory8" 
};

/*****************************************************************************
* DriverEntry()
*****************************************************************************
*/
#pragma code_seg("PAGE")
extern "C" NTSTATUS DriverEntry
(
    _In_      PVOID   Context1,   // Context for the class driver.
    _In_      PVOID   Context2    // Context for the class driver.
)
{
    PAGED_CODE();

    LOG(DEBUG, "DriverEntry");

    // Tell the class driver to initialize the driver.
    return PcInitializeAdapterDriver((PDRIVER_OBJECT)Context1,
        (PUNICODE_STRING)Context2,
        (PDRIVER_ADD_DEVICE)AddDevice);
}

/*****************************************************************************
* InstallSubdevice()
*****************************************************************************
*/
#pragma code_seg("PAGE")
NTSTATUS InstallSubdevice
(
    _In_        PVOID          Context1,
    _In_        PVOID          Context2,
    _In_        PWSTR          Name,
    _In_        REFGUID        PortClassId, //DirectMusic or MIDI
    _In_        REFGUID        MiniportClassId, //Our .lib I guess
    _In_opt_    PUNKNOWN       UnknownAdapter,     //not used - null
    _In_        PRESOURCELIST  ResourceList,       //not optional, but can be EMPTY!
    _In_        REFGUID        PortInterfaceId,
    _Out_opt_   PUNKNOWN*      OutPortInterface,   //not used - null
    _Out_opt_   PUNKNOWN*      OutPortUnknown      //not used - null
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(UnknownAdapter);

    LOG(DEBUG, "InstallSubdevice");
    ASSERT(Context1);
    ASSERT(Context2);
    ASSERT(Name);
    ASSERT(ResourceList);

    // Create the port driver object
    PPORT port = NULL;
    NTSTATUS ntStatus = PcNewPort(&port, PortClassId); //Create a directmusic or midi port driver
    if (NT_SUCCESS(ntStatus))
    {
        // Deposit the port somewhere if it's needed.
        if (OutPortInterface)
        {
            //  Failure here doesn't cause the entire routine to fail.
            (void)port->QueryInterface(
                PortInterfaceId,
                (PVOID *)OutPortInterface);
        }

        PMINIPORT miniport;
        // Create the miniport object
        ntStatus = PcNewMiniport(&miniport, MiniportClassId);

        if (NT_SUCCESS(ntStatus))
        {
            // Init the port driver and miniport in one go.
            ntStatus = port->Init((PDEVICE_OBJECT)Context1,
                (PIRP)Context2,
                miniport,
                NULL,   // interruptsync created in miniport.
                ResourceList);

            if (NT_SUCCESS(ntStatus))
            {
                // Register the subdevice (port/miniport combination).
                ntStatus = PcRegisterSubdevice((PDEVICE_OBJECT)Context1,
                    Name,
                    port);
                if (!(NT_SUCCESS(ntStatus)))
                {
                    LOG(ERROR, "StartDevice: PcRegisterSubdevice failed");
                }
            }
            else
            {
                LOG(ERROR, "InstallSubdevice: port->Init failed");
            }

            // We don't need the miniport any more.  Either the port has it,
            // or we've failed, and it should be deleted.
            miniport->Release();
        }
        else
        {
            LOG(ERROR, "InstallSubdevice: PcNewMiniport failed");
        }

        if (NT_SUCCESS(ntStatus))
        {
            // Deposit the port as an unknown if it's needed.
            if (OutPortUnknown)
            {
                //
                //  Failure here doesn't cause the entire routine to fail.
                //
                /*(void)port->QueryInterface
                (
                IID_IUnknown,
                (PVOID *)OutPortUnknown
                );*/
            }
        }
        else
        {
            //
            // Retract previously delivered port interface.
            //
            if (OutPortInterface && (*OutPortInterface))
            {
                (*OutPortInterface)->Release();
                *OutPortInterface = NULL;
            }
        }

        // Release the reference which existed when PcNewPort() gave us the
        // pointer in the first place.  This is the right thing to do
        // regardless of the outcome.
        port->Release();
    }
    else
    {
        LOG(ERROR, "InstallSubdevice: PcNewPort failed");
    }

    return ntStatus;
}

/*****************************************************************************
* CreateMiniportDMusUART()
*****************************************************************************
* Creates a MPU-401 miniport driver for the adapter.  This uses a
* macro from STDUNK.H to do all the work.
*/
#pragma code_seg("PAGE")
NTSTATUS CreateMiniportDMusUART
(
    _Out_     PUNKNOWN *  Unknown,
    _In_      REFCLSID,
    _In_opt_  PUNKNOWN    UnknownOuter,
    _In_      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    LOG(DEBUG, "CreateMiniportDMusUART");
    ASSERT(Unknown);

    STD_CREATE_BODY_(CMiniportDMusUART,
        Unknown,
        UnknownOuter,
        PoolType,
        PMINIPORTDMUS);
}

/*****************************************************************************
* InstallSubdeviceVirtual()
*****************************************************************************
*/
#pragma code_seg("PAGE")
NTSTATUS InstallSubdeviceVirtual
(
    _In_   PVOID           Context1,
    _In_   PVOID           Context2,
    _In_   PWSTR           Name,
    _In_   REFGUID         PortClassId, //DirectMusic or MIDI
    _In_   REFGUID         MiniportClassId, //DirectMusic or MIDI
    _In_   PRESOURCELIST   ResourceList
)
{
    PAGED_CODE();

    PPORT port = NULL;
    NTSTATUS status = STATUS_SUCCESS;

    LOG(DEBUG, "InstallSubdevice");
    LOG(DEBUG, "Creating port driver...");

    if (Name == NULL)
    {
        LOG(ERROR, "Invalid name handle (null value).");
        return STATUS_INVALID_PARAMETER;
    }

    status = PcNewPort(&port, PortClassId); //Create a directmusic or midi port driver
    if (NT_SUCCESS(status))
    {
        //Manually create minport driver
        PMINIPORT miniport = NULL;
        LOG(DEBUG, "Creating miniport driver...");
        status = CreateMiniportDMusUART((PUNKNOWN*)&miniport, MiniportClassId, NULL, NonPagedPool); //Temporal fix, we need a new one!
        if (NT_SUCCESS(status))
        {
            // Init the port driver and miniport in one go.
            status = port->Init((PDEVICE_OBJECT)Context1,
                (PIRP)Context2,
                miniport,
                NULL,   // interruptsync created in miniport.
                ResourceList);
            if (NT_SUCCESS(status))
            {
                LOG(DEBUG, "Miniport-init: success. Registering sub device...")

                    // Register the subdevice (port/miniport combination).
                    status = PcRegisterSubdevice((PDEVICE_OBJECT)Context1, Name, port);
                if (!(NT_SUCCESS(status)))
                {
                    LOG(ERROR, "StartDevice: PcRegisterSubdevice failed");
                }

                LOG(DEBUG, "PcRegisterSubdevice: success.");
            }
            else
            {
                LOG(ERROR, "InstallSubdevice: port->Init failed");
            }

            //
            // We don't need the miniport any more.  Either the port has it,
            // or we've failed, and it should be deleted.
            //
            LOG(DEBUG, "Miniport successfully created!");
            miniport->Release();
        }

        //Release the port in any case if it was created
        port->Release();
    }
    else
    {
        LOG(ERROR, "Port driver creation failed!");
    }

    return status;
}


/*****************************************************************************
* StartDevice()
*****************************************************************************
*/
#pragma code_seg("PAGE")
NTSTATUS StartDevice
(
    _In_      PDEVICE_OBJECT  pDeviceObject,  // Context for the class driver.
    _In_      PIRP            pIrp,           // Context for the class driver.
    _In_      PRESOURCELIST   ResourceList    // List of hardware resources.
)
{
    PAGED_CODE();

    LOG(DEBUG, "StartDevice");

    if (!ResourceList)
    {
        LOG(ERROR, "StartDevice: NULL resource list");
        return STATUS_INVALID_PARAMETER;
    }

    NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;

    for (ULONG i = 0; i < NUM_CHANNEL_REFERENCE_NAMES; i++)
    {
#if (kUseDMusicMiniport)
        ntStatus = InstallSubdeviceVirtual(
            pDeviceObject,
            pIrp,
            PortReferenceNames[i],
            CLSID_PortDMus,
            CLSID_MiniportDriverDMusUART,
            ResourceList);
#else
        ntStatus = InstallSubdeviceVirtual(
            pDeviceObject,
            pIrp,
            PortReferenceNames[i],
            IID_IPortMidi,
            CLSID_MiniportDriverUart,
            ResourceList);
#endif
    }

    return ntStatus;
}

/*****************************************************************************
* AddDevice()
*****************************************************************************
*/
#pragma code_seg("PAGE")
NTSTATUS AddDevice
(
    _In_ PDRIVER_OBJECT   DriverObject,
    _In_ PDEVICE_OBJECT   PhysicalDeviceObject
)
{
    PAGED_CODE();
    
    LOG(DEBUG, "AddDevice");

    // Tell the class driver to add the device.
    //http://msdn.microsoft.com/en-us/library/windows/hardware/ff537683(v=vs.85).aspx
    //NUM_CHANNEL_REFERENCE_NAMES gives the maximun number of subdevices. Adjust above. 
    return PcAddAdapterDevice(DriverObject, PhysicalDeviceObject, StartDevice, NUM_CHANNEL_REFERENCE_NAMES, 0);
}
