/*
 * @brief Main USB service task management
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * Copyright(C) Dean Camera, 2011, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */


#ifndef __USBTASK_H__
#define __USBTASK_H__

	/* Includes: */
		#include "../../../Common/Common.h"
		#include "USBMode.h"		
		#include "USBController.h"
		#include "Events.h"
		#include "StdRequestType.h"
		#include "StdDescriptors.h"

		#if defined(USB_CAN_BE_DEVICE)
			#include "DeviceStandardReq.h"
		#endif

		#if defined(USB_CAN_BE_HOST)
			#include "HostStandardReq.h"
		#endif

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_USB_DRIVER)
			#error Do not include this file directly. Include lpcroot/libraries/LPCUSBlib/Drivers/USB/USB.h instead.
		#endif

	/* Public Interface - May be used in end-application: */
		/* Global Variables: */
			/** Indicates if the USB interface is currently initialized but not necessarily connected to a host
			 *  or device (i.e. if @ref USB_Init() has been run). If this is false, all other library globals related
			 *  to the USB driver are invalid.
			 *
			 *  @note This variable should be treated as read-only in the user application, and never manually
			 *        changed in value.
			 *
			 *  @ingroup Group_USBManagement
			 */
			extern volatile bool USB_IsInitialized;

			/** Structure containing the last received Control request when in Device mode (for use in user-applications
			 *  inside of the @ref EVENT_USB_Device_ControlRequest() event, or for filling up with a control request to 
			 *  issue when in Host mode before calling @ref USB_Host_SendControlRequest().
			 *
			 *  @note The contents of this structure is automatically endian-corrected for the current CPU architecture.
			 *
			 *  @ingroup Group_USBManagement
			 */
			 extern USB_Request_Header_t USB_ControlRequest;

			#if defined(USB_CAN_BE_HOST) || defined(__DOXYGEN__)
				#if !defined(HOST_STATE_AS_GPIOR) || defined(__DOXYGEN__)
					/** Indicates the current host state machine state. When in host mode, this indicates the state
					 *  via one of the values of the @ref USB_Host_States_t enum values.
					 *
					 *  This value should not be altered by the user application as it is handled automatically by the
					 *  library.
					 *
					 *  To reduce program size and speed up checks of this global on the LPC architecture, it can be
					 *  placed into one of the LPC's \c GPIOR hardware registers instead of RAM by defining the
					 *  \c HOST_STATE_AS_GPIOR token to a value between 0 and 2 in the project makefile and passing it to
					 *  the compiler via the -D switch. When defined, the corresponding GPIOR register should not be used
					 *  in the user application except implicitly via the library APIs.
					 *
					 *  @note This global is only present if the user application can be a USB host.
					 *
					 *  @see @ref USB_Host_States_t for a list of possible device states.
					 *
					 *  @ingroup Group_Host
					 */
					extern volatile uint8_t USB_HostState[MAX_USB_CORE];
				#else
					#define _GET_HOST_GPIOR_NAME2(y) GPIOR ## y
					#define _GET_HOST_GPIOR_NAME(x)  _GET_HOST_GPIOR_NAME2(x)
					#define USB_HostState            _GET_HOST_GPIOR_NAME(HOST_STATE_AS_GPIOR)
				#endif
			#endif

			#if defined(USB_CAN_BE_DEVICE) || defined(__DOXYGEN__)
				#if !defined(DEVICE_STATE_AS_GPIOR) || defined(__DOXYGEN__)
					/** Indicates the current device state machine state. When in device mode, this indicates the state
					 *  via one of the values of the @ref USB_Device_States_t enum values.
					 *
					 *  This value should not be altered by the user application as it is handled automatically by the
					 *  library. The only exception to this rule is if the NO_LIMITED_CONTROLLER_CONNECT token is used
					 *  (see @ref EVENT_USB_Device_Connect() and @ref EVENT_USB_Device_Disconnect() events).
					 *
					 *  To reduce program size and speed up checks of this global on the LPC architecture, it can be
					 *  placed into one of the LPC's \c GPIOR hardware registers instead of RAM by defining the
					 *  \c DEVICE_STATE_AS_GPIOR token to a value between 0 and 2 in the project makefile and passing it to
					 *  the compiler via the -D switch. When defined, the corresponding GPIOR register should not be used
					 *  in the user application except implicitly via the library APIs.
					 *
					 *  @note This global is only present if the user application can be a USB device.
					 *        \n\n
					 *
					 *  @note This variable should be treated as read-only in the user application, and never manually
					 *        changed in value except in the circumstances outlined above.
					 *
					 *  @see @ref USB_Device_States_t for a list of possible device states.
					 *
					 *  @ingroup Group_Device
					 */
					extern volatile uint8_t USB_DeviceState[];
				#else
					#define _GET_DEVICE_GPIOR_NAME2(y) GPIOR ## y
					#define _GET_DEVICE_GPIOR_NAME(x)  _GET_DEVICE_GPIOR_NAME2(x)
					#define USB_DeviceState[0]            _GET_DEVICE_GPIOR_NAME(DEVICE_STATE_AS_GPIOR)
					#define USB_DeviceState[1]            _GET_DEVICE_GPIOR_NAME(DEVICE_STATE_AS_GPIOR)
				#endif
			#endif

		/* Function Prototypes: */
			/** This is the main USB management task. The USB driver requires this task to be executed
			 *  continuously when the USB system is active (device attached in host mode, or attached to a host
			 *  in device mode) in order to manage USB communications. This task may be executed inside an RTOS,
			 *  fast timer ISR or the main user application loop.
			 *
			 *  The USB task must be serviced within 30ms while in device mode, or within 1ms while in host mode.
			 *  The task may be serviced at all times, or (for minimum CPU consumption):
			 *
			 *    - In device mode, it may be disabled at start-up, enabled on the firing of the @ref EVENT_USB_Device_Connect()
			 *      event and disabled again on the firing of the @ref EVENT_USB_Device_Disconnect() event.
			 *
			 *    - In host mode, it may be disabled at start-up, enabled on the firing of the @ref EVENT_USB_Host_DeviceAttached()
			 *      event and disabled again on the firing of the @ref EVENT_USB_Host_DeviceEnumerationComplete() or
			 *      @ref EVENT_USB_Host_DeviceEnumerationFailed() events.
			 *
			 *  If in device mode (only), the control endpoint can instead be managed via interrupts entirely by the library
			 *  by defining the INTERRUPT_CONTROL_ENDPOINT token and passing it to the compiler via the -D switch.
			 *
			 *  @see @ref Group_Events for more information on the USB events.
			 *
			 *  @ingroup Group_USBManagement
			 */
			void USB_USBTask(uint8_t corenum, uint8_t mode);

	/* Private Interface - For use in library only: */
	#if !defined(__DOXYGEN__)
		/* Function Prototypes: */
			#if defined(__INCLUDE_FROM_USBTASK_C)
				#if defined(USB_CAN_BE_HOST)
					static void USB_HostTask(uint8_t corenum);
				#endif

				#if defined(USB_CAN_BE_DEVICE)
					static void USB_DeviceTask(uint8_t corenum);
				#endif
			#endif

		/* Macros: */
			#define HOST_TASK_NONBLOCK_WAIT(CoreID, Duration, NextState) MACROS{ USB_HostState[(CoreID)]   = HOST_STATE_WaitForDevice; \
			                                                             WaitMSRemaining = (Duration);               \
			                                                             PostWaitState   = (NextState);              }MACROE
	#endif

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif

