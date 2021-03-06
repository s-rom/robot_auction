#ifndef MONITOR_AUCTION
#define MONITOR_AUCTION

#include <cstdlib>
#include <ctime>

#include "MessageSystem.h"
#include "Message.h"
#include "NetProfile.h"
#include "SafeQueue.h"
#include "InfoReporter.h"
#include "RobotStatus.h"
#include "TaskStatus.h"

#include <boost/atomic/atomic.hpp>
#include <boost/thread.hpp>

#include <unordered_map>
#include <signal.h>





namespace Auction
{
    class Monitor;
}

class Auction::Monitor
{
public:
    Monitor();
    Monitor(std::string & program_path);
    
    void message_processor(boost::atomic<bool>& running);
    void message_server(boost::atomic<bool>& running);

    void new_task_message_handler(Auction::NewTaskMessage & new_task);
    void new_robot_message_handler(Auction::NewRobotMessage * nr);
    void kill_robot_message_handler(Auction::SimpleMessage & kill_robot);
    void leader_alive_message_handler(Auction::MonitoringMessage & leader_alive);
    void robot_alive_message_handler(Auction::MonitoringMessage & robot_alive);
    void helper_alive_message_handler(Auction::MonitoringMessage & helper_alive);

    Auction::SafeQueue<Auction::Message*> message_queue;
    Auction::MessageSystem message_system;
    Auction::InfoReporter info_report;
    std::unordered_map<int, Auction::RobotStatusInfo> robot_status;    
    std::unordered_map<int, std::pair<Auction::Task, Auction::TaskStatus>> task_list;

    // Simulation ploting
    void update_robot_pose(int id, Auction::Point2D point, float yaw)
    {
        if (robot_points.find(id) == robot_points.end())
        {
            info_report << "[Update Robot Pos] First instanciation for robot"<< id <<"\n";
            auto pair = std::make_pair(point, yaw);
            robot_points[id] = pair;
        }
        else 
        {
            auto & pair = robot_points[id];
            pair.first = point;
            pair.second = yaw;
        }
    }


    std::unordered_map<int, std::pair<Auction::Point2D, float>> robot_points;

    int get_number_of_robots() { return num_of_robots; }
    int get_number_of_tasks() { return num_of_tasks; }

    
private:
    std::string get_log_path(std::string & program_path);
    Auction::Task generate_random_task();
    void check_robot_status();
    int robot_id = 0;
    int num_of_tasks = 0;
    int num_of_robots = 0;
    int next_task_id();
    int next_robot_id();

    const int NULL_ID = -1;
    const int NULL_TASK = NULL_ID;
};


#endif