#ifndef SERIAL_H
#define SERIAL_H

#include <iostream>
#include <stdio.h>
#include <deque> 

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp> 
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/lexical_cast.hpp> 

class Serial
{
	public:
		/**
		 * Constructor.
		 * \param port device name, example "/dev/ttyUSB0" or "COM4"
		 * \param baud_rate communication speed, example 9600 or 115200
		 * \throws boost::system::system_error if cannot open the
		 * serial device
		 */
		Serial(boost::asio::io_service& io, std::string port, unsigned int baud_rate)
		: io_service(io), serial(io_service, port), active(true)
		{
			if (!serial.is_open())
			{
				std::cerr << "Failed to open serial port\n";
				return;
			} 
		    boost::this_thread::sleep(boost::posix_time::seconds(2)); // /!\ arduino needs time to start serial because openning serial triggers bootloader
		    serial.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
		    serial.set_option(boost::asio::serial_port_base::flow_control( boost::asio::serial_port_base::flow_control::none ));
		    serial.set_option(boost::asio::serial_port_base::parity( boost::asio::serial_port_base::parity::none ));
		    serial.set_option(boost::asio::serial_port_base::stop_bits( boost::asio::serial_port_base::stop_bits::one ));
		    serial.set_option(boost::asio::serial_port_base::character_size(boost::asio::serial_port_base::character_size(8U)));
		    read_start(); 
		}
		
		void write(const char msg) // pass the write data to the do_write function via the io service in the other thread
		{
		    io_service.post(boost::bind(&Serial::do_write, this, msg));
		}
	   
		void close() // call the do_close function via the io service in the other thread
		{
		    io_service.post(boost::bind(&Serial::do_close, this, boost::system::error_code()));
		}

		bool isActive() // return true if the socket is still active
		{
		    return active;
		} 
   
	private:
		static const int max_read_length = 512; // maximum amount of data to read in one operation
		
		void read_start(void)
        { // Start an asynchronous read and call read_complete when it completes or fails
                serial.async_read_some(boost::asio::buffer(read_msg, max_read_length),
                        boost::bind(&Serial::read_complete,
                                this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
        }
       
        void read_complete(const boost::system::error_code& error, size_t bytes_transferred)
        { // the asynchronous read operation has now completed or failed and returned an error
                if (!error)
                { // read completed, so process the data
                        std::cout << "read msg: " << read_msg << "bytes transferred: " << bytes_transferred << std::endl; // echo to standard output
                        read_start(); // start waiting for another asynchronous read again
                }
                else
                        do_close(error);
        }
       
        void do_write(const char msg)
        { // callback to handle write call from outside this class
                bool write_in_progress = !write_msgs.empty(); // is there anything currently being written?
                write_msgs.push_back(msg); // store in write buffer
                if (!write_in_progress) // if nothing is currently being written, then start
                        write_start();
        }
       
        void write_start()
        { // Start an asynchronous write and call write_complete when it completes or fails
                boost::asio::async_write(serial,
                        boost::asio::buffer(&write_msgs.front(), 1),
                        boost::bind(&Serial::write_complete,
                                this,
                                boost::asio::placeholders::error));
        }
       
        void write_complete(const boost::system::error_code& error)
        { // the asynchronous read operation has now completed or failed and returned an error
                if (!error)
                { // write completed, so send next write data
                        write_msgs.pop_front(); // remove the completed data
                        if (!write_msgs.empty()) // if there is anthing left to be written
                                write_start(); // then start sending the next item in the buffer
                }
                else
                        do_close(error);
        }
       
        void do_close(const boost::system::error_code& error)
        { // something has gone wrong, so close the socket & make this object inactive
                if (error == boost::asio::error::operation_aborted) // if this call is the result of a timer cancel()
                        return; // ignore it because the connection cancelled the timer
                if (error)
                        std::cerr << "Error: " << error.message() << std::endl; // show the error message
                else
                        std::cout << "Error: Connection did not succeed.\n";
                std::cout << "Press Enter to exit\n";
                serial.close();
                active = false;
        } 
         
		boost::asio::io_service& io_service;
		boost::asio::serial_port serial;
		bool active;
		char read_msg[max_read_length]; // data read from the socket
		std::deque<char> write_msgs; // buffered write data 
};

#endif // SERIAL_H
