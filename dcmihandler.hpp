#pragma once

#include "nlohmann/json.hpp"

#include <sdbusplus/bus.hpp>

#include <map>
#include <string>
#include <vector>

namespace dcmi
{

using NumInstances = size_t;
using Json = nlohmann::json;

enum Commands
{
    GET_POWER_READING = 0x02,
    GET_SENSOR_INFO = 0x07,
    SET_CONF_PARAMS = 0x12,
    GET_CONF_PARAMS = 0x13,
};

static constexpr auto propIntf = "org.freedesktop.DBus.Properties";
static constexpr auto assetTagIntf =
    "xyz.openbmc_project.Inventory.Decorator.AssetTag";
static constexpr auto assetTagProp = "AssetTag";
static constexpr auto networkServiceName = "xyz.openbmc_project.Network";
static constexpr auto networkConfigObj = "/xyz/openbmc_project/network/config";
static constexpr auto networkConfigIntf =
    "xyz.openbmc_project.Network.SystemConfiguration";
static constexpr auto hostNameProp = "HostName";
static constexpr auto temperatureSensorType = 0x01;
static constexpr size_t maxInstances = 255;
static constexpr uint8_t maxRecords = 8;
static constexpr auto gDCMISensorsConfig =
    "/usr/share/ipmi-providers/dcmi_sensors.json";
static constexpr auto ethernetIntf =
    "xyz.openbmc_project.Network.EthernetInterface";
static constexpr auto ethernetDefaultChannelNum = 0x1;
static constexpr auto networkRoot = "/xyz/openbmc_project/network";
static constexpr auto dhcpObj = "/xyz/openbmc_project/network/dhcp";
static constexpr auto dhcpIntf =
    "xyz.openbmc_project.Network.DHCPConfiguration";
static constexpr auto systemBusName = "org.freedesktop.systemd1";
static constexpr auto systemPath = "/org/freedesktop/systemd1";
static constexpr auto systemIntf = "org.freedesktop.systemd1.Manager";
static constexpr auto gDCMICapabilitiesConfig =
    "/usr/share/ipmi-providers/dcmi_cap.json";
static constexpr auto gDCMIPowerMgmtCapability = "PowerManagement";
static constexpr auto gDCMIPowerMgmtSupported = 0x1;
static constexpr auto gMaxSELEntriesMask = 0xFFF;
static constexpr auto gByteBitSize = 8;

namespace sensor_info
{

/** @struct Response
 *
 *  DCMI payload for Get Sensor Info response
 */
struct Response
{
    uint8_t recordIdLsb; //!< SDR record id LS byte
    uint8_t recordIdMsb; //!< SDR record id MS byte
} __attribute__((packed));

using ResponseList = std::vector<Response>;
} // namespace sensor_info

static constexpr auto groupExtId = 0xDC;

/** @brief Check whether DCMI power management is supported
 *         in the DCMI Capabilities config file.
 *
 *  @return True if DCMI power management is supported
 */
bool isDCMIPowerMgmtSupported();

/** @brief Parse out JSON config file.
 *
 *  @param[in] configFile - JSON config file name
 *
 *  @return A json object
 */
Json parseJSONConfig(const std::string& configFile);

namespace sensor_info
{
/** @brief Create response from JSON config.
 *
 *  @param[in] config - JSON config info about DCMI sensors
 *
 *  @return Sensor info response
 */
Response createFromJson(const Json& config);

/** @brief Read sensor info and fill up DCMI response for the Get
 *         Sensor Info command. This looks at a specific
 *         instance.
 *
 *  @param[in] type - one of "inlet", "cpu", "baseboard"
 *  @param[in] instance - A non-zero Entity instance number
 *  @param[in] config - JSON config info about DCMI sensors
 *
 *  @return A tuple, containing a sensor info response and
 *          number of instances.
 */
std::tuple<Response, NumInstances> read(const std::string& type,
                                        uint8_t instance, const Json& config);

/** @brief Read sensor info and fill up DCMI response for the Get
 *         Sensor Info command. This looks at a range of
 *         instances.
 *
 *  @param[in] type - one of "inlet", "cpu", "baseboard"
 *  @param[in] instanceStart - Entity instance start index
 *  @param[in] config - JSON config info about DCMI sensors
 *
 *  @return A tuple, containing a list of sensor info responses and the
 *          number of instances.
 */
std::tuple<ResponseList, NumInstances>
    readAll(const std::string& type, uint8_t instanceStart, const Json& config);
} // namespace sensor_info

/** @brief Read power reading from power reading sensor object
 *
 *  @param[in] bus - dbus connection
 *
 *  @return total power reading
 */
int64_t getPowerReading(sdbusplus::bus_t& bus);

/** @struct GetPowerReadingRequest
 *
 *  DCMI Get Power Reading command request.
 *  Refer DCMI specification Version 1.1 Section 6.6.1
 */
struct GetPowerReadingRequest
{
    uint8_t mode;          //!< Mode
    uint8_t modeAttribute; //!< Mode Attributes
} __attribute__((packed));

/** @struct GetPowerReadingResponse
 *
 *  DCMI Get Power Reading command response.
 *  Refer DCMI specification Version 1.1 Section 6.6.1
 */
struct GetPowerReadingResponse
{
    uint16_t currentPower;     //!< Current power in watts
    uint16_t minimumPower;     //!< Minimum power over sampling duration
                               //!< in watts
    uint16_t maximumPower;     //!< Maximum power over sampling duration
                               //!< in watts
    uint16_t averagePower;     //!< Average power over sampling duration
                               //!< in watts
    uint32_t timeStamp;        //!< IPMI specification based time stamp
    uint32_t timeFrame;        //!< Statistics reporting time period in milli
                               //!< seconds.
    uint8_t powerReadingState; //!< Power Reading State
} __attribute__((packed));

/** @struct GetSensorInfoRequest
 *
 *  DCMI payload for Get Sensor Info request
 */
struct GetSensorInfoRequest
{
    uint8_t sensorType;     //!< Type of the sensor
    uint8_t entityId;       //!< Entity ID
    uint8_t entityInstance; //!< Entity Instance (0 means all instances)
    uint8_t instanceStart;  //!< Instance start (used if instance is 0)
} __attribute__((packed));

/** @struct GetSensorInfoResponseHdr
 *
 *  DCMI header for Get Sensor Info response
 */
struct GetSensorInfoResponseHdr
{
    uint8_t numInstances; //!< No. of instances for requested id
    uint8_t numRecords;   //!< No. of record ids in the response
} __attribute__((packed));
/**
 *  @brief Parameters for DCMI Configuration Parameters
 */
enum class DCMIConfigParameters : uint8_t
{
    ActivateDHCP = 1,
    DiscoveryConfig,
    DHCPTiming1,
    DHCPTiming2,
    DHCPTiming3,
};

/** @struct SetConfParamsRequest
 *
 *  DCMI Set DCMI Configuration Parameters Command.
 *  Refer DCMI specification Version 1.1 Section 6.1.2
 */
struct SetConfParamsRequest
{
    uint8_t paramSelect; //!< Parameter selector.
    uint8_t setSelect;   //!< Set Selector (use 00h for parameters that only
                         //!< have one set).
    uint8_t data[];      //!< Configuration parameter data.
} __attribute__((packed));

/** @struct GetConfParamsRequest
 *
 *  DCMI Get DCMI Configuration Parameters Command.
 *  Refer DCMI specification Version 1.1 Section 6.1.3
 */
struct GetConfParamsRequest
{
    uint8_t paramSelect; //!< Parameter selector.
    uint8_t setSelect;   //!< Set Selector. Selects a given set of parameters
                         //!< under a given Parameter selector value. 00h if
                         //!< parameter doesn't use a Set Selector.
} __attribute__((packed));

/** @struct GetConfParamsResponse
 *
 *  DCMI Get DCMI Configuration Parameters Command response.
 *  Refer DCMI specification Version 1.1 Section 6.1.3
 */
struct GetConfParamsResponse
{
    uint8_t major;         //!< DCMI Spec Conformance - major ver = 01h.
    uint8_t minor;         //!< DCMI Spec Conformance - minor ver = 05h.
    uint8_t paramRevision; //!< Parameter Revision = 01h.
    uint8_t data[];        //!< Parameter data.
} __attribute__((packed));

} // namespace dcmi
