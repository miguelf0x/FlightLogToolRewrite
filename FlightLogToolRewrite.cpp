// FlightLogToolRewrite.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <Windows.h>
#include <map>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <variant>

#include "Aircraft.h"
#include "Flight.h"

#undef max

int n;
int lines;
std::string native_flightlog_path;
std::string modded_flightlog_path;

void print_load_menu() {
    std::cout << "Select input method: " << std::endl;
    std::cout << "1. Load from original logbook file" << std::endl;
    std::cout << "2. Load from formatted (.csv) logbook file" << std::endl;
    return;
}

void print_main_menu() {
    std::cout << std::endl;
    std::cout << "Select action:" << std::endl;
    std::cout << "1. Print logbook" << std::endl;
    std::cout << "2. Edit logbook" << std::endl;
    std::cout << "3. Statistics" << std::endl;
    std::cout << "8. Settings" << std::endl;
    std::cout << "9. Save and exit" << std::endl;
    std::cout << "0. Exit\n" << std::endl;
    return;
}

void print_stats_menu() {
    std::cout << std::endl;
    std::cout << "Select action:" << std::endl;
    std::cout << "1. Calculate landings and total hours by their types" << std::endl;
    std::cout << "2. Calculate landings and total hours by aircraft type" << std::endl;
    std::cout << "3. Calculate landings and total hours by aircraft manufacturer" << std::endl;
    std::cout << "9. Return to main menu\n" << std::endl;
    return;
}

int choice() {
    std::cout << "> ";
    short unsigned int menuchoice = 0;
    std::cin >> menuchoice;
    return menuchoice;
}

void change_settings() {

    std::ifstream fin;
    fin.open("./settings.txt");

    std::string path;

    try {
        if (!fin) {
            throw std::invalid_argument("Settings file not found!");
        }
    }
    catch (std::invalid_argument const& ex) {
        std::cerr << "\nCERR: " << ex.what() << '\n';
    }

    fin >> path;
    std::cout << "Current logbook path: " << path << std::endl;
    fin.close();

    std::cout << "Please enter full path to native logbook path:" << std::endl;
    std::cin.ignore();
    std::getline(std::cin, path);

    std::ofstream fout;
    fout.open("./settings.txt");
    fout << path << std::endl;

    native_flightlog_path = path;

    fout.close();

    return;
};

Aircraft aircraft_type_and_manufacturer_resolver(std::string input) {
    std::map <std::string, int> mapping;

    mapping["727-200Adv"] = 0;
    mapping["Rotate-MD-80-XP11"] = 1;
    mapping["B733"] = 2;
    mapping["B38M"] = 3;
    mapping["a321_StdDef"] = 4;
    mapping["a321"] = 5;
    mapping["A340-600_StdDef"] = 6;
    mapping["A340-600"] = 7;
    mapping["tu154"] = 8;
    mapping["29A_XP11"] = 9;
    mapping["a320neo"] = 10;
    mapping["Cessna_172SP"] = 11;
    mapping["A350_xp11"] = 12;
    mapping["Orbiter"] = 13;

    switch (mapping[input]) {
    case 0: return{ "Boeing", "737-200 Advanced" }; break;
    case 1: return{ "McDonnell Douglas", "MD-88" }; break;
    case 2: return{ "Boeing", "737-300" }; break;
    case 3: return{ "Boeing", "737-800" }; break;
    case 4:
    case 5: return{ "Airbus", "A321" }; break;
    case 6:
    case 7: return{ "Airbus", "A340-600" }; break;
    case 8: return{ "Tupolev", "TU154M" }; break;
    case 9: return{ "Aero", "L-29" }; break;
    case 10: return{ "Airbus", "A320neo" }; break;
    case 11: return{ "Cessna", "172SP" }; break;
    case 12: return{ "Airbus", "A350-900" }; break;
    case 13: return{ "[multiple]", "Space Shuttle" }; break;
    default: return{ "Unknown", "Unknown" }; break;
    }

}

int input_from_file(std::vector<Flight>& Flights) {

    std::ifstream fin;
    fin.open("./settings.txt");

    try {
        if (!fin) {
            throw std::invalid_argument("Settings file not found!");
        }
    }
    catch (std::invalid_argument const& ex) {
        std::cerr << "\nCERR: " << ex.what() << '\n';
        change_settings();
        fin.open("./settings.txt");
    }

    char temp[50] = "";
    fin.getline(temp, 50);
    native_flightlog_path = temp;
    fin.close();

    std::ifstream linecounter{ native_flightlog_path };
    lines = std::count(std::istream_iterator<char>(linecounter >> std::noskipws), {}, '\n');
    linecounter.close();

    std::cout << native_flightlog_path << " " << lines << std::endl;

    std::ifstream readlog;
    readlog.open(native_flightlog_path.c_str());

    try {
        if (!readlog) {
            throw std::invalid_argument("Flight log file not found!");
        }
    }
    catch (std::invalid_argument const& ex) {
        std::cerr << "\nCERR: " << ex.what() << '\n';
        return -1;
    }

    readlog.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    readlog.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < lines - 3; i++) {
        std::string fldate = "", dep = "", arr = "";
        short unsigned int dummy, ldg_cnt = 0;
        float total_h = 0.0, night_h = 0.0, ifr_h = 0.0, cc_h = 0.0;
        std::string tail, aircraft, aircraft_man, aircraft_type;

        readlog >> dummy >> fldate >> dep >> arr >> ldg_cnt >> total_h >> night_h >> ifr_h >> cc_h >> tail >> aircraft;
        char cdate[7] = "nodate", temp[3] = "00";
        strcpy_s(cdate, fldate.c_str());             // yymmdd
        temp[0] = cdate[0]; temp[1] = cdate[1];      // yymmdd yy
        cdate[0] = cdate[4]; cdate[1] = cdate[5];    // ddmmdd yy
        cdate[4] = temp[0]; cdate[5] = temp[1];

        Aircraft a = aircraft_type_and_manufacturer_resolver(aircraft);
        aircraft_man = a.aircraft_manufacturer;
        aircraft_type = a.aircraft_type;

        Flights.push_back({ cdate, dep, arr, ldg_cnt, total_h, night_h, ifr_h, cc_h, tail, aircraft_man, aircraft_type });
    };

    n = Flights.size();

    readlog.close();

    return 0;

}

int input_from_formatted_file(std::vector<Flight>& Flights) {

    std::string formatted_path = "";

    std::cout << "Please enter full path to .csv file: " << std::endl;
    std::cin.ignore();
    std::getline(std::cin, formatted_path);

    std::ifstream linecounter{ formatted_path };
    lines = std::count(std::istream_iterator<char>(linecounter >> std::noskipws), {}, '\n');
    linecounter.close();

    std::cout << formatted_path << "; " << lines << std::endl;

    std::ifstream readlog;
    readlog.open(formatted_path);

    try {
        if (!readlog) {
            throw std::invalid_argument("Flight log file not found!");
        }
    }
    catch (std::invalid_argument const& ex) {
        std::cerr << "\nCERR: " << ex.what() << '\n';
        return -2;
    }

    readlog.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string line, word;
    std::vector<std::string> data;

    while (std::getline(readlog, line)) {
        std::stringstream ss(line);
        data.clear();
        while (std::getline(ss, word, ',')) {
            data.push_back(word);
        }
        Flight flight = { data[0], data[1], data[2], stoi(data[3]), stof(data[4]), stof(data[5]), stof(data[6]), stof(data[7]), data[8], data[9], data[10] };
        Flights.push_back(flight);
    }

    n = Flights.size();

    readlog.close();

    return 0;
}

void print_flights(std::vector<Flight>& Flights) {
    
    std::cout << " Num | Flight | Dep. | Arr. |  LDG  | Total | Night |  IFR  |  C/C  |  Tail  |      Aircraft      |    Aircraft    " << std::endl;
    std::cout << "     |  date  | ICAO | ICAO | Count | hours | hours | hours | hours | number |    manufacturer    |      type      " << std::endl;
    std::cout << "-----+--------+------+------+-------+-------+-------+-------+-------+--------+--------------------+----------------" << std::endl;
    
    for (int i = 0; i < n; i++) {

        std::string strnum;
        if (i < 10) { strnum = "00" + std::to_string(i) + "."; }
        else if (i < 100) { strnum = "0" + std::to_string(i) + "."; }
        else { strnum = std::to_string(i) + "."; }

        std::string formatted_tailnum = Flights[i].tailnumber;
        while (formatted_tailnum.length() < 6) { formatted_tailnum += " "; };

        std::string formatted_manufacturer = Flights[i].aircraft_manufacturer;
        while (formatted_manufacturer.length() < 16) { formatted_manufacturer = " " + formatted_manufacturer + " "; };
        while (formatted_manufacturer.length() < 18) { formatted_manufacturer += " "; };

        std::cout << strnum << " | " << Flights[i].date << " | " << Flights[i].departure_airport << " | " << Flights[i].arrival_airport << " |   " << Flights[i].landings_count << "   |  ";
        std::cout << std::fixed << std::setprecision(1);
        std::cout << Flights[i].total_hours << "  |  " << Flights[i].night_hours << "  |  " << Flights[i].instrument_hours;
        std::cout << "  |  " << Flights[i].cross_country_hours << "  | " << formatted_tailnum << " | " << formatted_manufacturer << " | " << Flights[i].aircraft_type << std::endl;
    }

    return;
}

void edit_flights(std::vector<Flight>& Flights) {
    int line = 1;
    print_flights(Flights);
    std::cout << "Enter line to be edited number (1-" << n << "): ";
    std::cin >> line;

    try {
        if (line <= 0 || line > n) {
            throw std::invalid_argument("Entered line number not found!");
        }
    }
    catch (std::invalid_argument const& ex) {
        std::cerr << "\nCERR: " << ex.what() << '\n';
        return;
    }

    std::string strbuf = "";

    std::cout << "Enter flight date [Current: " << Flights[line - 1].date << "]: ";
    std::cin >> Flights[line - 1].date;

    std::cout << "Enter departure airport ICAO code [Current: " << Flights[line - 1].departure_airport << "]: ";
    std::cin >> Flights[line - 1].departure_airport;

    std::cout << "Enter arrival airport ICAO code [Current: " << Flights[line - 1].arrival_airport << "]: ";
    std::cin >> Flights[line - 1].arrival_airport;

    std::cout << "Enter landings count [Current: " << Flights[line - 1].landings_count << "]: ";
    std::cin >> Flights[line - 1].landings_count;

    std::cout << "Enter total hours [Current: " << Flights[line - 1].total_hours << "]: ";
    std::cin >> Flights[line - 1].total_hours;

    std::cout << "Enter night hours [Current: " << Flights[line - 1].night_hours << "]: ";
    std::cin >> Flights[line - 1].night_hours;

    std::cout << "Enter instrument hours [Current: " << Flights[line - 1].instrument_hours << "]: ";
    std::cin >> Flights[line - 1].instrument_hours;

    std::cout << "Enter cross country hours [Current: " << Flights[line - 1].cross_country_hours << "]: ";
    std::cin >> Flights[line - 1].cross_country_hours;

    std::cout << "Enter tailnumber [Current: " << Flights[line - 1].tailnumber << "]: ";
    std::cin >> Flights[line - 1].tailnumber;

    std::cin.ignore();
    std::cout << "Enter aircraft manufacturer [Current: " << Flights[line - 1].aircraft_manufacturer << "]: ";
    std::getline(std::cin, strbuf);
    Flights[line - 1].aircraft_manufacturer = strbuf;

    std::cin.ignore();
    std::cout << "Enter aircraft type [Current: " << Flights[line - 1].aircraft_type << "]: ";
    std::getline(std::cin, strbuf);
    Flights[line - 1].aircraft_type = strbuf;

    std::cout << std::endl << "Changes saved!";

    return;
}

void calculate_stats(std::vector<Flight>& Flights) {

    print_stats_menu();
    short unsigned int statschoice = choice();
    switch (statschoice) {
    case 1:

    {
        float hours_sum = 0.0, night_hours_sum = 0.0, instrument_hours_sum = 0.0, cross_country_hours_sum = 0.0;
        int landings = 0;
        for (int i = 0; i < n; i++) {
            hours_sum += Flights[i].total_hours;
            night_hours_sum += Flights[i].night_hours;
            instrument_hours_sum += Flights[i].instrument_hours;
            cross_country_hours_sum += Flights[i].cross_country_hours;
            landings += Flights[i].landings_count;
        }

        std::cout << "==============================" << std::endl;
        std::cout << " Total hours: " << hours_sum << std::endl;
        std::cout << " Total night hours: " << night_hours_sum << std::endl;
        std::cout << " Total instrument hours: " << instrument_hours_sum << std::endl;
        std::cout << " Total cross-country hours: " << cross_country_hours_sum << std::endl;
        std::cout << " Total landings: " << landings << std::endl;
        std::cout << "==============================" << std::endl;
    }

    break;

    case 2:

    {
        std::string type;
        std::vector<std::string> aircraft_types;

        for (int i = 0; i < n; i++) {
            type = Flights[i].aircraft_type;
            if (std::find(aircraft_types.begin(), aircraft_types.end(), type) == aircraft_types.end())
                aircraft_types.push_back(type);
        };

        for (int i = 0; i < aircraft_types.size(); i++) {

            float hours_sum = 0.0, night_hours_sum = 0.0, instrument_hours_sum = 0.0, cross_country_hours_sum = 0.0;
            int landings = 0;
            type = aircraft_types[i];

            for (int i = 0; i < n; i++) {
                if (Flights[i].aircraft_type == type) {
                    hours_sum += Flights[i].total_hours;
                    night_hours_sum += Flights[i].night_hours;
                    instrument_hours_sum += Flights[i].instrument_hours;
                    cross_country_hours_sum += Flights[i].cross_country_hours;
                    landings += Flights[i].landings_count;
                }
            }

            std::cout << "==============================" << std::endl;
            std::cout << " Aircraft type: " << type << std::endl;
            std::cout << " Total hours: " << hours_sum << std::endl;
            std::cout << " Total night hours: " << night_hours_sum << std::endl;
            std::cout << " Total instrument hours: " << instrument_hours_sum << std::endl;
            std::cout << " Total cross-country hours: " << cross_country_hours_sum << std::endl;
            std::cout << " Total landings: " << landings << std::endl;

        }

        std::cout << "==============================\n" << std::endl;

    }

    break;

    case 3:

    {
        std::string manufacturer;
        std::vector<std::string> aircraft_manufacturers;

        for (int i = 0; i < n; i++) {
            manufacturer = Flights[i].aircraft_manufacturer;
            if (std::find(aircraft_manufacturers.begin(), aircraft_manufacturers.end(), manufacturer) == aircraft_manufacturers.end())
                aircraft_manufacturers.push_back(manufacturer);
        };

        for (int i = 0; i < aircraft_manufacturers.size(); i++) {

            float hours_sum = 0.0, night_hours_sum = 0.0, instrument_hours_sum = 0.0, cross_country_hours_sum = 0.0;
            int landings = 0;
            manufacturer = aircraft_manufacturers[i];

            for (int i = 0; i < n; i++) {
                if (Flights[i].aircraft_manufacturer == manufacturer) {
                    hours_sum += Flights[i].total_hours;
                    night_hours_sum += Flights[i].night_hours;
                    instrument_hours_sum += Flights[i].instrument_hours;
                    cross_country_hours_sum += Flights[i].cross_country_hours;
                    landings += Flights[i].landings_count;
                }
            }

            std::cout << "==============================" << std::endl;
            std::cout << " Aircraft manufacturer: " << manufacturer << std::endl;
            std::cout << " Total hours: " << hours_sum << std::endl;
            std::cout << " Total night hours: " << night_hours_sum << std::endl;
            std::cout << " Total instrument hours: " << instrument_hours_sum << std::endl;
            std::cout << " Total cross-country hours: " << cross_country_hours_sum << std::endl;
            std::cout << " Total landings: " << landings << std::endl;

        }

        std::cout << "==============================\n" << std::endl;

    }

    break;

    }

    return;
}

void write_to_file(std::vector<Flight>& Flights) {

    std::string temp = "";
    std::cout << "Enter file name for output: ";
    std::cin >> temp;

    namespace fs = std::filesystem;
    fs::create_directory("output");

    temp = "./output/" + temp + ".csv";

    std::ofstream fout;
    fout.open(temp.c_str());

    fout << "Flight date" << "," << "Departure ICAO" << "," << "Arrival ICAO" << "," << "Landings count" << "," << "Normal hours" << "," << "Night hours" << "," << "IFR hours" << "," << "Cross-country hours" << "," << "Tail number" << "," << "Aircraft manufacturer" << "," << "Aircraft type" << std::endl;

    for (int i = 0; i < n; i++) {
        fout << Flights[i].date << "," << Flights[i].departure_airport << "," << Flights[i].arrival_airport << "," << Flights[i].landings_count << "," << Flights[i].total_hours << "," << Flights[i].night_hours << "," << Flights[i].instrument_hours << "," << Flights[i].cross_country_hours << "," << Flights[i].tailnumber << "," << Flights[i].aircraft_manufacturer << "," << Flights[i].aircraft_type << std::endl;
    }

    fout.close();

    std::cout << "\nSave complete! (" << temp << ")\n";

    return;
}

int main() {

    std::vector<Flight> Flights;
    short unsigned int menuchoice;

    print_load_menu();
    menuchoice = choice();

    switch (menuchoice) {
    case 1:
        if (input_from_file(Flights) == -1) { return -1; };
        break;
    case 2:
        if (input_from_formatted_file(Flights) == -2) { return -2; };
        break;
    }

    do {
        print_main_menu();
        menuchoice = choice();

        switch (menuchoice) {
        case 1:
            system("cls");
            std::cout << std::endl;
            print_flights(Flights);
            break;

        case 2:
            system("cls");
            std::cout << std::endl;
            edit_flights(Flights);
            break;

        case 3:
            system("cls");
            std::cout << std::endl;
            calculate_stats(Flights);
            break;

        case 8:
            system("cls");
            std::cout << std::endl;
            change_settings();
            break;

        case 9:
            system("cls");
            std::cout << std::endl;
            write_to_file(Flights);
            return 0;

        case 0:
            return 0;
        }
    } while (menuchoice != 0);

    return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
