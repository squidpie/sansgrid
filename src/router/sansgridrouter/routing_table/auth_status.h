/* Device Authentication status
 *
 * Copyright (C) 2013 SansGrid
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 */
#ifndef __SG_AUTH_STATUS_H__
#define __SG_AUTH_STATUS_H__
/** \file */

#include <stdint.h>
typedef struct DeviceAuth DeviceAuth;

/// Device Authentication Types
enum DeviceAuthEnum {
	/// Bad Authentication Type
	DEV_AUTH_ERROR,
	/// Loose Authentication
	DEV_AUTH_LOOSE,
	/// Filtered Authentication
	DEV_AUTH_FILTERED,
	/// Strict Authentication
	DEV_AUTH_STRICT,
};

int deviceAuthEnableLoosely(DeviceAuth *dev_auth);
int deviceAuthEnableFiltered(DeviceAuth *dev_auth);
int deviceAuthEnable(DeviceAuth *dev_auth);
int deviceAuthIsEnabled(DeviceAuth *dev_auth);
DeviceAuth *deviceAuthInit(int strictness);
void deviceAuthDestroy(DeviceAuth *dev_auth);
int deviceAuthIsGeneralPayloadTypeValid(DeviceAuth *dev_auth, uint32_t gdt);
int deviceAuthIsSGPayloadTypeValid(DeviceAuth *dev_auth, uint8_t dt);
int deviceAuthSetNextGeneralPayload(DeviceAuth *dev_auth, uint32_t gdt);
enum SansgridDeviceStatusEnum deviceAuthGetNextGeneralPayload(DeviceAuth *dev_auth);
enum SansgridDeviceStatusEnum deviceAuthGetCurrentGeneralPayload(DeviceAuth *dev_auth);
int32_t deviceAuthSetCurrentGeneralPayload(DeviceAuth *dev_auth, enum SansgridDeviceStatusEnum gdt);




#endif // __SG_AUTH_STATUS_H__

// vim: ft=c ts=4 noet sw=4:

