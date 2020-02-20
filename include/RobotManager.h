#ifndef ROBOT_MANAGER
#define ROBOT_MANAGER

// ROS
#include <ros/ros.h>


// Auction
#include "NetProfile.h"
#include "Task.h"
#include "Point2D.h"
#include "Message.h"
#include "MessageSystem.h"
#include "SafeQueue.h"
#include "InfoReporter.h"

// std
#include <iostream>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <chrono>

// TMP -- TODO: REPLACE
#include <ctime>
#include <cstdlib>


// Boost
#include <boost/thread.hpp>
#include <boost/atomic/atomic.hpp>

namespace Auction
{
    using std::to_string;
    using std::chrono::system_clock;
    using std::chrono::duration_cast;
    using namespace Auction;
    class RobotManager;
}


class Auction::RobotManager
{
public:
    /**
     * Constructor.
     * @param net_info a reference to a NetProfile with the robot's net host and port
     */ 
    RobotManager(NetProfile & net_info);


    /**
     * Message server function to be executed by a thread.
     * Receives UDP messages and stores them into a the safe queue
     * 
     * @param running while true, the server is running
     */
    void message_server(boost::atomic<bool> & running);


    /**
     * General auction function, to be executed by a thread.
     * Processes messages found in the safe queue
     * 
     * @param running while true, the process is running
     */ 
    void auction_process(boost::atomic<bool> & running);

    /**
     * Leader request algorithm. 
     * Executed when a robot requests being a leader for a given task.
     * 
     * Implementation of algorithm 1 in reference paper
     * 
     * @param t reference to the task the robot is requesting 
     */
    void leader_request(Task & t);

    /**
     * Auction for a task algorithm.
     * Executed by the leader of a given task. The leader generates
     * a list of robots that can complete the task within the deadline frame
     *  
     * Implementation of algorithm 2 in reference paper
     */
    void task_auction(Task & t);
    

    // Member attributes
    int id;                                            // Robot unique id
    MessageSystem message_system;                      // Message system
    std::unordered_map<int, Task> task_list;           // List of known tasks
    SafeQueue<Message*> message_queue;                 // Queue of received messages

private:
  
    /**
     * Function executed when a NewRobotMessage is found in the queue 
     * by the auction_process
     * 
     * @param nr message found
     */
    void new_robot_message_handler(NewRobotMessage & nr);      

    /**
     * Function executed when a NewTask is found in the queue 
     * by the auction_process
     * 
     * @param nr message found
     */
    void new_task_message_handler(NewTaskMessage & nt);                                                       
                                                     
    
    /**
     * Function executed by the message process thread when a LeaderOfTaskMessage
     * is found in the queue. 
     * Do not confuse with the leader request algorithm, this method is only executed
     * when a robot is not requesting a task
     */ 
    void leader_of_task_handler(LeaderOfTaskMessage & lead_msg);


    /**
     * Returns the work_capacity for a given task. 
     * Implementation of equation 5 in reference paper
     */
    float get_work_capacity(Task & t);
    
    const float LOAD_CAPACITY = 1;      // Maximum load capacity - kg (PLACEHOLDER)
    const float V_MAX = 10;             // Max linear velocity - m/s  (PLACEHOLDER)                   
    const int TIME_LEADERSHIP = 3000;   // Max time for the leader request - millis (PLACEHOLDER)
    const int TIME_AUCTION = 3000;      // Max time for the auction for a task algorithm - millis (PLACEHOLDER)
};

#endif