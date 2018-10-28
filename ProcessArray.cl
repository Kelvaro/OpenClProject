__kernel void ProcessArray(__global char* data, __global char* outData) {

	int r = data[get_global_id(0)] & 0xff;
	int g = (data[get_global_id(0)] >> 8) & 0xff;
	int b = (data[get_global_id(0)] >> 16) & 0xff;
	int a = (data[get_global_id(0)] >> 24) & 0xff;

	char greyScale = (r + g + b) / 3;
	
	
	//char greyScale = data[get_global_id(0)] / 3;

	outData[get_global_id(0)] = greyScale;//data[get_global_id(0)] = 2;
}

/*
__kernel void ProcessArray(__global int* data, __global int* outData) {
	outData[get_global_id(0)] = data[get_global_id(0)] = 2;
}*/