
#include "cl_util.h"
#include "cl_aesEbc.h"
#include "cl_sha1.h"
#include "cl_file64.h"

#include <iostream>
using namespace std;

void print_help();
int main(int argc, char** argv)
{
	cl_util::debug_memleak();
	if (cl_util::string_array_find(argc, argv, "-h") > 0
		|| cl_util::string_array_find(argc, argv, "-v") > 0
		|| argc < 3)
	{
		print_help();
	}

	cl_aesEbcHeader_t inf;
	uchar aeskey[32];
	char hash[41];
	size64_t size = 0;

	memset(aeskey, 0, 32);
	memset(hash, 0, 41);
	if (argc > 3 && 0 == strcmp(argv[1], "-e"))
	{
		cl_aes::rand_key(aeskey, 16);
		if (0 == cl_aesEbc_encrypt_file(argv[2], argv[3], aeskey, inf))
		{
			size = cl_file64::get_file_size(argv[3]);
			Sha1_BuildFile(argv[3], hash, NULL);
			list<cl_string> ls;
			char buf[128];
			cl_string shapath = argv[3];
			shapath += ".sha";
			sprintf(buf, "%lld", size);
			ls.push_back(hash);
			ls.push_back(buf);
			ls.push_back((char*)aeskey);
			cl_util::put_stringlist_to_file(shapath, ls);

			cout << "encrypt " << argv[2] << " to " << argv[3] << " ok!" << endl;
			cout << "key = " << aeskey << endl;
			cout << "hash = " << hash << endl;
		}
	}

	if (argc > 3 && 0 == strcmp(argv[1], "-d"))
	{
		if(argc>4)
			strcpy((char*)aeskey, argv[4]);
		if (0 == cl_aesEbc_decrypt_file(argv[2], argv[3], aeskey))
		{
			cout << "decrypt " << argv[2] << " to " << argv[3] << " ok!" << endl;
		}
	}


	getchar();
	return 0;
}

void print_help()
{
	cout << cl_util::get_module_name().c_str() <<  " -- [ver: 20180428]" << endl;
	cout << "params:" << endl;
	cout << " -e from_path to_path ; -- encrypt file. rand key and save to file" << endl;
	cout << " -d from_path to_path aeskey ; -- decrypt key." << endl;
}
