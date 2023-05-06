#include <fstream>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include <sys/wait.h>
#include <bits/stdc++.h>

#include "BoundedBuffer.h"
#include "common.h"
#include "Histogram.h"
#include "HistogramCollection.h"
#include "FIFORequestChannel.h"


let x = 10
let x = "eshwar"


// ecgno to use for datamsgs
#define EGCNO 1

using namespace std;

//creat a struct{}
    //stores patient_no -->int
    //result --> double
    //constructor to store data
struct patient_struct{
	int patient_no;
	double result;
	patient_struct(){};
	patient_struct(int a,double b){
		patient_no=a;
		result=b;
	};
};


void patient_thread_function (int num_requests,int patient_no, int buffer_size, BoundedBuffer &request_buffer) {
    // functionality of the patient threads
	double time = 0.0;
	for(int i=0;i<num_requests;i++){
		datamsg x(patient_no,time,EGCNO);
		char* buff = new char[buffer_size];
		memcpy(buff,&x,sizeof(datamsg));
		request_buffer.push(buff,sizeof(datamsg));
		time+=0.004;
		delete[] buff;
	}

    //create an intial datamsg
    //loop over all n items
        //push the request to a request_buffer
        //Increment time

	

}

void file_thread_function (FIFORequestChannel* channel,string fname, BoundedBuffer *request_buffer, int buffer_size) {
    // functionality of the file thread

    //send a filemsg request with fm(0,0) to get filesize through control channel::chan->cwrite()
    //recieve the file size : chan->cread()
	filemsg fm(0, 0);	
	int len = sizeof(filemsg) + (fname.size() + 1);
	int64_t filesize =0;
	char* Buff = new char[buffer_size];

	memcpy(Buff, &fm, sizeof(filemsg));
	strcpy(Buff + sizeof(filemsg), fname.c_str());
	channel->cwrite(Buff, len);  // I want the file length;
	channel->cread(&filesize,sizeof(int64_t));

	//int reqNum = filesize/buffer_size;
	//double actualReqNum = double(filesize)/double(buffer_size);
	
	string filepath = "received/" + string(fname);
	FILE* outFile = fopen(filepath.c_str(),"wb+");
	fseek(outFile,filesize,SEEK_SET);
	fclose(outFile);

	// filemsg* file_req = (filemsg*) Buff;
    // int64_t rem = filesize;

    // while(rem>0){
    //     file_req->length = min(rem,(int64_t) buffer_size);

    //     cout << "Came here"<< endl;
    //     filemsg temp(file_req->offset,file_req->length);
    //     char* buf_last = new char[len];
    //     memcpy(buf_last, &temp, sizeof(filemsg));
	// 	strcpy(buf_last + sizeof(filemsg), fname.c_str());
    //     request_buffer.push(buf_last, len);

    //     //request_buffer->push(buf, sizeof(filemsg)+fname.size()+1);
    //     file_req->offset +=file_req->length;
    //     rem -= file_req->length;
    //     delete[] buf_last;

	filemsg* f = (filemsg*) Buff;
    int offset =0;
    int64_t rem = filesize;
    int buffer = buffer_size;
    while (rem > 0){
        if (buffer > rem){
            buffer = rem;
        }

        offset = filesize - rem;
        f->offset = offset;
        f->length = buffer;

        request_buffer->push(Buff, sizeof(Buff));

        rem -= buffer;
    }   
    // if(filesize<buffer_size){
	// 	filemsg* file_req = (filemsg*) Buff;
	// 	file_req->offset =0;
	// 	file_req->length = filesize;

	// 	filemsg temp(file_req->offset,file_req->length);
    //     char* buf_last = new char[len];
    //     memcpy(buf_last, &temp, sizeof(filemsg));
	// 	strcpy(buf_last + sizeof(filemsg), fname.c_str());
    //     request_buffer.push(buf_last, len);
	// 	delete[] buf_last;

	// }else{

	// 	filemsg* file_req = (filemsg*) Buff;
	// 	file_req->offset = 0;
	// 	file_req->length = buffer_size;

	// 	filemsg temp(file_req->offset,file_req->length);
    //     char* buf_last = new char[len];
    //     memcpy(buf_last, &temp, sizeof(filemsg));s
	// 	strcpy(buf_last + sizeof(filemsg), fname.c_str());
    //     request_buffer.push(buf_last, len);
	// 	delete[] buf_last;

	// 	while(--reqNum){
	// 		file_req->offset += buffer_size;

	// 		filemsg temp(file_req->offset,file_req->length);
	//         char* buf_last = new char[len];
	//         memcpy(buf_last, &temp, sizeof(filemsg));
	// 		strcpy(buf_last + sizeof(filemsg), fname.c_str());
	//         request_buffer.push(buf_last, len);
	// 		delete[] buf_last;
	// 	}
	// 	file_req->offset +=buffer_size;
	// 	if(file_req->offset != filesize){

	// 		int lastSector = filesize - file_req->offset; 
	// 		file_req->length = lastSector;

	// 		filemsg temp(file_req->offset,file_req->length);
	//         char* buf_last = new char[len];
	//         memcpy(buf_last, &temp, sizeof(filemsg));
	// 		strcpy(buf_last + sizeof(filemsg), fname.c_str());
	//         request_buffer.push(buf_last, len);
	// 		delete[] buf_last;

	// 	}
	// }

	delete[] Buff;
    //open file
    // fopen(,,"'wb+");
    // fseek(,,file_size,SEEK_SET)
    // fclose();

    //loop over all the chunks in the file
        //Instead of sending a cwrite to a channel, push the request to request_buffer
        //rb->push((char*)x,size_of_request)
}

void worker_thread_function (FIFORequestChannel* channel,BoundedBuffer &request_buffer,BoundedBuffer &response_buffer,int buffer_size) {
    // functionality of the worker threads
    while(true){

    	char* buff = new char[buffer_size];
        //pop a request from the request_buffer into buf
        request_buffer.pop(buff,buffer_size);
        MESSAGE_TYPE* M = (MESSAGE_TYPE*) buff;

     	if(*M==DATA_MSG){
     		datamsg* x = (datamsg*) buff;
     		double response;
     		channel->cwrite(buff,sizeof(datamsg));
     		channel->cread(&response,sizeof(double));

     		patient_struct p(x->person,response);
     		char* newBuf = new char[buffer_size];
     		memcpy(newBuf,&p,sizeof(patient_struct));
     		response_buffer.push(newBuf,buffer_size);
     		delete[] newBuf;

     	}else if(*M==FILE_MSG){
     		filemsg f = *((filemsg*) buff);
            FILE * fp;
            
	        string filename = buff + sizeof(filemsg);
            filename = "received/" + filename;
            fp = fopen (filename.c_str() , "r+" );
            char * buf_loc = new char[buffer_size];
            int len = sizeof(filemsg) + (filename.size() + 1);
            channel->cwrite(buff, len);
            channel->cread(buf_loc,buffer_size);
            
            
            fseek (fp, f.offset, SEEK_SET );
            //cout << f.length << endl;
            //fputs (buf_loc, fp);
            fwrite(buf_loc, 1, f.length,fp);
            fclose(fp);
            delete[] buf_loc;
     		// char* recBuff = new char[buffer_size];
     		// filemsg* fm = (filemsg*) buff;
     		// // string fname = (char*)(fm+1);
     		// string fname = buff + sizeof(filemsg);
     		// int len = fname.size() + sizeof(filemsg) + 1;
     		// channel->cwrite(buff,len);
     		// channel->cread(recBuff,buffer_size);

     		// string filepath = "received/" + fname;

     		// FILE* outFile = fopen(filepath.c_str(),"wb");
     		// fseek(outFile,fm->offset,SEEK_SET);
     		// fwrite(recBuff,1,fm->length,outFile);
     		// fclose(outFile);
     		// delete[] recBuff;

     	}else if(*M==QUIT_MSG){
     		delete[] buff;
     		break;
     	}
        //if m is datamsg
            //write datamsg to the channel w
            //read result (--> Type double) from channelw
            //store result in instanace of struct along with person_number
            //push object to response_buffer
            //res_buffer->push((char*) &obj,sizeof(struc));
        //else if m is filemsg
            //typecast buf to filemsg
            //read the name of the file
            //write request to channel w
            //read respons form the channel2

            //open file
            //use fseek to point offset location in the file fseek(filname,offset,SEEK_SET);
            //write to file
            //close the file
        //else if m is quiting
            //write to channel
            //delete the channel
     	delete[] buff;
    }//check where and how to get out of loop
}

void histogram_thread_function (HistogramCollection &hist_collection,BoundedBuffer &response_buffer,int buffer_size) {
    while(true){
    	char* buff = new char[buffer_size];
    	response_buffer.pop(buff,buffer_size);

    	patient_struct p;
    	memcpy(&p,buff,sizeof(patient_struct));
    	if(p.patient_no == -1){
    		delete[] buff;
    		break;
    	}
    	hist_collection.update(p.patient_no,p.result);
    	delete[] buff;
    }
    // functionality of the histogram threads
    //pop a response from the response_buffer
    //Typecasting char* to struct
    //have a checj if preson value is <=0. to breake out of infinite loop
    //update histogram with the person_number and the value. --> have 
}


int main (int argc, char* argv[]) {
    int n = 1000;	// default number of requests per "patient"
    int p = 10;		// number of patients [1,15]
    int w = 100;	// default number of worker threads
	int h = 20;		// default number of histogram threads
    int b = 20;		// default capacity of the request buffer (should be changed)
	int m = MAX_MESSAGE;	// default capacity of the message buffer
	string f = "";	// name of file to be transferred

	bool isPatient = false;
	bool isFile = false;    

    // read arguments
    int opt;
	while ((opt = getopt(argc, argv, "n:p:w:h:b:m:f:")) != -1) {
		switch (opt) {
			case 'n':
				n = atoi(optarg);
                break;
			case 'p':
				p = atoi(optarg);
                isPatient = true;
                break;
			case 'w':
				w = atoi(optarg);
                break;
			case 'h':
				h = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
                break;
			case 'm':
				m = atoi(optarg);
                break;
			case 'f':
				f = optarg;
				isFile = true;
                break;
		}
	}

    
	// fork and exec the server
    int pid = fork();
    if (pid == 0) {
        execl("./server", "./server", "-m", (char*) to_string(m).c_str(), nullptr);
    }
    
	// initialize overhead (including the control channel)
	FIFORequestChannel* chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
    BoundedBuffer request_buffer(b);
    BoundedBuffer response_buffer(b);
	HistogramCollection hc;

	vector<FIFORequestChannel*> channels_vec;
	thread* producer_threads;
    thread* worker_threads = new thread[w];
	thread* histogram_threads;

	if(isPatient){
		producer_threads = new thread[p];
	}

	if(isFile){
		producer_threads = new thread[1];
	}


	
	if(isPatient){
		histogram_threads = new thread[h];
	}else{
		histogram_threads = new thread[0];
	}

    // making histograms and adding to collection
    for (int i = 0; i < p; i++) {
        Histogram* h = new Histogram(10, -2.0, 2.0);
        hc.add(h);
    }
	
	// record start time
    struct timeval start, end;
    gettimeofday(&start, 0);

    /* create all threads here */

    if(isPatient){
    	for(int i=0;i<p;i++){
    		producer_threads[i] = thread(patient_thread_function,n,i+1,m,ref(request_buffer));
	    }
    }

    if(isFile){
    	producer_threads[0] = thread(file_thread_function,chan,f,&request_buffer,m);
    }
    
	for(int i=0;i<w;i++){
		MESSAGE_TYPE newChan = NEWCHANNEL_MSG;

		chan->cwrite(&newChan,sizeof(MESSAGE_TYPE));
		char* newBuf = new char[m];
		chan->cread(newBuf,sizeof(string));
		string nameBuff = newBuf;

		FIFORequestChannel* newChannel = new FIFORequestChannel{nameBuff,FIFORequestChannel::CLIENT_SIDE};
		channels_vec.push_back(newChannel); 
		
		//worker_threads[i] = thread(worker_thread_function,newChannel,ref(request_buffer),ref(response_buffer),m);

		delete[] newBuf;
    }

    for(int i=0;i<w;i++){
    	worker_threads[i] = thread(worker_thread_function,channels_vec[i],ref(request_buffer),ref(response_buffer),m);
    }

    if(isPatient){
    	for(int i=0;i<h;i++){
	    	histogram_threads[i] = thread(histogram_thread_function,ref(hc),ref(response_buffer),m);
	    }
    }
    

    //thread* file_thread = new thread[1];
    //if no file name create p threeads -- array to 


    //create w threads for workers -- send refernece of each channel that these worker threads would use along with other required parameters
    //thread(worker_thread_function,wchans[i],...)

    //create h histogram thread(histogram_thread_function,..)

	/* join all threads here */
	if(isPatient){
		for(int i=0;i<p;i++){
	    	producer_threads[i].join();
	    }
	}

	if(isFile){
		producer_threads[0].join();
	}
    

    for(int i=0;i<w;i++){
    	MESSAGE_TYPE q = QUIT_MSG;
    	request_buffer.push((char*) &q,sizeof(MESSAGE_TYPE));
    }

    for(int i=0;i<w;i++){
    	worker_threads[i].join();
    }

    for(int i=0;i<h;i++){
    	patient_struct quitt(-1,0);
       	response_buffer.push((char *) &quitt, sizeof (patient_struct));
    }

    if(isPatient){
    	for(int i=0;i<h;i++){
	    	histogram_threads[i].join();
	    }
    }
    


    //join p threads or file threads

    //dont forget to send quitmsh signal (w) by pushin them to reques_buffer -- This would ensure that worker threads berak out of loop

    //join w woker threads
    //Dont' forget to send strcut msgs (h) with 0,0 as parameter -- this would breka the working of histogram threads out of loop
    //join histogram threads

    delete[] producer_threads;
    delete[] worker_threads;
    delete[] histogram_threads;

	// record end time
    gettimeofday(&end, 0);

    // print the results
	if (f == "") {
		hc.print();
	}
    int secs = ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) / ((int) 1e6);
    int usecs = (int) ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) % ((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;

	MESSAGE_TYPE qw = QUIT_MSG;
    for(int i=0;i<w;i++){
    	channels_vec[i]->cwrite((char *) &qw, sizeof (MESSAGE_TYPE));
    	delete channels_vec[i];
    }

	// quit and close control channel
    MESSAGE_TYPE q = QUIT_MSG;
    chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
    cout << "All Done!" << endl;
    delete chan;

	// wait for server to exit
	wait(nullptr);
}
