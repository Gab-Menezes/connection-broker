#include <iostream>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "Server.h"

int main()
{        
    //reads the configuration file
    property_tree::ptree config;
    try
    {
        property_tree::read_json("config.json", config);
        //creates the output directory for safety
        std::filesystem::create_directory(config.get<std::string>("output_dir"));

        //starts the server
        Server server(config);
        //run loop
        while (true)
            server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
    }

    return 0;
}
