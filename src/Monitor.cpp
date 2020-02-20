#include <cstdlib>
#include <ctime>

#include "MessageSystem.h"
#include "Message.h"
#include "NetProfile.h"
#include "SafeQueue.h"


#include <unordered_map>

#include <boost/thread.hpp>
#include <boost/atomic/atomic.hpp>

namespace Auction
{
    using std::cout;
    using std::endl;
    using namespace Auction;
    class Monitor;
}

class Auction::Monitor
{
public:

    Monitor()
    {
    }


    void message_processor(boost::atomic<bool>& running)
    {
        std::cout << "[Message thread] ---> running" << std::endl;
        while(running)
        {
            if (message_list.isEmpty()) continue;
            std::cout << "[Message processor] Processing new message"<<std::endl;
            Message * m;
            message_list.pop(m);
            boost::this_thread::sleep_for(boost::chrono::milliseconds(1));

            switch (m->type)
            {
                case NEW_ROBOT:
                    NewRobotMessage * nr = dynamic_cast<NewRobotMessage*>(m);
                    new_robot_message_handler(nr);
                    break;
            }
            delete m;    

            if (num_of_robots == 3)
            {
                const int TASK_NUM = 2;
                std::cout << "[Message processor] Generating and sending " <<TASK_NUM<<(TASK_NUM>1?"tasks":"task") << endl;
                for (int i = 0; i<TASK_NUM; i++)
                {
                    NewTaskMessage nt(generate_random_task());
                    message_system.broadcast_message(nt);
                    boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
                }
            }

        }
    }

    void new_robot_message_handler(NewRobotMessage * nr)
    {
        // add netprofile to unordered map
        if (nr->unique_id == NewRobotMessage::REQUEST_ID)
        {
            std::cout << "Received NewRobot message from "<<nr->np.to_string()<< std::endl;
            nr->unique_id = next_robot_id();    // replace REQUEST_ID with the new unique_id
            message_system.add_robot_info(nr->unique_id, nr->np); // store new robot net profile

            
            for (int id = 1; id<=num_of_robots; id++)
            {
                NetProfile np = message_system.get_robot_info(id);
                nr->np = np;
                nr->unique_id = id;

                message_system.broadcast_message(*nr);
            }
        }
    }

    
    void message_server(boost::atomic<bool>& running)
    {
        NetProfile monitor_info = message_system.get_monitor_info();
        if (monitor_info.host == NULL || monitor_info.port == NULL)
        {
            std::cout << "NULL member" << std::endl;
            return;
        }


        int socket_descriptor = RcSocket::passiveSocket(monitor_info.port, "udp", 0);
        if (socket_descriptor < 0)
        {
            std::cout << "Error in RcSocket::passiveSocket"<< std::endl;
            return;
        }

        std::cout << "[Monitor] --> listening on port " << monitor_info.port << std::endl;

        while (running)
        {
            struct sockaddr sender;
            uint addrlen;
            char message_systemg[256];
            memset(&message_systemg, 0, 256);
            // Recibe la informacion y la guarda en el buffer
            if (!recvfrom(socket_descriptor, message_systemg, sizeof(message_systemg), 0, &sender, &addrlen))
            {
                printf("[Server thread] Error de recepcion \n");
            }
            else
            { 
                Message * m = this->message_system.create_message_from(message_systemg);
                message_list.push(m);
            }
        }
    }


    Task generate_random_task(){
        Task t(Point2D(), Point2D(), 1, 2, next_task_id());
        return t;
    }

    int next_task_id()
    {
        return ++num_of_tasks;
    }

    int next_robot_id()
    {
        return ++num_of_robots;
    }

    SafeQueue<Message*> message_list;
    MessageSystem message_system;
    int robot_id = 0;
    int num_of_robots = 0;
    int num_of_tasks = 0;
};



int main()
{
    Auction::Monitor m;
    boost::atomic<bool> running(true);
    boost::thread server_thread(&Auction::Monitor::message_server, &m, boost::ref(running));
    boost::thread message_thread(&Auction::Monitor::message_processor, &m, boost::ref(running));
    server_thread.join();
    message_thread.join();
}