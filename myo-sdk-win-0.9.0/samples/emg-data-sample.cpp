// Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.

// This sample illustrates how to use EMG data. EMG streaming is only supported for one Myo at a time.

#include <array>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <myo/myo.hpp>
#include <filesystem>
#include "matplotlibcpp.h"

namespace plt = matplotlibcpp;

class DataCollector : public myo::DeviceListener {
public:
    DataCollector()
    : emgSamples()
    {
    }

    // onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user.
    void onUnpair(myo::Myo* myo, uint64_t timestamp)
    {
        // We've lost a Myo.
        // Let's clean up some leftover state.
        emgSamples.fill(0);
    }

    // onEmgData() is called whenever a paired Myo has provided new EMG data, and EMG streaming is enabled.
    void onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg)
    {
        for (int i = 0; i < 8; i++) {
            emgSamples[i] = emg[i];
        }
    }

    // There are other virtual functions in DeviceListener that we could override here, like onAccelerometerData().
    // For this example, the functions overridden above are sufficient.

    // We define this function to print the current values that were updated by the on...() functions above.
    void print()
    {
        // Clear the current line
        std::cout << '\r';

        // Print out the EMG data.
        for (size_t i = 0; i < emgSamples.size(); i++) {
            std::ostringstream oss;
            oss << static_cast<int>(emgSamples[i]);
            std::string emgString = oss.str();

            std::cout << '[' << emgString << std::string(4 - emgString.size(), ' ') << ']';
        }

        std::cout << std::flush;
    }
    // The values of this array is set by onEmgData() above.
    std::array<int8_t, 8> emgSamples;
};

void save_result(const int& number_of_test, const std::string& folder_name, const std::vector<std::array<int8_t, 8Ui64>>& results) {
    std::ofstream file_save("tests/"+folder_name + "/test" + std::to_string(number_of_test) + ".csv");
    // posibly will have to add *10^-2
    for (auto emgSampless : results) {
        for (auto emg_chanel : emgSampless) {
            file_save << double(emg_chanel) << ';';
        }
        file_save << std::endl;
    }
    file_save.close();
}

std::vector<std::vector<double>>  display_chart(const std::vector<std::array<int8_t, 8Ui64>>& results,const int& how_long) {
    std::vector<std::vector<double>> output;
    std::vector<double> y1, y2, y3, y4, y5, y6, y7, y8;
    std::vector<double> x(100);
    std::iota(std::begin(x), std::end(x), 0);
    for (auto prep_ploting : results) {
            y1.push_back(double(prep_ploting.at(0)));
            y2.push_back(double(prep_ploting.at(1)));
            y3.push_back(double(prep_ploting.at(2)));
            y4.push_back(double(prep_ploting.at(3)));
            y5.push_back(double(prep_ploting.at(4)));
            y6.push_back(double(prep_ploting.at(5)));
            y7.push_back(double(prep_ploting.at(6)));
            y8.push_back(double(prep_ploting.at(7)));
    }
    output.push_back(y1);
    output.push_back(y2);
    output.push_back(y3);
    output.push_back(y4);
    output.push_back(y5);
    output.push_back(y6);
    output.push_back(y7);
    output.push_back(y8);
    return output;
};

int main(int argc, char** argv)
{
    int number_of_testes{ 0 };
    char start_of_test{ '5' };
    std::string enter_folder_name{ "none" }, thrash{""};
    //printable is a vector of outputs to print 
    std::vector<std::vector<double>> printable;
    bool is_file_valid{ true };
    // enter in seconds how many second measurement should be going on
    int time{ 0 };
    // its help for time of mesurement how much time you gonna measure is Timecounter/number what is inside its  100hz not 200 dont know why 
    // that means we have 100 samples in 1 second

    std::vector<std::array<int8_t, 8Ui64>> results;

    // We catch any exceptions that might occur below -- see the catch statement for more details.
    try {

        // First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
        // publishing your application. The Hub provides access to one or more Myos.
        myo::Hub hub("com.example.emg-data-university");

        std::cout << "Attempting to find a Myo..." << std::endl;

        // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
        // immediately.
        // waitForMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
        // if that fails, the function will return a null pointer.
        myo::Myo* myo = hub.waitForMyo(10000);

        // If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
        if (!myo) {
            throw std::runtime_error("Unable to find a Myo!");
        }

        // We've found a Myo.
        std::cout << "Connected to a Myo armband!" << std::endl << std::endl;

        // section to create folder where results of operation will be stored
        std::cout << "Enter how many seconds tests should takes :";
        std::cin >> time;
        time *= 100;
        int Time_counter{ time };
        std::getline(std::cin, thrash);

        std::cout << std::endl;

        std::cout << "Enter folder name where will be stored test results :";
        while (is_file_valid == true)
        {
            std::cin >> enter_folder_name;
            is_file_valid = std::filesystem::is_directory("tests/" + enter_folder_name);
            if (is_file_valid == true) std::cout << "Folder name |" << enter_folder_name << "|  already exist try difrent name\n";
        }
        std::filesystem::create_directories("tests/" + enter_folder_name);


        // Next we enable EMG streaming on the found Myo.
        myo->setStreamEmg(myo::Myo::streamEmgEnabled);

        // Next we construct an instance of our DeviceListener, so that we can register it with the Hub.
        DataCollector collector;

        // Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
        // Hub::run() to send events to all registered device listeners.
        hub.addListener(&collector);

        std::cout << "To start measurement  ";
        system("pause");
        while (start_of_test != '1') {
            /*if (start_of_test != 'Y' && start_of_test != 'y') {
                std::cin >> start_of_test;
                std::getline(std::cin, thrash);
            }*/
            std::cout << "\nTest in progres..\n"; 
            // Finally we enter our main loop.
            while (Time_counter--) {
                // In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
                // In this case, we wish to update our display 50 times a second, so we run for 1000/20 milliseconds. 
                hub.run(5);
                // vector to store values
                results.push_back(collector.emgSamples);
                // After processing events, we call the print() member function we defined above to print out the values we've
                // obtained from any events that have occurred.
                collector.print();
            }
            printable = display_chart(results, Time_counter);
            // here i will implement ploting results where you will be able to decide if you want to save your test
            plt::plot(printable.at(0));
            plt::plot(printable.at(1));
            plt::plot(printable.at(2));
            plt::plot(printable.at(3));
            plt::plot(printable.at(4));
            plt::plot(printable.at(5));
            plt::plot(printable.at(6));
            plt::plot(printable.at(7));
            plt::show();

            start_of_test = 'a'; // just a helper to ewnter 2 loops
            std::cout << "\nDo you want to save result ?" << "\n[y]/[n] :";
            while (start_of_test != 'Y' && start_of_test != 'y' && start_of_test != 'N' && start_of_test != 'n')
            {
                std::cin >> start_of_test;
                std::getline(std::cin,thrash);
                if (start_of_test != 'Y' && start_of_test != 'y' && start_of_test != 'N' && start_of_test != 'n') std::cerr << "\nThis |" << start_of_test << thrash<< "| not work\n Try : [y]/[n]";
                if (start_of_test == 'Y' || start_of_test == 'y') {
                    save_result(number_of_testes, enter_folder_name, results);
                    number_of_testes++;
                }
            }
            start_of_test = 'a';
            results.clear();
            std::cout << "\nWhould you like to take next test ?" << "\nAlready you save : |" << number_of_testes << "| measurements" << "\n[y]/[n] :";
            while (start_of_test != 'Y' && start_of_test != 'y' && start_of_test != 'N' && start_of_test != 'n')
            {
                std::cin >> start_of_test;
                std::getline(std::cin, thrash);
                if (start_of_test != 'Y' && start_of_test != 'y' && start_of_test != 'N' && start_of_test != 'n') std::cerr << "\nThis |" << start_of_test << thrash<< "| not work\n Try : [y]/[n]";
            }
            if (start_of_test == 'Y' || start_of_test == 'y') {
                start_of_test = 'y';
                Time_counter = time;
            }
            else if (start_of_test == 'N' || start_of_test == 'n') {
                start_of_test = '1';
                break;
            }
            system("CLS");
        }
        // If a standard exception occurred, we print out its message and exit.
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
        return 1;
    }
    system("CLS");
}