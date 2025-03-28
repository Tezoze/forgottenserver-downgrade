#include <boost/asio/ip/address.hpp> // Add this include for address::from_string

// Replace the problematic line with the following:
boost::asio::ip::address_v4::from_string(getString(ConfigManager::IP)), serverPort));
