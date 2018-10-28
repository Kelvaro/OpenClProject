#define CL_USE_DEPRECATED_OPENCL_2_0_APIS

//#include <CL/cl.h>
#include <CL/cl.hpp>
#include <fstream>
#include <iostream>
#include <chrono>

#include <vector>
#include <iterator>
#include <algorithm>

#define HundredMill 100000000
using namespace std;


int mode = 1;
size_t bmpHeaderSize = 54;



int main()
{



	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	cl::Device device;

	uint64_t max = 0;
	for (size_t i = 0;i < platforms.size();i++) {

		std::vector<cl::Device> devices;
		platforms[i].getDevices(CL_DEVICE_TYPE_GPU, &devices);
		if (devices.size() == 0) continue;

		for (size_t n = 0;n < devices.size();n++) {
			auto vendor = devices[n].getInfo<CL_DEVICE_VENDOR>();
			auto version = devices[n].getInfo<CL_DEVICE_VERSION>();
			auto memSize = devices[n].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>(); //CL_DEVICE_MAX_WORK_GROUP_SIZE

			if (memSize > max) {
				max = memSize;
				device = devices[n];
			}
			std::cout << "Vendor: " << vendor << "\tVersion: " << version << "\tMax: " << memSize << std::endl;
		}

	}

	auto vendor = device.getInfo<CL_DEVICE_VENDOR>();
	std::cout << "vendor: " << vendor << std::endl;
	
	std::ifstream infile("ProcessArray.cl");
	std::string src(std::istreambuf_iterator<char>(infile), (std::istreambuf_iterator<char>()));

	cl::Program::Sources sources(1, std::make_pair(src.c_str(), src.length() + 1)); //vector
	cl::Context context(device); //physical devices
	cl::Program program(context, sources);

	auto err = program.build("-cl-std=CL1.2"); //target opencl 1.2


	//read in image here
	ifstream ifs("./images/omg.bmp", ios::binary | ios::ate);	//read in image to imageBuffer
	ifstream::pos_type pos = ifs.tellg();

	std::vector<char>  imageBuffer(pos);
	ifs.seekg(0, ios::beg);
	ifs.read(&imageBuffer[0], pos);
	ifs.close();

	vector<char> bmpheader;

	for (size_t n = 0;n < bmpHeaderSize;n++) bmpheader.push_back(imageBuffer[n]);


	auto t1 = std::chrono::high_resolution_clock::now();
	if (mode == 0) {
		for (int i = bmpHeaderSize;i < imageBuffer.size();i++) {

			if (imageBuffer[i] != 255) {

				int r = imageBuffer[i] & 0xff;
				int g = (imageBuffer[i] >> 8) & 0xff;
				int b = (imageBuffer[i] >> 16) & 0xff;
				int a = (imageBuffer[i] >> 24) & 0xff;

				char greyScale = (r + g + b) / 3;
				imageBuffer[i] = greyScale;
			}

		}
	}
	else if (mode == 1) {

		cl::Buffer inBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, sizeof(char) * imageBuffer.size(), imageBuffer.data()); //dont want kernel to modify array being passed in
		cl::Buffer outBuf(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sizeof(char)  * imageBuffer.size()); //only allocate outBuf on the device, we dont need to give it any data?
		cl::Kernel kernel(program, "ProcessArray");
		kernel.setArg(0, inBuf);
		kernel.setArg(1, outBuf);

		cl::CommandQueue queue(context, device); //send some commands to the device
	
		err = queue.enqueueNDRangeKernel(kernel, cl::NDRange(bmpHeaderSize), cl::NDRange(imageBuffer.size()));

		err = queue.enqueueReadBuffer(outBuf, CL_FALSE, 0, sizeof(char) * imageBuffer.size(), imageBuffer.data());
		cl::finish();
	}

	auto t2 = std::chrono::high_resolution_clock::now();
	std::cout << "f() took "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
		<< " milliseconds\n";


	//std::cout << imageBuffer[imageBuffer.size() - 100] << "\n";



	for (size_t n = 0; n < bmpHeaderSize;n++) imageBuffer[n] = bmpheader[n]; //sets bmp header info back to the way it was 

	//write to file
	std::ofstream fout; 
	fout.open("./images/output.bmp", std::ios::binary | std::ios::out);
	std::ostream_iterator<unsigned char> output_iterator(fout);
	std::copy(imageBuffer.begin(), imageBuffer.end(), output_iterator);

	fout.close();


	/*
	std::vector<int> vec(1000000);
	//std:fill(vec.begin(), vec.end(), 1);

	cl::Buffer inBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, sizeof(int) * vec.size(), vec.data()); //dont want kernel to modify array being passed in
	cl::Buffer outBuf(context, CL_MEM_WRITE_ONLY|CL_MEM_HOST_READ_ONLY, sizeof(int)  * vec.size()); //only allocate outBuf on the device, we dont need to give it any data?
	cl::Kernel kernel(program, "ProcessArray");
	kernel.setArg(0, inBuf);
	kernel.setArg(1, outBuf);


	cl::CommandQueue queue(context, device); //send some commands to the device
	//fill vector
	err =queue.enqueueFillBuffer(inBuf, 10, sizeof(int) * 10, sizeof(int) * (vec.size()-10)); //fills inBuffer? from the 10th element to last, with 3s /////only works for cpu device?!?!?!
	

	auto t1 = std::chrono::high_resolution_clock::now();

	//work
	err =queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(vec.size()));
	err =queue.enqueueReadBuffer(outBuf, CL_FALSE, 0, sizeof(int) * vec.size(), vec.data());
	cl::finish();


	auto t2 = std::chrono::high_resolution_clock::now();
	std::cout << "f() took "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
		<< " milliseconds\n";


	std::cout << vec[vec.size()-10] << "\n";
	*/

	system("pause");
	return 0;
}

/*


	auto t1 = std::chrono::high_resolution_clock::now();

	//work

	auto t2 = std::chrono::high_resolution_clock::now();
	std::cout << "f() took "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
		<< " milliseconds\n";



	std::vector<int> data(100000000); //100 mill

	for (int i = 0;i < 100000000;++i) {
		data[i] *= 2;
	
	}
*/


void printOpenCLInfo()
{
	std::cout << "===== OPENCL INFORMATION =====" << std::endl;
	// platform info
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	for (auto& platform : platforms)
	{
		std::string info;
		platform.getInfo(CL_PLATFORM_NAME, &info);
		std::cout << "PLATFORM:\t" << info << std::endl;

		// device info
		std::vector<cl::Device> devices;
		platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
		for (auto& device : devices)
		{
			auto vendor = device.getInfo<CL_DEVICE_VENDOR>();
			auto version = device.getInfo<CL_DEVICE_VERSION>();
			std::cout << "DEVICE:\t" << vendor << "\t" << version << std::endl;
		}

		std::cout << std::endl;
	}
	std::cout << "==============================" << std::endl;
}


void ReadImage() {



	ifstream ifs("./images/img1.bmp", ios::binary | ios::ate);
	ifstream::pos_type pos = ifs.tellg();

	std::vector<char>  result(pos);

	ifs.seekg(0, ios::beg);
	ifs.read(&result[0], pos);


	for (int i = 55;i < result.size();i++) {

		if (result[i] != 255) {

			int r = result[i] & 0xff;
			int g = (result[i] >> 8) & 0xff;
			int b = (result[i] >> 16) & 0xff;
			int a = (result[i] >> 24) & 0xff;

			char greyScale = (r + g + b) / 3;
			result[i] = greyScale;
		}

	}



	std::ofstream fout;
	fout.open("./images/output.file", std::ios::binary | std::ios::out);
	std::ostream_iterator<unsigned char> output_iterator(fout);
	std::copy(result.begin(), result.end(), output_iterator);

	fout.close();

}