/* Payload Specifications
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

#ifndef __SG_PAYLOADS_H__
#define __SG_PAYLOADS_H__
/** \file */

#include <stdint.h>


/// Size of an IP address, in bytes
#define IP_SIZE 16
/// Size of a Sansgrid Payload, in bytes
#define PAYLOAD_SIZE 81


/**
 * \brief Converts between Sansgrid Payload and raw byte array
 *
 * Given a Sansgrid Payload, this creates a union 
 * to convert between a formatted structure and raw byte data.
 *
 * For instance, to create a union for a SansgridEyeball payload, call
 * \code {.c}
 * SANSGRID_UNION(SansgridEyeball, SGEBU) sg_eyeball_un;
 * \endcode
 * To convert between a raw payload and formatted code, call
 * \code {.c}
 * SansgridEyeball *sg_eyeball;
 * uint8_t data[PAYLOAD_SIZE];
 * sg_eyeball_un.serialdata = data;
 * sg_eyeball = sg_eyeball_un.formdata;
 * \endcode
 */
#define SANSGRID_UNION(type, name) union name { \
	type *formdata; \
	uint8_t *serialdata; \
}



/**
 * \brief Router/Radio Initialization
 *
 * A Hatch is sent to the radio from the router at startup
 * to set the subnet mask and set the radio as a "router radio."
 *
 * Sent from a router to a radio.
 *
 * Valid Sansgrid Hatch Datatype(s): SG_HATCH
 */
typedef struct SansgridHatching {
	/**
	 * \brief Sansgrid Hatch datatype (0xF1)
	 *
	 * Valid Sansgrid Datatype(s): SG_HATCH
	 */
	uint8_t datatype;
	/// Netmask address
	uint8_t ip[16];
	uint8_t padding[64];
} SansgridHatching;



/**
 * \brief ESSID Broadcast
 *
 * A Fly is broadcast with the ESSID
 * to indicate the presence of a network.
 * 
 * Sent from a router to a radio.
 *
 * Valid Sansgrid Fly Datatype(s): SG_FLY
 */
typedef struct SansgridFly {
	/**
	 * \brief Sansgrid Fly datatype (0xF0)
	 *
	 * Valid Sansgrid Fly Datatype(s): SG_FLY
	 */
	uint8_t datatype;
	/// ESSID
	char network_name[80];
} SansgridFly;




/**
 * \brief Device wishes to connect to network
 *
 * An Eyeball is sent from a sensor
 * to indicate it wishes to connect
 * to the network.
 *
 * Sent from a sensor to the server.
 *
 * Valid Sansgrid Eyeball Datatype(s): SG_EYEBALL
 */
typedef struct SansgridEyeball {
	/**
	 * \brief Sansgrid Eyaball datatype (0x00)
	 *
	 * Valid Sansgrid Eyeball Datatype(s): SG_EYEBALL
	 */
	uint8_t datatype;
	/// Device Manufacturing ID
	uint8_t manid[4];
	/// Device Model Number
	uint8_t modnum[4];
	/// Device Serial Number
	uint8_t serial_number[8];
	uint8_t profile;

	/**
	 * Device Mating Intent (0/1)
	 *
	 * \param 00 - Standard mode
	 * \param 01 - Sensor Ready to mate
	*/
	uint8_t mode;
	uint8_t padding[62];
} SansgridEyeball;




/**
 * \brief Server responds to sensor with recognition status
 *
 * A Sansgrid Peck is sent from the server to inform the sensor
 * of whether it was recognized or not. The recognition determines
 * whether the next packet will be a SansgridSing, or a SansgridSquawk.
 *
 * Sent from the server to a sensor
 *
 * Valid Sansgrid Peck Datatype(s): SG_PECK
 */
typedef struct SansgridPeck {
	/**
	 * \brief Sansgrid Peck datatype (0x01)
	 *
	 * Valid Sansgrid Datatype(s): SG_PECK
	 */
	uint8_t datatype;
	/// Router's IP address
	uint8_t router_ip[IP_SIZE];
	/// Device's assigned IP address
	uint8_t assigned_ip[IP_SIZE];
	/// Server Unique Identifier
	uint8_t server_id[16];
	/**
	 * \brief Server Recognition (0/1/2/3)
	 * \param	0x00	- Recognized
	 * \param	0x01	- Not Recognized, server will mate
	 * \param	0x02	- Not Recognized, server refuses mate
	 * \param	0x03	- Not Recognized, sensor refuses mate
	 */
	uint8_t recognition;
	/// Device's Manufacturing ID echoed back
	uint8_t manid[4];
	/// Device's Model Number echoed back
	uint8_t modnum[4];
	/// Device's Serial Number echoed back
	uint8_t serial_number[8];
	uint8_t padding[15];
} SansgridPeck;




/**
 * \brief Server sends its public key
 *
 * The server sends its public key to confirm its identity
 *
 * Sent from the server to a sensor
 *
 * Valid Sansgrid Sing Datatypes:
 * \param	SG_SING_WITH_KEY		ACK, valid public key contained
 * \param	SG_SING_WITHOUT_KEY		ACK, no public key contained
 */
typedef struct SansgridSing {
	/**
	 * \brief Sansgrid Sing datatype (0x02, 0x03)
	 *
	 * Valid Sansgrid Datatypes:
	 * \param	SG_SING_WITH_KEY		ACK, valid public key contained
	 * \param	SG_SING_WITHOUT_KEY		ACK, no public key contained
	 */
	uint8_t datatype;
	/// Server Public Key
	uint8_t pubkey[80];
} SansgridSing;



/**
 * \brief Sensor sends its public key
 *
 * The sensor sends its public key to confirm its identity
 *
 * Sent from a sensor to the server
 *
 * Valid Sansgrid Mock Datatypes:
 * \param	SG_MOCK_WITH_KEY		ACK, valid public key contained
 * \param	SG_MOCK_WITHOUT_KEY		ACK, no public key contained
 */
typedef struct SansgridMock {
	/**
	 * \brief Sansgrid Mock datatype (0x07, 0x08)
	 *
	 * Valid Sansgrid Datatypes:
	 * \param	SG_MOCK_WITH_KEY		ACK, valid public key contained
	 * \param	SG_MOCK_WITHOUT_KEY		ACK, no public key contained
	 */
	uint8_t datatype;
	/// Sensor Public Key
	uint8_t pubkey[80];
} SansgridMock;



/**
 * \brief Sensor sends its capabilities
 *
 * The sensor sends its signals to the server
 *
 * Sent from a sensor to the server
 *
 * Valid Sansgrid Peacock Datatypes:
 * \param SG_PEACOCK	Sending capabilities
 */
typedef struct SansgridPeacock {
	/**
	 * \brief Sansgrid Peacock datatype (0x0C)
	 *
	 * Valid Sansgrid Peacock Datatypes:
	 * \param	SG_PEACOCK	Sending capabilities
	 */
	uint8_t		datatype;
	// I/O(A) capabilities
	/// Signal A identifier
	uint8_t		IO_A_id;
	/// Signal A class
	uint8_t		IO_A_class;
	/// Signal A direction
	uint8_t		IO_A_direc;
	/// Signal A label
	char 		IO_A_label[30];
	/// Signal A units
	char 		IO_A_units[6];

	// I/O(B) capabilities
	/// Signal B identifier
	uint8_t		IO_B_id;
	/// Signal B class
	uint8_t		IO_B_class;
	/// Signal B direction
	uint8_t		IO_B_direc;
	/// Signal B label
	char 		IO_B_label[30];
	/// Signal B units
	char 	 	IO_B_units[6];

	/**
	 * \brief If more I/O types are coming
	 *
	 * \param	0	No additional IO needed
	 * \param	1	Additional IO needed. Sending another Peacock
	 */
	uint8_t 	additional_IO_needed;

	uint8_t 	padding;
} SansgridPeacock;



/**
 * \brief Sensor has been authenticated
 *
 * The server informs the sensor that it can send data
 *
 * Sent from the server to a sensor
 *
 * Valid Sansgrid Nest Datatypes:
 * \param SG_NEST	Device has successfully authenticated
 */
typedef struct SansgridNest {
	/**
	 * \brief Sansgrid Nest datatype (0x10)
	 *
	 * Valid Sansgrid Nest Datatypes:
	 * \param SG_NEST	Device has successfully authenticated
	 */
	uint8_t datatype;
	uint8_t padding[80];
} SansgridNest;



/**
 * \brief Sensor and Server check identities
 *
 * The sensor checks a server's public key against a stored copy. \n
 * The server checks a sensor's public key against a stored copy.
 *
 * Sent from the server to a sensor. \n
 * Sent from a sensor to the server
 *
 * Valid Sansgrid Squawk Datatypes:
 * \param SG_SQUAWK_SERVER_CHALLENGE_SENSOR		Server Challenges sensor
 * \param SG_SQUAWK_SERVER_NOCHALLENGE_SENSOR	Server doesn't challenge sensor
 * \param SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE	Sensor doesn't require challenge
 * \param SG_SQUAWK_SENSOR_RESPOND_REQUIRE_CHALLENGE Sensor requires challenge
 * \param SG_SQUAWK_SENSOR_CHALLENGE_SERVER Sensor challenges server
 * \param SG_SQUAWK_SERVER_DENY_SENSOR Server denies sensor
 * \param SG_SQUAWK_SERVER_RESPOND Server's response to the sensor
 * \param SG_SQUAWK_SENSOR_ACCEPT_RESPONSE Sensor accepts server response
 */
typedef struct SansgridSquawk {
	/**
	 * \brief Sansgrid Squawk Datatype (0x11, 0x15, 0x16, 0x17, 0x1B, 0x1C, 0x1D)
	 *
	 * Valid Sansgrid Squawk Datatypes:
	 * \param SG_SQUAWK_SERVER_CHALLENGE_SENSOR		Server Challenges sensor
	 * \param SG_SQUAWK_SERVER_NOCHALLENGE_SENSOR	Server doesn't challenge sensor
	 * \param SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE	Sensor doesn't require challenge
	 * \param SG_SQUAWK_SENSOR_RESPOND_REQUIRE_CHALLENGE Sensor requires challenge
	 * \param SG_SQUAWK_SENSOR_CHALLENGE_SERVER Sensor challenges server
	 * \param SG_SQUAWK_SERVER_DENY_SENSOR Server denies sensor
	 * \param SG_SQUAWK_SERVER_RESPOND Server's response to the sensor
	 * \param SG_SQUAWK_SENSOR_ACCEPT_RESPONSE Sensor accepts server response
	 */
	uint8_t datatype;
	/// Key, if applicable
	uint8_t data[80];
} SansgridSquawk;




/** 
 * \brief Pings from router or from sensor
 *
 * The router sends a ping to the sensor. \n
 * The sensor responds back with a ping response.
 *
 * Sent from the router to a sensor radio. \n
 * Sent from a sensor radio to the router.
 *
 * Valid Sansgrid Heartbeat Datatypes:
 * \param SG_HEARTBEAT_ROUTER_TO_SENSOR		router pings sensor
 * \param SG_HEARTBEAT_SENSOR_TO_ROUTER		sensor responds to router
 */
typedef struct SansgridHeartbeat {
	/**
	 * \brief Sansgrid Heartbeat Datatype (0x1E, 0x1F)
	 *
	 * Valid Sansgrid Heartbeat Datatypes:
	 * \param SG_HEARTBEAT_ROUTER_TO_SENSOR		router pings sensor
	 * \param SG_HEARTBEAT_SENSOR_TO_ROUTER		sensor responds to router
	 */
	uint8_t datatype;
	uint8_t padding[80];
} SansgridHeartbeat;




/**
 * \brief Data is sent from sensor or from server
 *
 * A sensor sends data to the server. \n
 * The server sends a command to a sensor.
 *
 * Sent from a sensor to the server. \n
 * Sent from the server to a sensor.
 *
 * Valid Sansgrid Chirp Datatypes:
 * \param SG_CHIRP_COMMAND_SERVER_TO_SENSOR	Server sends command to sensor
 * \param SG_CHIRP_DATA_SENSOR_TO_SERVER	Sensor sends data to server
 * \param SG_CHIRP_NETWORK_DISCONNECTS_SENSOR	Network is disconnecting sensor
 * \param SG_CHIRP_SENSOR_DISCONNECT		Sensor is disconnecting from network
 */
typedef struct SansgridChirp {
	/**
	 * \brief Sansgrid Chirp Datatype (0x20, 0x21, 0x25, 0x26)
	 *
	 * Valid Sansgrid Chirp Datatypes:
	 * \param SG_CHIRP_COMMAND_SERVER_TO_SENSOR	Server sends command to sensor
	 * \param SG_CHIRP_DATA_SENSOR_TO_SERVER	Sensor sends data to server
	 * \param SG_CHIRP_NETWORK_DISCONNECTS_SENSOR	Network is disconnecting sensor
	 * \param SG_CHIRP_SENSOR_DISCONNECT		Sensor is disconnecting from network
	 */
	uint8_t datatype;
	/// Signal Identifier
	uint8_t sid;
	/// Data being sent
	uint8_t data[79];
} SansgridChirp;




/**
 * \brief Sansgrid Payload Types
 *
 * Sent in the datatype field of Sansgrid Payloads
 */
enum SansgridDataTypeEnum {
	/// Initialization
	SG_HATCH = 0xF1,
	/// ESSID Broadcast
	SG_FLY = 0xF0,
	/// Eyeballing
	SG_EYEBALL = 0xeb,
	/// Pecking
	SG_PECK = 0x01,
	/// Singing with key
	SG_SING_WITH_KEY = 0x02,
	/// Singing without key
	SG_SING_WITHOUT_KEY = 0x03,
	/// Mocking with key
	SG_MOCK_WITH_KEY = 0x07,
	/// Mocking without key
	SG_MOCK_WITHOUT_KEY = 0x08,
	/// Peacocking
	SG_PEACOCK = 0x0C,
	/// Nesting
	SG_NEST = 0x10,
	/// Squawking: Server Challenges Sensor
	SG_SQUAWK_SERVER_CHALLENGE_SENSOR = 0x11,
	/// Squawking: Server Doesn't Challenge Sensor
	SG_SQUAWK_SERVER_NOCHALLENGE_SENSOR = 0x12,
	/// Squawking: Sensor Responds, doesn't require challenge
	SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE = 0x15,
	/// Squawking: Sensor Responds, requires challenge
	SG_SQUAWK_SENSOR_RESPOND_REQUIRE_CHALLENGE = 0x16,
	/// Squawking: Sensor Challenges Server
	SG_SQUAWK_SENSOR_CHALLENGE_SERVER = 0x17,
	/// Squawking: Server Denies Sensor
	SG_SQUAWK_SERVER_DENY_SENSOR = 0x1B,
	/// Squawking: Server Responds to Sensor Challenge
	SG_SQUAWK_SERVER_RESPOND = 0x1C,
	/// Squawking: Sensor Accepts Server Response
	SG_SQUAWK_SENSOR_ACCEPT_RESPONSE = 0x1D,
	/// Heartbeat from Router to Sensor Radio
	SG_HEARTBEAT_ROUTER_TO_SENSOR = 0x1E,
	/// Heartbeat from Sensor Radio to Router
	SG_HEARTBEAT_SENSOR_TO_ROUTER = 0x1F,
	/// Chirp: Command from Server to Sensor
	SG_CHIRP_COMMAND_SERVER_TO_SENSOR = 0x20,
	/// Chirp: Data from Sensor to Server
	SG_CHIRP_DATA_SENSOR_TO_SERVER = 0x21,
	/// Chirp: Network is disconnecting Sensor
	SG_CHIRP_NETWORK_DISCONNECTS_SENSOR = 0x25,
	/// Chirp: Sensor is disconnecting from network
	SG_CHIRP_SENSOR_DISCONNECT = 0x26,
	/// Squawk: Cancel Squawk. Server Requires sensor to forget it
	SG_SQUAWK_FORGET_ME = 0x27,
	/// Internal
	SG_SERVSTATUS = 0xfd,
};


/**
 * \brief Eyeball Mating Mode 
 *
 * Sent in an eyeball mode
 * to indicate whether it's ready to mate or not
 */
enum SansgridEyeballModeEnum {
	/// Don't start authentication
	SG_EYEBALL_NOMATE = 0x0,
	/// Start authentication
	SG_EYEBALL_MATE = 0x01
};


/**
 * \brief Peck Recognition Mode
 *
 * Sent in a peck recognized
 * to indicate whether or not the server recognizes the sensor
 */
enum SansgridPeckRecognitionEnum {
	/// Server recognizes the sensor. Squawk
	SG_PECK_RECOGNIZED = 0x0,
	/// Server doesn't recognize the sensor. Sing/Mock/Peacock
	SG_PECK_MATE = 0x01,
	/// Server Refuses Mating
	SG_PECK_SERVER_REFUSES_MATE = 0x02,
	/// Sensor Refuses Mating
	SG_PECK_SENSOR_REFUSES_MATE = 0x03,
	/// Server Waiting for permission
	SG_PECK_SERVER_WAIT = 0x04,
};


/**
 * \brief Simplified Payload datatypes
 *
 * Used to simplify the Sansgrid Payloads
 * into general payload types.
 * For instance, all the different kinds of chirp 
 * are simplified to SG_DEVSTATUS_CHIRPING.
 */
enum SansgridDeviceStatusEnum {
	SG_DEVSTATUS_NULL = 0,					///< Error
	SG_DEVSTATUS_HATCHING = (1<<1),			///< Hatching status
	SG_DEVSTATUS_FLYING = (1<<2),		///< Flying status
	SG_DEVSTATUS_EYEBALLING = (1<<3),	///< Eyeballing Status
	SG_DEVSTATUS_PECKING = (1<<4),		///< Pecking Status
	SG_DEVSTATUS_SINGING = (1<<5),		///< Singing Status
	SG_DEVSTATUS_MOCKING = (1<<6),		///< Mocking Status
	SG_DEVSTATUS_PEACOCKING = (1<<7),	///< Peacocking Status
	SG_DEVSTATUS_NESTING = (1<<8),		///< Nesting Status
	SG_DEVSTATUS_SQUAWKING = (1<<9),	///< Squawking Status
	SG_DEVSTATUS_HEARTBEAT = (1<<10),	///< Heartbeat Status
	SG_DEVSTATUS_CHIRPING = (1<<11),	///< Chirping Status
	
	// Compound types
	SG_DEVSTATUS_LEASED = (1<<12), 		///< Device Associated with Network
};

#endif

// vim: ft=c ts=4 noet sw=4:
