#include <string>
#include <map>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <time.h>
#include <uuid/uuid.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/types.h>
#include <signal.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <iostream>

#include "OCPlatform.h"
#include "OCApi.h"
#include "sensor.h"

#define UUID_SIZE 36
#define BEFORE_VALUE 0
#define AFTER_VALUE 1
#define VALUE_GAP 2
#define CHANGE_TRACKER 3

using namespace OC;

//static const char * SVR_DB_FILE_NAME = "./oic_svr_db_client.dat"
typedef std::map<OCResourceIdentifier, std::shared_ptr<OCResource>> DiscoveredResourceMap;

int block_flag = 1;

int initFindResource();
DiscoveredResourceMap discoveredResources;
std::shared_ptr<OCResource> curResource;
static ObserveType OBSERVE_TYPE_TO_USE = ObserveType::Observe;
std::mutex curResourceLock;
std::string uri ;
int sleepTime=1500000;
std::mutex blocker;
std::mutex put_blocker;
std::mutex get_blocker;
std::condition_variable cv;
std::condition_variable put_cv;
std::condition_variable get_cv;




class SensorResource 
{

public: 
    std::string m_name;
    int light_state;
    int light_power;

    int gas_state;
    int gas_efflux;

    double temper;
    double humi;
    int temp_state;
    int hum_state;

    int flame_state;
    int flame_power;

    int fire_alarm;

    std::string json_info;

    int time_stp;

    SensorResource():m_name(""),light_state(0),light_power(0),gas_state(0),gas_efflux(0),
                        temper(0.0),humi(0.0),temp_state(0),hum_state(0),flame_state(0),flame_power(0),
                        fire_alarm(0),json_info(""),time_stp(0)
    {

    }
};

SensorResource sensorDev;

void blockCheckFunc()
{
    while(true)
    {
        time_t time_now;
        time(&time_now);

        if (time_now - sensorDev.time_stp > 20)
        {
            uri = "/a/sensor0";
            curResource = 0;
            std::cout << "****************************************************" << std::endl;
            std::cout << "first uri : " << uri << std::endl;
            std::cout << "first find" << std::endl;
            initFindResource(2);
            std::cout << "****************************************************" << std::endl;
            std::cout << "****************************************************" << std::endl;
            std::cout << "second find" << std::endl;
            initFindResource(2);
            std::cout << "****************************************************" << std::endl;
            if(block_flag==1)
            {
                std::unique_lock<std::mutex> put_lock(put_blocker);
                put_cv.notify_all();
            }
            else if(block_flag==2)
            {
                std::unique_lock<std::mutex> get_lock(get_blocker);
                get_cv.notify_all();
            }
        }
    }
}


void onGet(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    try
    {
        if(eCode == OC_STACK_OK)
        {
            std::cout << "GET request was successful" << std::endl;
            std::cout << "Resource URI: " << rep.getUri() << std::endl;

            rep.getValue("fire_alarm",sensorDev.fire_alarm);
            rep.getValue("light_state",sensorDev.light_state);
            std::cout << "fire_alarm " <<sensorDev.fire_alarm<< std::endl;

            std::cout << "light_state " <<sensorDev.light_state<< std::endl;
        }
        else
        {
            std::cout << "onGET Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch(std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onGet" << std::endl;
    }

    std::unique_lock<std::mutex> get_lock(get_blocker);
   	get_cv.notify_all();
}

void getSensorRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        std::cout << "Getting Light Representation..."<<std::endl;
        // Invoke resource's get API with the callback parameter

        QueryParamsMap test;
        block_flag=2;
        resource->get(test, &onGet);
    }
}



void onPut(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    try
    {
        if(eCode == OC_STACK_OK)
        {
            std::cout << "PUT request was successful" << std::endl;
        }
        else
        {
            std::cout << "onPut Response error: " << eCode << std::endl;
           // std::exit(-1);
        }
    }
    catch(std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onPut" << std::endl;
    }

    std::unique_lock<std::mutex> put_lock(put_blocker);
    put_cv.notify_all();
}

void putSensorRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        OCRepresentation rep;

        std::cout << "Putting light representation..."<<std::endl;

        std::cout<<"gas value result:"<< "'" << sensorDev.gas_efflux << "'" << std::endl;

        rep.setValue("m_name", sensorDev.m_name);
        rep.setValue("light_state", sensorDev.light_state);
        rep.setValue("light_power", sensorDev.light_power);
        rep.setValue("gas_state", sensorDev.gas_state);
        rep.setValue("gas_efflux", sensorDev.gas_efflux);
        rep.setValue("temper", sensorDev.temper);
        rep.setValue("humi", sensorDev.humi);
        rep.setValue("temp_state", sensorDev.temp_state);
        rep.setValue("humi_state", sensorDev.hum_state);
        rep.setValue("flame_state", sensorDev.flame_state);
        rep.setValue("flame_power", sensorDev.flame_power);
        rep.setValue("time_stp", sensorDev.time_stp);
        rep.setValue("json_info", sensorDev.json_info);

        std::cout << "sending"<<std::endl;
        block_flag=1;
        // Invoke resource's put API with rep, query map and the callback parameter
        std::cout <<"put result print: "<<resource->put(rep, QueryParamsMap(),&onPut)<<std::endl;
        std::cout <<"put result print2: "<<OC_STACK_OK<<std::endl;
            // std::cout << "sending.."<<std::endl;
        
    }
}


// Local function to get representation of light resource


void onPost(const HeaderOptions& /*headerOptions*/,
        const OCRepresentation& rep, const int eCode)
{
    try
    {
        if(eCode == OC_STACK_OK || eCode == OC_STACK_RESOURCE_CREATED)
        {
            std::cout << "POST request was successful" << std::endl;

            if(rep.hasAttribute("createduri"))
            {
                std::ostringstream requestURI;

                rep.getValue("createduri",uri);
                std::cout << "**********************************8"
                    << std::endl;
                std::cout << "\tUri of the created resource: "
                    <<uri<< std::endl;
                std::cout << "**********************************8"
                    << std::endl;
            }
        }
        else
        {
            std::cout << "onPost Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch(std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onPost" << std::endl;
    }
   // initFindResource();
    
    std::unique_lock<std::mutex> lock(blocker);
    cv.notify_all();
}

// Local function to put a different state for this resource
void postSensorRepresentaion(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        OCRepresentation rep;

        std::cout << "Posting light representation..."<<std::endl;

        rep.setValue("createduri",1);

        // Invoke resource's post API with rep, query map and the callback parameter
        resource->post(rep, QueryParamsMap(), &onPost);
    }
}

void foundResource(std::shared_ptr<OCResource> resource)
{
    std::cout << "In foundResource\n"<<std::endl;
    std::string resourceURI;
    std::string hostAddress;
    try
    {
        
        std::lock_guard<std::mutex> lock(curResourceLock);
        if(discoveredResources.find(resource->uniqueIdentifier()) == discoveredResources.end())
        {
            std::cout << "Found resource " << resource->uniqueIdentifier() <<
                    " for the first time on server with ID: "<< resource->sid()<<std::endl;
            discoveredResources[resource->uniqueIdentifier()] = resource;
        }
        else
        {
            std::cout<<"Found resource "<< resource->uniqueIdentifier() << " again!"<<std::endl;
        }

        // Do some operations with resource object.
        if(resource)
        {
            std::cout<<"DISCOVERED Resource:"<<std::endl;
            // Get the resource URI
            resourceURI = resource->uri();
            std::cout << "\tURI of the resource: " << resourceURI << std::endl;

            // Get the resource host address
            hostAddress = resource->host();
            std::cout << "\tHost address of the resource: " << hostAddress << std::endl;

            // Get the resource types
            std::cout << "\tList of resource types: " << std::endl;
            for(auto &resourceTypes : resource->getResourceTypes())
            {
                std::cout << "\t\t" << resourceTypes << std::endl;
            }

            // Get the resource interfaces
            std::cout << "\tList of resource interfaces: " << std::endl;
            for(auto &resourceInterfaces : resource->getResourceInterfaces())
            {
                std::cout << "\t\t" << resourceInterfaces << std::endl;
            }

            std::cout << "\t------------------------------------------: " << std::endl;
            std::cout << "find uri :" << uri<<std::endl;
            std::cout << "\t------------------------------------------: " << std::endl;

            if(resourceURI == "/a/sensor0" && !curResource) //post section
            {
                std::cout << "\t------------------------------------------: " << std::endl;
                    std::cout << "find uri sensor0 " << std::endl;
                curResource = resource;
                // Call a local function which will internally invoke get API on the resource pointer
                postSensorRepresentaion(resource);
            }
            else if(resourceURI==uri && curResource) //get,put
            {   
                curResource = resource;
                std::unique_lock<std::mutex> lock(blocker);
                cv.notify_all(); 
            }
        }
        else
        {
            // Resource is invalid
            std::cout << "Resource is invalid" << std::endl;
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Exception in foundResource: "<< e.what() << std::endl;
    }
}

static FILE* client_open(const char* /*path*/, const char *mode)
{
    return fopen("./oic_svr_db_client.json", mode);
}

int initFindResource(int check)
{
    std::ostringstream requestURI;
    if(check==1)
    {
        OCPersistentStorage ps {client_open, fread, fwrite, fclose, unlink };
        try
        {
           OBSERVE_TYPE_TO_USE = ObserveType::ObserveAll;
        }
        catch(std::exception& )
        {
            std::cout << "<===Invalid input arguments===>\n\n";
            return -1;
        }

        // Create PlatformConfig objobect
        PlatformConfig cfg {
            OC::ServiceType::InProc,
            OC::ModeType::Both,
            "0.0.0.0",
            0,
            OC::QualityOfService::LowQos,
            &ps
        };

        OCPlatform::Configure(cfg);
    }
    try
    {
        // makes it so that all boolean values are printed as 'true/false' in this stream
        if(check==1) std::cout.setf(std::ios::boolalpha);
        // Find all resources
        requestURI << OC_RSRVD_WELL_KNOWN_URI<<"?rt=core.sensor";

        OCPlatform::findResource("",requestURI.str(),CT_DEFAULT, &foundResource);
        std::cout<< "Finding Resource... " <<std::endl;
        std::unique_lock<std::mutex> lock(blocker);
        cv.wait(lock);
        std::cout<< "end found resources "<<std::endl;
    }catch(OCException& e)
    {
        oclog() << "Exception in main: "<<e.what();
    }

    return 0;
}




std::string exec(const char* cmd) 
{
    char buffer[128];
    std::string result = "";
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
            break;    
        }
    return result;
}


int main() 
{
	SENSOR_VALUE sensor_value;
	time_t time_now;
	uuid_t id;
	char *tmp = new char [37];
    int fd,sensor_flag=0,fire_flag=0,gasTracker[5]={-200,-200,-200,-200,-200},
            flameTracker[5]={-200,-200,-200,-200,-200},humTracker[7]={-200,-200,-200,-200,-200,-200,0},
            tempTracker[7]={-200,-200,-200,-200,-200,-200,0},filter=0;
    FILE *pFile=NULL;


    if(wiringPiSetup() == -1)
    { 
            fprintf (stdout, "Unable to start wiringPi: %s\n", strerror(errno));
    }
    pinMode(26, OUTPUT);
    digitalWrite(26, HIGH);
  


	if(0<(fd = open( "./uuid_log.txt",O_RDONLY)))
   	{
        std::cout<< "success open"<<std::endl;
      	read(fd,tmp,UUID_SIZE);
      	//tmp[36]='/0';
   	}
  	else
   	{
        std::cout<< "fail open"<<std::endl;
     	uuid_generate(id);
		uuid_unparse(id,tmp);
		system("touch uuid_log.txt");
		if(0<(fd = open( "./uuid_log.txt",O_WRONLY)))
   		{
      		write(fd,tmp,UUID_SIZE);
   		}
        //tmp[36]='/0';
   	}

	sensorDev.m_name = tmp;

    std::cout<<"uuid: "<<sensorDev.m_name<<std::endl;

    uri = "/a/sensor0";
    std::cout<< "****************************************************"<<std::endl;
    std::cout<< "first uri : "<<uri<<std::endl;
    std::cout<< "first find"<<std::endl;
    initFindResource(1);
    std::cout<< "****************************************************"<<std::endl;
    std::cout<< "****************************************************"<<std::endl;
    std::cout<< "second find"<<std::endl;
    initFindResource(2);
    std::cout<< "****************************************************"<<std::endl;
    //std::lock_guard<std::mutex> lock(curResourceLock);

    std::cout<< "start run"<<std::endl;
   
	//curResource->observe(OBSERVE_TYPE_TO_USE, QueryParamsMap(), &onObserve);
	
    sensorDev.light_state = false;
    sensorDev.light_power = 0;

    thread t(&blockCheckFunc);

    while(true)
    {
        
	    sensor_value = readSensor(sensor_flag);
        if(sensor_flag==0) sensor_flag=1;
	    

	    sensorDev.gas_efflux = sensor_value.gasValue;
	    sensorDev.flame_power = sensor_value.flameValue;

        if(th_data[2]!= -1000) sensorDev.temper = th_data[2];

	     if(th_data[0]!= -1000) sensorDev.humi = th_data[0];

        gasTracker[4] =  sensorDev.gas_efflux;
        flameTracker[4] = sensorDev.flame_power;
        tempTracker[5] = sensorDev.temper;
        humTracker[5] = sensorDev.humi;

        std::cout<< "value : "<<tempTracker[5]<<std::endl;
        
        if(filter >  5)
        {
            checkTempState(tempTracker);
            checkFlameState(flameTracker);
            checkGasState(gasTracker);
            checkHumiState(humTracker);
        }

	    time(&time_now);
	    sensorDev.time_stp = time_now;

	    putSensorRepresentation(curResource);
	    std::unique_lock<std::mutex> put_lock(put_blocker);
	    put_cv.wait(put_lock);

       
	    getSensorRepresentation(curResource);
	    std::unique_lock<std::mutex> get_lock(get_blocker);
	    get_cv.wait(get_lock);


	    if(sensorDev.fire_alarm > 0)//화재 발생시에
	    {
            int check=0;
            if (sensorDev.light_state>0)
            {
                pinMode(26, OUTPUT);
                digitalWrite(26, HIGH);
            } 
            else if(sensorDev.light_state == 0)
            {
                pinMode(26, OUTPUT);
                digitalWrite(26, LOW);
            }

            if (fire_flag==0)//처음 발견 
            {
                std::cout<<"init fire alarm"<<std::endl;
                std::string command_tmp = std::to_string(sensorDev.fire_alarm);
                check = system("rm ./json_string.txt");
                std::string command ="node ./nodejs-iotivityBeacon/app.js "+command_tmp+" 1 &";

                check = system(command.c_str());
                fire_flag= 1;
                if (check == -1 || check == 127)
                {   
                    std::cout<< "system function error "<<std::endl;
                }

            }
            
            std::string json_string;

            if(pFile == NULL)//Json 읽기
            {
                std::cout<<"read file"<<std::endl;
                pFile = fopen( "./json_string.txt", "r" );
            }
            else if(pFile != NULL)
            {
                char str [1024]; 
                fseek ( pFile , 0 , SEEK_SET );
                while(fgets(str, sizeof(str), pFile))
                {
                    json_string+=str;
                }
            } 

             sensorDev.json_info = json_string;


            std::cout << "'" << json_string << "'" << std::endl;

	    }
        else if(sensorDev.fire_alarm == 0 && fire_flag==1)
        {
            int check=0;
            std::string tmp;
            pinMode(26, OUTPUT);
            digitalWrite(26, LOW);
            tmp = exec("ps -elf | grep node | grep -v grep | awk '{ print $2 }'");
            std::cout << tmp <<std::endl;
            int PID = atoi(tmp.c_str());
            std::cout << "---------------------------" <<std::endl;
            std::cout << PID <<std::endl;
            kill(PID,9);
            check = system("rm ./nodejs-iotivityBeacon/json_string.tx");
            if (check == -1 || check == 127)
            {
                std::cout<< "system function error "<<std::endl;
            }

            sensorDev.json_info = "";

            fire_flag = 0;
        }

        delay(1000);
	    std::cout<<" while end "<<std::endl;
        filter++;


  	}

    return 0;
}