import random as rnd

### Generation params
N_ROBOTS = 6
load_capacity_range = [1, 4]
max_linear_speed_range = [3, 6]

### Each robot will take a random color from the following list
colors = ['green', 'blue', 'red']

### Paths and names
catkin_ws = "/home/sergi/catkin_ws/src/auction/" # catkin workspace root 
launch_path = catkin_ws + "launch/"              # launch files directory inside catkin ws
world_path = launch_path + "world_files/"        # world files directory inside catkin ws
starting_port = 25556                            # port corresponding to robot 0

base_world_file = "base.world"                   # local name of the base world file         -- STAGE
robot_inc_file = "pioner.txt"                    # local name of the robot include file      -- STAGE

base_launch = "launch_1_robot_vfh.launch"        # local name of the base launch file for one robot

generated_world_suffix = "_robots.world"         # suffix for the generated world file (n_robots.world)
generated_launch_suffix = "_robots_vfh.launch"   # suffix for the generated launch file (n_robots.launch)

def main():
    global N_ROBOTS
    generate_world_file(N_ROBOTS)

    # At least two robots must be generated to avoid stage_ros name collisions
    if (N_ROBOTS == 1):
        N_ROBOTS = 2
    
    generate_launch_file(world_path + str(N_ROBOTS) + generated_world_suffix, N_ROBOTS)


def generate_world_file(n_robots):
    with open(world_path + base_world_file,'r') as base_word:
        with open(world_path + str(n_robots) + generated_world_suffix, "w") as world_file:
            world_file.write(base_word.read())
            for i in range(0, n_robots):
                world_file.write("\n"+generate_random_robot(i)+"\n")
            print("Generated "+world_file.name)

def generate_launch_file(world_file_name, n_robots):
    with open(launch_path+str(n_robots)+generated_launch_suffix,'w') as launch_file:
        launch_file.write("<launch>\n\n")

        # stage node
        launch_file.write("\t<node pkg=\"stage_ros\" type=\"stageros\" name=\"stage\" args=\""+world_file_name+"\"/>\n")

        # Add robots
        for i in range(0, n_robots):
            launch_file.write("\t<include file=\"$(find auction)/launch/"+base_launch+"\">\n")
            launch_file.write("\t\t<arg name=\"robot_name\" value=\"robot_"+str(i)+"\"/>\n")

            launch_file.write("\t\t<arg name=\"port\" value=\"#"+str(i + starting_port)+"\"/>\n")
            launch_file.write("\t\t<arg name=\"hostname\" value=\"localhost\"/>\n")

            load_cap = rnd.randint(load_capacity_range[0], load_capacity_range[-1])
            max_vel = rnd.randint(max_linear_speed_range[0], max_linear_speed_range[-1])

            launch_file.write("\t\t<arg name=\"load_capacity\" value=\""+str(load_cap)+"\"/>\n")
            launch_file.write("\t\t<arg name=\"max_linear_speed\" value=\""+str(max_vel)+"\"/>\n")

            launch_file.write("\t</include>\n")
    
        launch_file.write("\n</launch>")
        print("Generated "+launch_file.name)


def generate_random_robot(id):
    global colors
    x = rnd.randrange(-10,10)
    y = rnd.randrange(-10,10)
    color = colors[rnd.randint(0,len(colors)-1)]
    robot = get_robot_world_chunk(id, x, y, 0, 0, color)
    return robot

def get_robot_world_chunk(robot_id, x, y, z, yaw, color):
    data = ''
    with open(world_path + robot_inc_file) as generic_file:
        data = generic_file.read()
        data = data.replace('@robot_name', 'robot_' + str(robot_id))
        data = data.replace('@yaw', str(yaw))
        data = data.replace('@x', str(x))
        data = data.replace('@y', str(y))
        data = data.replace('@z', str(z))
        data = data.replace('@color', color)
    return data
    

if __name__ == "__main__":
    main()

