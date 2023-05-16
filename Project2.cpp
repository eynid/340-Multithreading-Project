/*
Enid Hou Project 2 Compilation: 

g++ -std=c++11 Project2.cpp -o Project2

./Project2

 - outputFile should be named "output.txt" 
 
 - instruction and prose files are unchanged and should be in the same folder as the project. does not require any argv[]. 

CHANGES FROM PROJECT 2 WRITEUP TEST: 

- Changed thread total into string a array to easily break up the strings with a loop and ensure an orderly print out with 
appropriate information with pwrite(0);

- Changed the final string into a string array as well, allowing for the final thread to be printed in order with pwrite();

- Renamed thread Array into instruction Array for clarity. 

- Finished the pwrite sections 

- 
*/
#include <iostream>
#include <string>
#include <pthread.h>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <cstring>
#include <mutex>
#include <map>

using namespace std;

const int buffSize = 4096;

string instructionArray[20];

string threadTotal[20];

string stringFinal[20];

int outPut = open("output.txt", O_RDWR);

//The output file to write to.

int proseFile = open("prose.txt", O_RDONLY);

char stringBuffer[buffSize];

class threadParam{
    
    public:

    threadParam(int firstL, int secondL ,int pid){
        firstLimit = firstL;
        secondLimit = secondL;
        threadID = pid;
    }
    int getPID(){
        return threadID;
    }
    //Set/Get the first instruction that the thread must start reading from.
    int getfirstLimit(){
        return firstLimit;
    }
    //Set/Get the stop integer for the thread.
    int getsecondLimit(){
        return secondLimit;
    }

    void setfirstLimit(int first){
        firstLimit = first;
    }
    void setsecondLimit(int second){
        secondLimit = second;
    }
    
    //Set/Get for the offset and amount to be read from the files. 
    
    void setjReadAmount(int jread){
        jreadAmount = jread;
    }
    
    void setkRead(int kread){
        kreadValues = kread;
    }
    
    int getjRead(){
        return jreadAmount;
    }
    
    int getkRead(){
        return kreadValues;
    }
    
    //Obtains the last string read by the thread.
    string getLastValue(){
        return lastValue;
    }
    // Concatenates the strings read by a thread. 
    string getallRead(){
        return allRead;
    }
    void setLastRead(string threadRead){
        allRead = threadRead;
    }
    void setAllRead(string lastRead){
        lastValue = lastRead;
    }
    
    private:
    int firstLimit, secondLimit;
    int jreadAmount, kreadValues;
    string lastValue;
    string allRead;
    int threadID;

};

string printThreadRead(char buf[], ssize_t bytesRead){
    //Used to obtain the string read by a thread. 
    string s = "";
    
    for(int i = 0; i < bytesRead; i++){
        s += buf[i];
    }

    return s;
}

void *runner(void * param){

    threadParam * p = (threadParam *)param;
    
    int tID = p->getPID();
    
    int count = 0;

    mutex locker;

    /*The critical section of the threads, will read from the threadArray that contains the instructions, and use the
     received information to read from the prose file and store into stringBuffer. 
     This section uses a mutex locker to prevent any other threads from entering this section and potentially messing up 
     the shared variable count. 
    */
    while(true){
        locker.lock();

        //Stop Condition, each thread is supposed to read 5 instructions, this prevents it from reading any more than what it is supposed to.
        
        if(p->getfirstLimit() > p->getsecondLimit()){
            break;
        }
            //Obtain the input line to be broken up, starting from the thread's respective integers. 
            string input(instructionArray[p->getfirstLimit()]);
            stringstream ss(input);
            string token;
            
            string execute = "\nInstruction: " + instructionArray[p->getfirstLimit()] + "\n";
            cout << execute;

            while(getline(ss, token, ',') && p->getfirstLimit() <= p->getsecondLimit()){ 
                //Loop through the array until the second limit is reached. 
                count++;
                if(count == 2){
                    p->setjReadAmount(stoi(token));
                    //Obtain the offset.
                }
            
                if(count % 3  == 0){

                    p->setkRead(stoi(token));
                    //Set amount to be read from buffer.
                    ssize_t bytesRead = pread(proseFile, stringBuffer,p->getkRead(), p->getjRead());

                    p->setAllRead(printThreadRead(stringBuffer, bytesRead));
                    
                    p->setLastRead(p->getallRead() + " " + p->getLastValue() );   
                    
                    threadTotal[p->getfirstLimit()] = "\n Thread : " + to_string(tID) + " - Executed instruction - " + instructionArray[p->getfirstLimit()] + " < Read word: " + p->getLastValue()+  " >\n";   
                    //Set Value to be written to the outfile with pwrite. 
                    stringFinal[p->getfirstLimit()] = p->getLastValue();

                    p->setfirstLimit(p->getfirstLimit() + 1); //Increment.

                    string temp = "\n Thread " + to_string(tID) + " read : " + p->getLastValue()  + "\n";
                    
                    cout << temp;  
                    
                    count = 0; 

            }
            

        }

        locker.unlock();
    }
    
    
    pthread_exit(NULL);
}

int main(int argc, char*argv[]) {

    int instructionFile = open("instruction2.txt", O_RDONLY);

    ssize_t bytesRead = 0;
    
    char buffer[buffSize];
    
    //Main reads all the instructions from the file and puts into buffer.

    bytesRead = pread(instructionFile, buffer, buffSize, 0);
    
    if(bytesRead == -1){
        cout << "Error reading file." << endl;
    }

    // Break the buffer into lines that can be put into the instruction Array.
    string input(buffer);
    stringstream ss(input);
    string line;

    int lineRead = 0;

    //Read instruction into an array, ensuring that i can set where the threads start reading from.
    while(getline(ss, line)){
        instructionArray[lineRead] = line;
        lineRead++;        
    }
    
    threadParam *p[4];  

    //initialize and create threads, setting the instruction lines that must be read to the thread parameters. 

    p[0] = new threadParam(0,4,1);
    p[1] = new threadParam(5,9,2);
    p[2] = new threadParam(10,14,3);
    p[3] = new threadParam(15,19,4);  

    pthread_t tid[4];

    pthread_attr_t attr[4];
    
    for(int i = 0; i < 4; i++){
        pthread_attr_init(&(attr[i]));
        pthread_create(&(tid[i]), &(attr[i]), runner, p[i]);
    }
    
    for (int i = 0; i < 4; ++i) {
     pthread_join(tid[i], NULL);
     cout << "Joined thread \n"; 

    }
    for (int i = 0; i < 4; ++i) delete p[i];
    
    int offset = 0;

    //Printing out all the required information to the outfile. Refer to line 180 to see the written information.

    for(int i = 0; i < 20; i ++){
    
        string msg = threadTotal[i];
    
        pwrite(outPut,msg.c_str(),threadTotal[i].length(),offset);
    
        offset += threadTotal[i].length();
        
    }
    
    string msg = "\n Final Thread : ";
    
    pwrite(outPut, msg.c_str(), msg.length(), offset);
    
    offset += msg.length();
    

    string orderedStitch = "";

    //Stitch together all the threads one by one. This ensures the order of the final thread regardless of which thread decided to go first. 
    for(int i = 0; i< 20; i++){
        orderedStitch += stringFinal[i];
        orderedStitch += " " ;
    }

    pwrite(outPut, orderedStitch.c_str(), orderedStitch.length(), offset);
    
    //Cleaning up files.

    close(instructionFile);
    
    close(outPut);
    
    close(proseFile);
}