#pragma once
#include <string>

struct Flight {
    std::string date = "ERROR!";
    std::string departure_airport = "----";
    std::string arrival_airport = "----";
    short unsigned int landings_count = 0;
    float total_hours = 0.0;
    float night_hours = 0.0;
    float instrument_hours = 0.0;
    float cross_country_hours = 0.0;
    std::string tailnumber = "";
    std::string aircraft_manufacturer = "";
    std::string aircraft_type = "";
};