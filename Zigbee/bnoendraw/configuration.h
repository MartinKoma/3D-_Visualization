#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

//-----------------------------------------------
// Includes board-specific peripherals support in application.
//-----------------------------------------------
#include <BoardConfig.h>

//-----------------------------------------------
// BSP identifiers
//-----------------------------------------------
#define BOARD_FAKE 0x01
#define BOARD_RCB 0x02
#define BOARD_RCB_KEY_REMOTE 0x03
#define BOARD_ATMEGA256RFR2_XPRO 0x04
#define BOARD_SAMR21_XPRO 0x05
#define BOARD_SAMR21_ZLLEK 0x06
#define BOARD_SAMR21B18_MZ210PA_MODULE 0x07
#define BOARD_SAMR21G18_MR210UA_MODULE 0x08

//-----------------------------------------------
// Atmel communication interfaces identifiers.
// Supported interfaces are platform and application dependent.
//-----------------------------------------------
#define APP_INTERFACE_USART 0x01
#define APP_INTERFACE_VCP 0x02
#define APP_INTERFACE_SPI 0x03
#define APP_INTERFACE_UART 0x04
#define APP_INTERFACE_USBFIFO 0x05
#define APP_INTERFACE_STDIO 0x06
#define APP_INTERFACE_USBCDC 0x07

//-----------------------------------------------
// Atmel external memory identifiers.
// Supported memory is platform and application dependent.
//-----------------------------------------------
#define AT25F2048  0x01
#define AT45DB041  0x02
#define AT25DF041A 0x03
#define AT25DF041E 0x04
#define M25P40VMN6PB 0x05
#define MX25V4006E 0x06
#define MX25L2006E 0x07

// Enables or disables APS Fragmentation support.
#define APP_FRAGMENTATION 0
//#define APP_FRAGMENTATION 1

// Enables or disables binding support.
#define APP_USE_BINDING 0
//#define APP_USE_BINDING 1

#define APP_BLINK_PERIOD 1000

#define APP_MIN_BLINK_PERIOD 100

#define APP_MAX_BLINK_PERIOD 10000

// Enables or disables request of APS Acknowledgement for outgoing frames.
#define APP_USE_APS_ACK 0
//#define APP_USE_APS_ACK 1

//#define BSP_SUPPORT BOARD_RCB
//#define BSP_SUPPORT BOARD_FAKE
//#define BSP_SUPPORT BOARD_RCB_KEY_REMOTE
#define BSP_SUPPORT BOARD_ATMEGA256RFR2_XPRO

// Defines primary serial interface type to be used by application.
#define APP_INTERFACE APP_INTERFACE_USART
//#define APP_INTERFACE APP_INTERFACE_USBFIFO

//-----------------------------------------------
//APP_INTERFACE == APP_INTERFACE_USART
//-----------------------------------------------
#if (APP_INTERFACE == APP_INTERFACE_USART)
  // Defines USART interface name to be used by application.
  #define APP_USART_CHANNEL USART_CHANNEL_1
  //#define APP_USART_CHANNEL USART_CHANNEL_0
#endif

//-----------------------------------------------
//APP_INTERFACE == APP_INTERFACE_USBFIFO
//-----------------------------------------------
#if (APP_INTERFACE == APP_INTERFACE_USBFIFO)
  // Defines USART interface name to be used by application.
  #define APP_USART_CHANNEL USART_CHANNEL_USBFIFO
#endif

// ZDP response waiting timeout in milliseconds. To use automatically calculated
// value set to 0. The parameter determines the length of time for which the reply
// in response to a ZDP request is waited. if the response is not received before
// the timeout exceeds the confirmation callback for the ZDP request is called to
// report a failure status.
#define CS_ZDP_RESPONSE_TIMEOUT 0

// The time in milliseconds between concentrator route discoveries.
#define NWK_CONCENTRATOR_DISCOVERY_TIME 20000UL

// Attribute report time
#define CS_ZCL_ATTRIBUTE_REPORT_TIMER_INTERVAL 4000

// 32-bit mask of channels to be scanned before network is started. Channels that
// should be used are marked with logical 1 at corresponding bit location.
//  Valid channel numbers for 2.4 GHz band are 0x0b - 0x1a
//  Valid channel numbers for 900 MHz band are 0x00 - 0x0a
// 
//  Notes:
//  1. for small amount of enabled channels it is more convinient to specify list
// of channels in the form of '(1ul << 0x0b)'
//  2. For 900 MHz band you also need to specify channel page
// 
//  Value range: 32-bit values:
//  Valid channel numbers for 2.4 GHz band are 0x0b - 0x1a
//  Valid channel numbers for 900 MHz band are 0x00 - 0x0a
// 
//  C-type: uint32_t
//  Can be set: at any time before network start
#define CS_CHANNEL_MASK (1L<<0x0f)

// Enables or disables use of predefined PAN ID. Actual PAN ID is specified via
// CS_NWK_PANID parameter. If predefined PAN ID is disabled then PAN ID is selected
// randomly (default).
#define CS_NWK_PREDEFINED_PANID false
//#define CS_NWK_PREDEFINED_PANID true

// Short PAN ID of the network to start or to join to. The parameter holds the
// short PANID value generated randomly if CS_NWK_PREDEFINED_PANID equals false.
// Otherwise, the predefined parameter's value is used as the short PANID.
#define CS_NWK_PANID 0x1234

// A period in ms of polling a parent for data by an end device. On a sleeping end
// device the parameter determines a period with which poll requests are sent to
// the parent while the end device is awaken. A parent of a sleeping end device
// uses the parameter to calculate estimates of the time when the next poll request
// from a child will be received.Recommended value range is 100 - 1000.
#define CS_INDIRECT_POLL_RATE 1000

// ZigBee device type determines network behavior of a given device and functions
// it can perform. To give a brief overview, each networks contains exacty one
// coordinator and an arbirtary number of routers and end devices; an end device
// does not have children, data is passed through the parent, that is, a router or
// the coordinator.
//#define CS_DEVICE_TYPE DEV_TYPE_ROUTER

// While scanning channels during network join the node keeps listening to each
// channel specified by the ::CS_CHANNEL_MASK for a period of time calculated
// according to the formula that for the 2.4GHz frequency band is: 960 * 16 * (2
// raised to a power n + 1) microseconds, providing n is a value of this parameter.
// Note that the formula for the Sub-GHz employs another constant instead of 16.
#define CS_SCAN_DURATION 5

// Determines the maximum number of attempts to enter a network performed by the
// stack during network start. Upon each attempt ZDO sends a beacon request and
// collects beacon responses from nearby devices all over again.
#define CS_ZDO_JOIN_ATTEMPTS 4

// The parameter specifies the time span in milliseconds between two attempts to
// join the network.
#define CS_ZDO_JOIN_INTERVAL 1000

// 64-bit Unique Identifier (UID) determining the device extended address. If this
// value is 0 stack will try to read hardware UID from external UID or EEPROM chip.
// at startup. Location of hardware UID is platform dependend and it may not be
// available on all platforms. If the latter case then UID value must be provided
// by user via this parameter. This parameter must be unique for each device in a
// network. This should not be 0 for Coordinator.
//#define CS_UID 0x0A01LL

/****************************** Netzwerkids und Rollen ************************** */

#define DEV_ID 1 //Device is a coordinator.
//#define DEV_ID 1 //Device is Enddevice 1
//#define DEV_ID 2 //Device is Enddevice 2

#if (DEV_ID == 0)
	#define CS_DEVICE_TYPE DEV_TYPE_COORDINATOR
	#define CS_UID 0x0100000A11LL	
#endif

#if (DEV_ID == 1)
	#define CS_DEVICE_TYPE DEV_TYPE_ENDDEVICE
	#define CS_UID 0x0100000A01LL
#endif

#if (DEV_ID == 2)
	#define CS_DEVICE_TYPE DEV_TYPE_ENDDEVICE
	#define CS_UID 0x0100000A02LL
#endif


//#define CS_EXT_PANID 0x1AAAAAAAAAAACAFELL
#define CS_EXT_PANID 0xCAFEAAAAAAAAAA01LL

// Determines whether the static or automatic addressing mode will be used for the
// short address.
// 
//  If set to 1, the CS_NWK_ADDR parameter will be used as the device's short
// address. Otherwise, the short address is assigned automatically by the stack. An
// actual assignment method is specified in CS_ADDRESS_ASSIGNMENT_METHOD.
#define CS_NWK_UNIQUE_ADDR 0
//#define CS_NWK_UNIQUE_ADDR 1

//-----------------------------------------------
//CS_NWK_UNIQUE_ADDR == 1
//-----------------------------------------------
#if (CS_NWK_UNIQUE_ADDR == 1)
  // Specifies short (network) address if CS_NWK_UNIQUE_ADDR equals 1
  // 
  //  If static addressing is applied the stack uses the value of the parameter as a
  // short address. Otherwise, the stack assigns the parameter to a randomly chosen
  // value unique within the network. In both cases after the network start the
  // parameter holds actual short address of the device. While the device is in the
  // network its value must not be changed.
  #define CS_NWK_ADDR 0x0001
#endif

// The method of automatic address assignment.
// 
//  If CS_NWK_UNIQUE_ADDR equals 0 this parameter is used to determine the
// assignment method that is applied when a device enters the network to choose a
// short address. Otherwise, the parameter is ignored.
// 
//  Value range:
//  NWK_ADDR_ALLOC_DISTRIBUTED (equals 0) - distributed address allocation; the
// stack applies a special recurrent algorithm to form a kind of a search tree from
// the network to simplify routing
//  NWK_ADDR_ALLOC_STOCHASTIC (equals 2) - the address is set to a random value,
// different from all other short addresses in the network
//  NWK_ADDR_ALLOC_FROM_UID (equals 3) - two lower bytes of the extended address
// are used for the short address
#define CS_ADDRESS_ASSIGNMENT_METHOD 2
//Use distributed address allocation
//#define CS_ADDRESS_ASSIGNMENT_METHOD 0
//Use 2 octet from IEEE extended address
//#define CS_ADDRESS_ASSIGNMENT_METHOD 3

// Specifies whether a user descriptor is available on this device.
#define CS_USER_DESCRIPTOR_AVAILABLE true
//#define CS_USER_DESCRIPTOR_AVAILABLE false

// The maximum number of direct children that a given device (the coordinator or a
// router) can have.
// 
//  The parameter is only enabled for routers and the coordinator. An end device
// can not have children. If an actual number of children reaches a parameter's
// value, the node will have not been able to accept any more children joining the
// network. The parameter can be set to 0 on a router thus preventing it from
// accepting any children and can help form a desired network topology. For
// example, if the parameter is set to 0 on all routers, then the coordinator will
// be the only device that can have children and the network will have star
// topology.
#define CS_MAX_CHILDREN_AMOUNT 4

// The maximum number of routers among the direct children of the device
// 
//  The parameter determines how many routers the device can have as children. Note
// that the maximum number of end devices is equal to CS_MAX_CHILDREN_AMOUNT -
// CS_MAX_CHILDREN_ROUTER_AMOUNT.
#define CS_MAX_CHILDREN_ROUTER_AMOUNT 2

// Network depht limits amount of hops that packet may travel in the network.
// Actual maximum number of hops is network depth multiplied by 2.
// 
//  The parameter determines the maximum depth of a network tree formed by
// child-parent relationships between nodes.
// 
//  While joining the network the node receives beacon responses from potential
// parents containing their actual network depth and declines those which show
// values not less than the maximum network depth on the joining device. A
// potential parent will also reject a beacon from the joining device and will not
// sent a response if the joining device shows the network depth greater than it is
// allowed on the potential parent. This logic is enabled if the parameter value is
// not greater than 15. If its value is greater than 15, then device does not
// perform any checkings of the network depth, neither when joining a network nor
// when accepting other nodes as children. This allows forming long chains of
// devices across considerable distances.
#define CS_MAX_NETWORK_DEPTH 5

// If the parameter being switched between 0xff and 0x00, determines whether the
// device accepts or not a child joining the network via MAC association, that is,
// if the joining device does not possess the PANID value of the network and its
// PANID parameter is set to 0.
#define CS_PERMIT_DURATION 0xFF
//MAC association is off.
//#define CS_PERMIT_DURATION 0x00

// If the number of consecutive link status frames given by this parameter is
// missed from a neighbor it is removed from the neighbor table. For all neighbors
// except for end device children the stack tracks the time of receiving link
// statuses. If link statuses are not received from a given neighbor for this
// parameter's value times of link status period (typically 15 seconds), then the
// neighbor is deleted from the neighbor table.
#define CS_NWK_MAX_LINK_STATUS_FAILURES 3

// Is used to calculate the length of time after which a not responding end device
// child is considered lost. A sleeping end device is considered lost and a
// corresponding notification is raised on the parent, if the end device does not
// polls for data for the time span which duration is calculated by the following
// formula: CS_NWK_END_DEVICE_MAX_FAILURES * (CS_END_DEVICE_SLEEP_PERIOD +
// CS_INDIRECT_POLL_RATE)
#define CS_NWK_END_DEVICE_MAX_FAILURES 3

// Specifies receiver state (enabled or disabled) during inactive period for an end
// device. The parameter is taken into account on end devices only. Other devices
// behave as if the parameter equals true.
// 
//  If on an end device the parameter equals true, then the end device can receive
// data at any time, radio is always on, and its parent, which is informed about
// the parameter's value during association, sends data to the child immediately
// upon receiving a frame for the child.
// 
//  Switching the parameter to false on an end devices turns on indirect delivery:
// the end device's parent suspends data delivery to the child until it receives a
// polling request from the child; on the end device radio is only on when data is
// being sent.
#define CS_RX_ON_WHEN_IDLE false
//#define CS_RX_ON_WHEN_IDLE true

// End device sleep period given in milliseconds.
// 
//  On an end device this parameter determines the duration of a sleep period.
// Falling asleep is performed with the ZDO_SleepReq() request. After sleeping
// period exceeds the node is awakened and the application receives an indication
// via ZDO_WakeUpInd(). If the parameter's value is 0, then after the node falls
// asleep it can only be awakened by a hardware interrupt; a callback for a given
// IRQ is registered via HAL_RegisterIrq().
// 
//  On a router or the coordinator, the parameter is used in two ways:
// 
//  1) To remove information about lost child end devices. If a parent receives no
// data polls or data frames from the child end device for
// CS_NWK_END_DEVICE_MAX_FAILURES * (CS_END_DEVICE_SLEEP_PERIOD +
// CS_INDIRECT_POLL_RATE) ms, then it assumes it to be lost and deletes all
// information about such child.
// 
//  2) To determine whether to store or drop a message addressed to a child end
// device. The parent estimates the time when its child end device will wake up by
// adding this value to the moment when the last poll request has been received. If
// the time till end device wake up is greater than CS_MAC_TRANSACTION_TIME(Default
// 7680msec) the frame is stored. Otherwise, the frame is dropped.
#define CS_END_DEVICE_SLEEP_PERIOD 7500L

// Sleep period of routers and coordinator in ms.
#define CS_FFD_SLEEP_PERIOD 10000

// The parameter enabled in the high security mode specifies the size of the APS
// key-pair set. The APS key-pair set stores pairs of corresponding extended
// address and a link key or a master key. For each node with which the current
// node is going to communicate it must keep an entry with the remote node extended
// address and a link key. If the link key is unknown, the node can request the
// trust center for it via APS_RequestKeyReq(). The trust center must store a link
// key or a master key depending on the CS_SECURITY_STATUS used for each node it is
// going to authenticate. Entries can also be added manually by APS_SetLinkKey()
// and APS_SetMasterKey().
#define CS_APS_KEY_PAIR_DESCRIPTORS_AMOUNT 4

//-----------------------------------------------
//STANDARD_SECURITY_MODE
//-----------------------------------------------
#ifdef STANDARD_SECURITY_MODE
  // The parameter is used to determine the security type.
  // 
  //  Value range: 0,3 - for standard security; 1,2 - for high security.
  //  0 - network key is preconfigured ;
  //  1 - network join without master key, but with a trust center link key, which
  // must be set via APS_SetLinkKey();
  //  2 - network join employs a master key, which must be set APS_SetMasterKey();
  //  3 - network key is no preconfigured, but rather received from the trust center
  // in an unencrypted frame. <br.
  #define CS_ZDO_SECURITY_STATUS 0
  //#define CS_ZDO_SECURITY_STATUS 3
  
  // Depending on security key type and security mode this is either network key,
  // master key, link key or initial link key.
  // 
  //  Network key is used to encrypt a part of a data frame occupied by the NWK
  // payload. This type of encryption is applied in both the standard and high
  // security mode. The high security mode also enables encryption of the APS payload
  // with a link key, but if the txOptions.useNwkKey field in APS request parameters
  // is set to 0, the APS payload is encrypted with the network key.
  // 
  //  The network key must be predefined if standard security is used with
  // CS_ZDO_SECURITY_STATUS set to 0. For all other values of CS_ZDO_SECURITY_STATUS
  // the network key is received from the trust center during device authentication.
  // Note that in the standard security mode with CS_ZDO_SECURITY_STATUS equal to 3
  // the network key is transferred to the joining device in an unencrypted frame.
  #define CS_NETWORK_KEY {0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC}
  
  // The maximum number of network keys that can be stored on the device
  // 
  //  A device in a secured network can keep several network keys up to the value of
  // this parameter. Upon frame reception the device extracts key sequence number
  // from the auxiliary header of the frame and decrypts the message with the network
  // key corresponding to this sequence number. Besides, one key is considered active
  // for each device; this is the key that is used for encrypting outgoing frames.
  // The keys are distributed by the trust center with the help of the
  // APS_TransportKeyReq() command. The trust center can also change the active key
  // of a remote node via a call to APS_SwitchKeyReq().
  #define CS_NWK_SECURITY_KEYS_AMOUNT 1
  
  // Address of device responsible for authentication and key distribution (Trust
  // Center).
  // 
  //  The parameter specifies the trust center extended address. The stack makes use
  // of the parameter to support various operations in networks with security
  // enabled. For correct network operation a parameter's value must coincide with
  // the actual trust center address.
  // 
  //  In case the trust center extended address is unknown, for example, for testing
  // purposes, the parameter can be assigned to the universal trust center address
  // which equals APS_SM_UNIVERSAL_TRUST_CENTER_EXT_ADDRESS.
  #define CS_APS_TRUST_CENTER_ADDRESS 0xAAAAAAAAAAAAAAAALL
  
  // Security information waiting timeout before secure network join considered
  // failed.
  // 
  //  A timeout is started when connection with a parent is established. If the
  // security related procedures that are performed after this will not be completed
  // before the timeout exceeds, the device will fail joining the network. A value is
  // measured in milliseconds.
  #define CS_APS_SECURITY_TIMEOUT_PERIOD 10000
#endif

//-----------------------------------------------
//STDLINK_SECURITY_MODE
//-----------------------------------------------
#ifdef STDLINK_SECURITY_MODE
  // Security information waiting timeout before secure network join considered
  // failed.
  // 
  //  A timeout is started when connection with a parent is established. If the
  // security related procedures that are performed after this will not be completed
  // before the timeout exceeds, the device will fail joining the network. A value is
  // measured in milliseconds.
  #define CS_APS_SECURITY_TIMEOUT_PERIOD 10000
  
  // The parameter is used to determine the security type.
  // 
  //  Value range: 0,3 - for standard security; 1,2 - for high security.
  //  0 - network key is preconfigured ;
  //  1 - network join without master key, but with a trust center link key, which
  // must be set via APS_SetLinkKey();
  //  2 - network join employs a master key, which must be set APS_SetMasterKey();
  //  3 - network key is no preconfigured, but rather received from the trust center
  // in an unencrypted frame. <br.
  #define CS_ZDO_SECURITY_STATUS 1
  //#define CS_ZDO_SECURITY_STATUS 2
  
  // Address of device responsible for authentication and key distribution (Trust
  // Center).
  // 
  //  The parameter specifies the trust center extended address. The stack makes use
  // of the parameter to support various operations in networks with security
  // enabled. For correct network operation a parameter's value must coincide with
  // the actual trust center address.
  // 
  //  In case the trust center extended address is unknown, for example, for testing
  // purposes, the parameter can be assigned to the universal trust center address
  // which equals APS_SM_UNIVERSAL_TRUST_CENTER_EXT_ADDRESS.
  #define CS_APS_TRUST_CENTER_ADDRESS 0xAAAAAAAAAAAAAAAALL
  
  // The maximum number of network keys that can be stored on the device
  // 
  //  A device in a secured network can keep several network keys up to the value of
  // this parameter. Upon frame reception the device extracts key sequence number
  // from the auxiliary header of the frame and decrypts the message with the network
  // key corresponding to this sequence number. Besides, one key is considered active
  // for each device; this is the key that is used for encrypting outgoing frames.
  // The keys are distributed by the trust center with the help of the
  // APS_TransportKeyReq() command. The trust center can also change the active key
  // of a remote node via a call to APS_SwitchKeyReq().
  #define CS_NWK_SECURITY_KEYS_AMOUNT 1
  
  // Depending on security key type and security mode this is either network key,
  // master key, link key or initial link key.
  // 
  //  Network key is used to encrypt a part of a data frame occupied by the NWK
  // payload. This type of encryption is applied in both the standard and high
  // security mode. The high security mode also enables encryption of the APS payload
  // with a link key, but if the txOptions.useNwkKey field in APS request parameters
  // is set to 0, the APS payload is encrypted with the network key.
  // 
  //  The network key must be predefined if standard security is used with
  // CS_ZDO_SECURITY_STATUS set to 0. For all other values of CS_ZDO_SECURITY_STATUS
  // the network key is received from the trust center during device authentication.
  // Note that in the standard security mode with CS_ZDO_SECURITY_STATUS equal to 3
  // the network key is transferred to the joining device in an unencrypted frame.
  #define CS_NETWORK_KEY {0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC}
#endif

// The maximum amount of records in the permission table.
#define CS_PERMISSIONS_TABLE_SIZE 5

// The maximum number of authentication requests that the trust center can process
// simultaneously. The parameter is used on the trust center to allocate memory for
// buffers used during joining device authentication. A value of the parameter
// determines how many authentication request the stack on the trust center can
// process at once.
#define CS_MAX_TC_AUTHENTIC_PROCESS_AMOUNT 1

// Keep-alive polling interval
#define CS_ZDO_TC_KEEP_ALIVE_INTERVAL 20

// Maximum amount of records in the Group Table.
// 
//  The Group Table size cannot be 0. The group table stores pairs of a group
// address and an endpoint. Upon receiving a frame addressed to members of a
// certain group which include the current node as well the stack fires indications
// on all endpoints registered with the group address.
#define CS_GROUP_TABLE_SIZE 1

// Maximum amount of records in the Neighbor Table.
// 
//  The parameter determines the size of the neighbor table which is used to store
// beacon responses from nearby devices. The parameter puts an upper bound over the
// amount of child devices possible for the node.
#define CS_NEIB_TABLE_SIZE 7

// Maximum amount of records in the network Route Table.
// 
//  The parameter sets the maximum number of records that can be kept in the NWK
// route table. The table is used by NWK to store information about established
// routes. Each table entry specifies the next-hop short address for a route from
// the current node to a given destination node. The table is being filled
// automatically during route discovery. An entry is added when a route is
// discovered.
#define CS_ROUTE_TABLE_SIZE 4

// Maximum amount of records in the network Address Map Table.
// 
//  The parameter sets the maximum number of records in the address map table used
// by NWK to store pairs of corresponding short and extended addresses. The stack
// appeals to the table when a data frame is being sent to a specified extended
// address to extract the corresponding short address. If it fails to find the
// short address, an error is reported.
#define CS_ADDRESS_MAP_TABLE_SIZE 5

// Maximum amount of records in the network Route Discovery Table.
// 
//  The parameter specifies the size of the route discovery table used by NWK to
// store next-hop addresses of the nodes for routes that are not yet established.
// Upon exhausting the capacity of the table, the stack starts rewriting old
// entries. If the size of the route table is big enough after all used routes are
// established the table may not be used.
#define CS_ROUTE_DISCOVERY_TABLE_SIZE 3

// Maximum amount of records in the Duplicate Rejection Table.
// 
//  The duplicate rejection table is used by APS to store information about
// incoming unicast messages in order to reject messages that have been already
// received and processed. Following ZigBee specification, the parameter should be
// not less than 1.
#define CS_DUPLICATE_REJECTION_TABLE_SIZE 10

// Maximum amount of records in the Broadcast Transaction Table.
// 
//  The broadcast transmission table is used for tracking incoming broadcast
// messages to mark messages that have already been processed by the node. This
// causes only one copy for each broadcast message to be processed. An entry for a
// broadcast message is stored for a certain period of time and then removed.
#define CS_NWK_BTT_SIZE 8

// Maximim amount of records in the Binding Table
// 
//  The parameter sets the size of the binding table used by APS to store binding
// links, which are structures containing information about source and destination
// extended addresses and endpoints for unicast bindings and just group addresses
// as destinations for group bindings. If the binding is going to be applied to
// send a data frame, then the corresponding entry shall be first inserted into the
// table via the APS_BindingReq() function.
#define CS_APS_BINDING_TABLE_SIZE 1

// MAC ban table size
#define CS_BAN_TABLE_SIZE 1

// Maximum number of records in the route cache.
#define CS_ROUTE_CACHE_SIZE 1

// The number of buffers for data requests on the APS layer.
// 
//  The parameter specifies the number of buffers that are allocated by APS to
// store data requests parameters. The parameter puts an upper bound to the number
// of data requests that can be processed by APS simultaneously. If all buffers are
// in use and a new data request appears, it is kept in a queue until a buffer is
// released.
#define CS_APS_DATA_REQ_BUFFERS_AMOUNT 3

// The number of buffers for acknowledgement messages sent by APS.
// 
//  This parameter determines the amount of memory that needs to be allocated for a
// special type of buffers used by APS to store payloads for acknowledgement
// frames. The need to use the buffers occurs when the node receives a frame that
// has to be acknowledged. That is, the APS component on the node has to send an
// acknowledgement frame. For frames initiated by the application, the memory for a
// payload is to be allocated by the application on its own, while the payload
// memory for an acknowledgement frame shall be reserved by APS. The request
// parameters are still stored in the data request buffers.
#define CS_APS_ACK_FRAME_BUFFERS_AMOUNT 2

// Amount of buffers on NWK layer used to keep incoming and outgoing frames. This
// parameters affects how many children of a parent are able to get broadcat
// messages.
#define CS_NWK_BUFFERS_AMOUNT 4

// Amount of ZCL memory buffers
#define CS_ZCL_MEMORY_BUFFERS_AMOUNT 5

// MAC transaction persistence time.The parameter determines the maximum interval
// (in ms) a frame addressed to a sleeping end device can be stored on the parent
// node. If the end device does not poll for data during this time, then the frame
// is dropped.
#define CS_MAC_TRANSACTION_TIME 7680L

// The maximum number of retries that will be performed by APS layer before
// reporting failed transmission.
// 
//  The parameter sets the number of attempts that will be made by APS layer to
// transmit a data frame. If all these attempts fail due to underlying layers
// failures, then APS response with an error status.
#define CS_APS_MAX_FRAME_RETRIES 3

// The maximum route cost between two nodes for direct delivery.
// 
//  A data frame is routed directly to the neighbor only if the route cost, read
// from the neighbor table, is less than the value of the parameter. If the route
// cost is greater than the value of the parameter, route discovery will be
// initiated to find an indirect route to the nighbor.
#define CS_MAX_NEIGHBOR_ROUTE_COST 8

// The parameter specifies the TX power of the transceiver device, is measured in
// dBm(s). After the node has entered the network the value can only be changed via
// the ZDO_SetTxPowerReq() function.
// 
//  Value range: depends on the hardware. Transmit power must be in the range from
// -17 to 3 dBm for AT86RF231, AT86RF230 and AT86RF230B. For AT86RF233 transmit
// power must be in the range from -17 to 4 dBm. For AT86RF212 transmit power must
// be in the range from -11 to 11 dBm.
#define CS_RF_TX_POWER 3

//-----------------------------------------------
//APP_FRAGMENTATION == 1
//-----------------------------------------------
#if (APP_FRAGMENTATION == 1)
  // This parameter limits the number of pieces to which the data sent with one APS
  // request can be split i f the fragmentation feature is applied. Thus it also
  // limits the maximum amount of data sent by the application with a single request:
  //  maximum data length = CS_APS_MAX_BLOCKS_AMOUNT * CS_APS_BLOCK_SIZE if the
  // latter parameter is not 0, else
  //  maximum data length = CS_APS_MAX_BLOCKS_AMOUNT * APS_MAX_ASDU_SIZE.
  #define CS_APS_MAX_BLOCKS_AMOUNT 4
  
  // Maximim size of a single fragment during fragmented transmission.
  // 
  //  If the value is 0, blocks will be of the maximum possilbe size; that is the
  // size will equal the value of CS_APS_MAX_ASDU_SIZE. The parameter and
  // CS_APS_MAX_BLOCKS_AMOUNT are also used to determine an amount of memory
  // allocated for a special buffer that keeps parts of incoming fragmented message
  // until all of them are received.
  #define CS_APS_BLOCK_SIZE 0
#endif

// The parameter determines how many pieces of a fragmented message are sent before
// waiting for an aknowledgement. After acknowledgement is received another portion
// of frames are sent and so on.
#define CS_APS_MAX_TRANSMISSION_WINDOW_SIZE 1


#endif // _CONFIGURATION_H_
