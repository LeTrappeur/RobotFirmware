#ifndef CAMERATRACKER_H
#define CAMERATRACKER_H

#include <iostream>
#include <stdio.h>

#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>

class SimpleSerial
{
public:
    /**
     * Constructor.
     * \param port device name, example "/dev/ttyUSB0" or "COM4"
     * \param baud_rate communication speed, example 9600 or 115200
     * \throws boost::system::system_error if cannot open the
     * serial device
     */
    SimpleSerial(std::string port, unsigned int baud_rate)
    : io(), serial(io)
    {
        serial.open(port);
        boost::this_thread::sleep(boost::posix_time::seconds(2)); // /!\ arduino needs time to start serial because openning serial trigger bootloader
        serial.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
        serial.set_option(boost::asio::serial_port_base::flow_control( boost::asio::serial_port_base::flow_control::none ));
        serial.set_option(boost::asio::serial_port_base::parity( boost::asio::serial_port_base::parity::none ));
        serial.set_option(boost::asio::serial_port_base::stop_bits( boost::asio::serial_port_base::stop_bits::one ));
        serial.set_option(boost::asio::serial_port_base::character_size(boost::asio::serial_port_base::character_size(8U)));

    }

    /**
     * Write a string to the serial device.
     * \param s string to write
     * \throws boost::system::system_error on failure
     */
    void writeString(std::string s)
    {
        boost::asio::write(serial,boost::asio::buffer(s.c_str(),s.size()));
    }

    void write(const char *data, size_t size)
    {
        boost::asio::write(serial,boost::asio::buffer(data,size));
    }

    void writeChar(char c)
    {
        char data[1];
        data[0] = c;
        boost::asio::write(serial,boost::asio::buffer(data,sizeof(data)));
    }


    /**
     * Blocks until a line is received from the serial device.
     * Eventual '\n' or '\r\n' characters at the end of the string are removed.
     * \return a string containing the received line
     * \throws boost::system::system_error on failure
     */
    std::string readLine()
    {
        //Reading data char by char, code is optimized for simplicity, not speed
        using namespace boost;
        char c;
        std::string result;
        for(;;)
        {
            asio::read(serial,asio::buffer(&c,1));
            switch(c)
            {
                case '\r':
                    break;
                case '\n':
                    return result;
                default:
                    result+=c;
            }
        }
    }

private:
    boost::asio::io_service io;
    boost::asio::serial_port serial;
};

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//values are set for a blue color in a typical room envir.
const int H_MIN = 54;
const int H_MAX = 118;
const int S_MIN = 70;
const int S_MAX = 134;
const int V_MIN = 40;
const int V_MAX = 108;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
// default movement borders
const int MIN_MOVEMENT_CENTER=250;
const int MAX_MOVEMENT_CENTER=390;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS=50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20*20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;

class CameraTracker
{
    public:
        CameraTracker();
        ~CameraTracker();

        void run();
    protected:
    private:
        void morphOps(cv::Mat& thresh);
        void trackFilteredObject(int& x, int& y, cv::Mat threshold, cv::Mat& cameraFeed);
};

#endif // CAMERATRACKER_H
