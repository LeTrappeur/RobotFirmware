#include <iostream>
#include <boost/asio.hpp>

#include "CameraTracker.h"
#include "Serial.h"

using namespace std;

int main()
{
	boost::asio::io_service io_service;
    Serial serial(io_service, "/dev/ttyACM0",9600); // rasp 
    //Serial serial(io_service, "/dev/pts/2", 9600); //emul
    boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service)); 
    
    CameraTracker cameraTracker(serial);
    cameraTracker.run();
    
    std::cout << "Closing thread" << std::endl;
    serial.close();
    t.join();
    return 0;
}
