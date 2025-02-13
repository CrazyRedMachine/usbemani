/*
             LUFA Library
     Copyright (C) Dean Camera, 2014.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2014  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  USB Device Descriptors, for library use when in USB device mode. Descriptors are special
 *  computer-readable structures which the host requests upon device enumeration, to determine
 *  the device's capabilities and functions.
 */

#include "Descriptors.h"
#include "Config.h"
#include "Rotary.h"

/** HID class report descriptor. This is a special descriptor constructed with values from the
 *  USBIF HID class specification to describe the reports and capabilities of the HID device. This
 *  descriptor is parsed by the host and its contents used to determine what data (and in what encoding)
 *  the device will send, and what it may be sent back from the host. Refer to the HID specification for
 *  more details on HID report descriptors.
 */

// We need access some of our device settings. We also need access to the rotary encoder tooth count.
Settings_Device_t* Device;
Settings_Rotary_t* Rotary;

USB_Descriptor_HIDReport_Datatype_t GenericReport[] =
{
	HID_RI_USAGE_PAGE(8, 0x01), /* Generic Desktop */
	HID_RI_USAGE(8, 0x04), /* Joystick */
	HID_RI_COLLECTION(8, 0x01), /* Application */
	    HID_RI_USAGE(8, 0x01), /* Pointer */
	    HID_RI_COLLECTION(8, 0x00), /* Physical */
			// X and Y axes.
	        HID_RI_USAGE(8, 0x30),
	        HID_RI_USAGE(8, 0x31),
	        HID_RI_LOGICAL_MINIMUM(8, -100),
	        HID_RI_LOGICAL_MAXIMUM(8, 100),
	        HID_RI_REPORT_COUNT(8, 0x02),
	        HID_RI_REPORT_SIZE(8, 0x08),
	        HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
	        // Dial and slider axes.
	        // Per Tau, we need to be able to alter the maximum value for the dial and slider, so Bemanitools can do scaling.
	        HID_RI_USAGE(8, 0x37), 
	        HID_RI_USAGE(8, 0x36),
	        HID_RI_LOGICAL_MINIMUM(16, 0),
			// This value is altered in later code.
	        HID_RI_LOGICAL_MAXIMUM(16, 255), // [32] should be the editable value. [33] is the 8 bits over 256.
	        HID_RI_REPORT_COUNT(8, 0x02),
	        HID_RI_REPORT_SIZE(8, 0x08),
	        HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_WRAP | HID_IOF_NO_PREFERRED_STATE),	    
	    HID_RI_END_COLLECTION(0),
	    // Buttons.
	    HID_RI_USAGE_PAGE(8, 0x09),
	    HID_RI_USAGE_MINIMUM(8, 1),
	    HID_RI_USAGE_MAXIMUM(8, 11),
	    HID_RI_LOGICAL_MINIMUM(8, 0x00),
	    HID_RI_LOGICAL_MAXIMUM(8, 0x01),
	    HID_RI_REPORT_SIZE(8, 1),
	    HID_RI_REPORT_COUNT(8, 11),
	    HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
	   	HID_RI_REPORT_SIZE(8, 0x05),
	    HID_RI_REPORT_COUNT(8, 0x01),
	    HID_RI_INPUT(8, HID_IOF_CONSTANT),
		// LED Output reports.
		// LEDs are broken up a number of ways. We have an ordinal...
	    HID_RI_USAGE_PAGE(8, 0x0A), 
	    // ...an instance (this is Instance 1)...
	    HID_RI_USAGE(8, 1),
	    // ...a logical collection...
	    HID_RI_COLLECTION(8, 0x02),
	    	// And an LED, a Generic Indicator.
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
	    // We have a total of 16 LEDs, so 16 instances. This will be a while...
	    // There is most likely a much better way to handle this programatically. However, I have not, and most likely will not, have an opportunity to revisit this.
	    HID_RI_USAGE(8, 2),
	    HID_RI_COLLECTION(8, 0x02),
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
	    HID_RI_USAGE(8, 3),
	    HID_RI_COLLECTION(8, 0x02),
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
	    HID_RI_USAGE(8, 4),
	    HID_RI_COLLECTION(8, 0x02),
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
	    HID_RI_USAGE(8, 5),
	    HID_RI_COLLECTION(8, 0x02),
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
	    HID_RI_USAGE(8, 6),
	    HID_RI_COLLECTION(8, 0x02),
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
	    HID_RI_USAGE(8, 7),
	    HID_RI_COLLECTION(8, 0x02),
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
	    HID_RI_USAGE(8, 8),
	    HID_RI_COLLECTION(8, 0x02),
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
	    HID_RI_USAGE(8, 9),
	    HID_RI_COLLECTION(8, 0x02),
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
		HID_RI_USAGE(8, 10),
	    HID_RI_COLLECTION(8, 0x02),
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
		HID_RI_USAGE(8, 11),
	    HID_RI_COLLECTION(8, 0x02),
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
		HID_RI_USAGE(8, 12),
	    HID_RI_COLLECTION(8, 0x02),
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
		HID_RI_USAGE(8, 13),
	    HID_RI_COLLECTION(8, 0x02),
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
		HID_RI_USAGE(8, 14),
	    HID_RI_COLLECTION(8, 0x02),
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
		HID_RI_USAGE(8, 15),
	    HID_RI_COLLECTION(8, 0x02),
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
		HID_RI_USAGE(8, 16),
	    HID_RI_COLLECTION(8, 0x02),
	    	HID_RI_USAGE_PAGE(8, 0x08),
	    	HID_RI_USAGE(8, 0x4B),
	    	HID_RI_REPORT_SIZE(8, 1),
	    	HID_RI_REPORT_COUNT(8, 1),
	    	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),	
	    HID_RI_END_COLLECTION(0),
	    // An empty 16 bits. We hold our configuration report here.
	    HID_RI_REPORT_SIZE(8, 8),
		HID_RI_REPORT_COUNT(8, 2),
		HID_RI_OUTPUT(8, HID_IOF_CONSTANT),
	HID_RI_END_COLLECTION(0),
};

/** Device descriptor structure. This descriptor, located in FLASH memory, describes the overall
 *  device characteristics, including the supported USB version, control endpoint size and the
 *  number of device configurations. The descriptor is read out by the USB host when the enumeration
 *  process begins.
 */
const USB_Descriptor_Device_t PROGMEM DeviceDescriptor =
{
	.Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

	.USBSpecification       = VERSION_BCD(1,1,0),
	.Class                  = USB_CSCP_NoDeviceClass,
	.SubClass               = USB_CSCP_NoDeviceSubclass,
	.Protocol               = USB_CSCP_NoDeviceProtocol,

	.Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

	.VendorID               = 0x1ccf,
	.ProductID              = 0x8048,
	.ReleaseNumber          = VERSION_BCD(0,0,1),

	.ManufacturerStrIndex   = STRING_ID_Manufacturer,
	.ProductStrIndex        = STRING_ID_Product,
	.SerialNumStrIndex      = NO_DESCRIPTOR,

	.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

/** Configuration descriptor structure. This descriptor, located in FLASH memory, describes the usage
 *  of the device in one of its supported configurations, including information about any device interfaces
 *  and endpoints. The descriptor is read out by the USB host during the enumeration process when selecting
 *  a configuration so that the host may correctly communicate with the USB device.
 */
const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor =
{
	.Config =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

			.TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
			.TotalInterfaces        = 1,

			.ConfigurationNumber    = 1,
			.ConfigurationStrIndex  = NO_DESCRIPTOR,

			.ConfigAttributes       = (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED),

			.MaxPowerConsumption    = USB_CONFIG_POWER_MA(500)
		},

	.HID_Interface =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

			.InterfaceNumber        = INTERFACE_ID_GenericHID,
			.AlternateSetting       = 0x00,

			.TotalEndpoints         = 2,

			.Class                  = HID_CSCP_HIDClass,
			.SubClass               = HID_CSCP_NonBootSubclass,
			.Protocol               = HID_CSCP_NonBootProtocol,

			.InterfaceStrIndex      = NO_DESCRIPTOR
		},

	.HID_GenericHID =
		{
			.Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},

			.HIDSpec                = VERSION_BCD(1,1,1),
			.CountryCode            = 0x00,
			.TotalReportDescriptors = 1,
			.HIDReportType          = HID_DTYPE_Report,
			.HIDReportLength        = sizeof(GenericReport)
		},

	.HID_ReportINEndpoint =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

			.EndpointAddress        = GENERIC_IN_EPADDR,
			.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			.EndpointSize           = GENERIC_EPSIZE,
			.PollingIntervalMS      = 0x01
		},

	.HID_ReportOUTEndpoint =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

			.EndpointAddress        = GENERIC_OUT_EPADDR,
			.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			.EndpointSize           = GENERIC_EPSIZE,
			.PollingIntervalMS      = 0x01
		}
};

/** Language descriptor structure. This descriptor, located in FLASH memory, is returned when the host requests
 *  the string descriptor with index 0 (the first index). It is actually an array of 16-bit integers, which indicate
 *  via the language ID table available at USB.org what languages the device supports for its string descriptors.
 */
const USB_Descriptor_String_t PROGMEM LanguageString = USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);

/** Manufacturer descriptor string. This is a Unicode string containing the manufacturer's details in human readable
 *  form, and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
const USB_Descriptor_String_t PROGMEM ManufacturerString = USB_STRING_DESCRIPTOR(L"jaesond/@progmem");

/** Product descriptor string. This is a Unicode string containing the product's details in human readable form,
 *  and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
const USB_Descriptor_String_t PROGMEM ProductString  	   = USB_STRING_DESCRIPTOR(L"USBemani v2 (Home)");
const USB_Descriptor_String_t PROGMEM ProductString1P      = USB_STRING_DESCRIPTOR(L"USBemani v2 (1P)");
const USB_Descriptor_String_t PROGMEM ProductString2P      = USB_STRING_DESCRIPTOR(L"USBemani v2 (2P)");
      USB_Descriptor_String_t         ProductStringCustom  = USB_STRING_DESCRIPTOR(L"Custom String Goes Here!");

/** This function is called by the library when in device mode, and must be overridden (see library "USB Descriptors"
 *  documentation) by the application code so that the address and size of a requested descriptor can be given
 *  to the USB library. When the device receives a Get Descriptor request on the control endpoint, this function
 *  is called so that the descriptor details can be passed back and the appropriate descriptor sent back to the
 *  USB host.
 */
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    const void** const DescriptorAddress
									#if defined(HAS_MULTIPLE_DESCRIPTOR_ADDRESS_SPACES)
									, uint8_t* const DescriptorMemorySpace
									#endif
									)
{
	const uint8_t  DescriptorType   = (wValue >> 8);
	const uint8_t  DescriptorNumber = (wValue & 0xFF);

	const void* Address = NULL;
	uint16_t    Size    = NO_DESCRIPTOR;

	Config_AddressDevice(&Device);
	Config_AddressRotary(&Rotary);

	// Injection point for altering the tooth count.
	// We won't alter this if the rotary tooth count is 0. This is to prevent people from trying to fake the code out, or possible corruption.
	GenericReport[32] = (Rotary->RotaryPPR ? (int8_t)(Rotary->RotaryPPR - 1) : GenericReport[32]);

	// All descriptors should be stored in progmem unless otherwise stated.
	#if defined(HAS_MULTIPLE_DESCRIPTOR_ADDRESS_SPACES)
	*DescriptorMemorySpace = MEMSPACE_FLASH;
	#endif

	switch (DescriptorType)
	{
		case DTYPE_Device:
			// We need to set the product ID based on the kind of board we're using.
			Address = &DeviceDescriptor;
			Size    = sizeof(USB_Descriptor_Device_t);
			break;
		case DTYPE_Configuration:
			Address = &ConfigurationDescriptor;
			Size    = sizeof(USB_Descriptor_Configuration_t);
			break;
		case DTYPE_String:
			switch (DescriptorNumber)
			{
				case STRING_ID_Language:
					Address = &LanguageString;
					Size    = pgm_read_byte(&LanguageString.Header.Size);
					break;
				case STRING_ID_Manufacturer:
					Address = &ManufacturerString;
					Size    = pgm_read_byte(&ManufacturerString.Header.Size);
					break;
				case STRING_ID_Product:
					switch(Device->DeviceName) {
						case N_P1:
							Address = &ProductString1P;
							Size    = pgm_read_byte(&ProductString1P.Header.Size);
							break;
						case N_P2:
							Address = &ProductString2P;
							Size    = pgm_read_byte(&ProductString2P.Header.Size);
							break;
						case N_Custom:
							#if defined(HAS_MULTIPLE_DESCRIPTOR_ADDRESS_SPACES)
							*DescriptorMemorySpace = MEMSPACE_RAM;
							#endif
							// For custom names, we're iterating through 25 characters (24 plus nul-terminator).
							for (int i = 0; i < 25; i++) {
								ProductStringCustom.UnicodeString[i] = Device->CustomName[i];
							}

							Address = &ProductStringCustom;
							Size    =  ProductStringCustom.Header.Size;
							break;
						default:
							Address = &ProductString;
							Size    = pgm_read_byte(&ProductString.Header.Size);
							break;
					}
					break;
			}
			break;
		case HID_DTYPE_HID:
			Address = &ConfigurationDescriptor.HID_GenericHID;
			Size    = sizeof(USB_HID_Descriptor_HID_t);
			break;
		case HID_DTYPE_Report:
			#if defined(HAS_MULTIPLE_DESCRIPTOR_ADDRESS_SPACES)
			*DescriptorMemorySpace = MEMSPACE_RAM;
			#endif

			Address = &GenericReport;
			Size    = sizeof(GenericReport);
			break;
	}

	*DescriptorAddress = Address;
	return Size;
}

